#include "Player_AttackComponent.h"
#include "Components/CapsuleComponent.h"
#include "Player_EvasiveComponent.h"
#include "Player_MovementComponent.h"
#include "Player_CameraComponent.h"
#include "Player_ElectroGaugeComponent.h"
#include "../PlayerCharacter.h"
#include "../Enemy/EnemyManager/EnemyManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

//===マジックナンバー排除用の定数定義===
namespace {
	//間合い詰め（ステップ）が完了したと判定するための残り距離の閾値
	constexpr float StopDistanceThreshold = 5.0f;
	//空中強攻撃（たたき落とし）発動時に、一度軽く浮き上がるためのZ軸上昇速度
	constexpr float AirFallRiseVelocityZ = 400.0f;
}

//コンストラクタ
UPlayer_AttackComponent::UPlayer_AttackComponent()
{
	//コンポーネントがゲーム開始時に初期化され、毎フレームTick処理が走るようにフラグを設定する
	PrimaryComponentTick.bCanEverTick = true;
}

//ゲーム開始時
void UPlayer_AttackComponent::BeginPlay()
{
	Super::BeginPlay();

	//オーナーであるプレイヤーキャラクターを取得し、依存する各種コンポーネントをキャッシュしておく
	//※毎フレームの取得コストを下げるための処理
	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player) return;

	m_MovementComponent = m_Player->FindComponentByClass<UPlayer_MovementComponent>();
	if (!m_MovementComponent) return;

	m_EvasiveComponent = m_Player->FindComponentByClass<UPlayer_EvasiveComponent>();
	if (!m_EvasiveComponent) return;

	m_CameraComponent = m_Player->FindComponentByClass<UPlayer_CameraComponent>();
	if (!m_CameraComponent) return;

	m_ElectroComponent = m_Player->FindComponentByClass<UPlayer_ElectroGaugeComponent>();
	if (!m_ElectroComponent) return;
}

//毎フレーム
void UPlayer_AttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//強攻撃の溜め時間の計測・処理の更新
	_updateAttackHeavy(DeltaTime);
	//ジャンプ時の高さに基づく空中攻撃の解禁判定の更新
	_updateAirAttackUnlock();
}

void UPlayer_AttackComponent::_updateAttackHeavy(float DeltaTime)
{
	//===地上の溜め攻撃フェーズ===
	if (m_IsHeavyCharging)
	{
		m_HeavyChargeTime += DeltaTime;

		//上限を超えないようにパラメーターの最大溜め時間で制限(クランプ)をかける
		m_HeavyChargeTime = FMath::Min(m_HeavyChargeTime, m_PlayerParam.HeavyChargeMaxTime);
		return;
	}

	//以降は空中強攻撃（たたき落とし）発動時のみ実行する
	if (!m_IsAirFallAttack) return;

	//===空中強攻撃の滞空(溜め)フェーズ===
	if (m_IsAirFallCharging)
	{
		m_AirFallChargeTime += DeltaTime;

		//指定の滞空時間を過ぎたら落下フェーズへ移行するためフラグを折る
		if (m_AirFallChargeTime >= m_PlayerParam.AirFallChargeDuration)
		{
			m_IsAirFallCharging = false;
		}
		return;
	}

	//===空中強攻撃の落下フェーズ===
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		//XY軸の移動を殺し、真下に向かって指定速度で急降下させる
		MoveComp->Velocity = FVector(0.f, 0.f, -m_PlayerParam.AirFallSpeed);
	}
}

void UPlayer_AttackComponent::_updateAirAttackUnlock()
{
	//ジャンプ中でない、あるいは既に解禁済みの場合は処理をスキップ
	if (!IsJumping()) return;
	if (m_CanAirAttack) return;

	//ジャンプを開始したZ座標からの相対的な上昇量を計算
	const float CurrentZ = m_Player->GetActorLocation().Z;
	const float DeltaZ = CurrentZ - m_JumpStartZ;

	//設定された最低高度に達した場合に空中攻撃の入力を解禁する
	if (DeltaZ >= m_PlayerParam.AirAttackMinHeight)
	{
		m_CanAirAttack = true;
	}
}

//弱攻撃の入力処理
void UPlayer_AttackComponent::Input_AttackLight(const FInputActionValue& Value)
{
	//システム的に攻撃が許可されていない場合は弾く
	if (!m_CanAttack) return;

	//現在の状態（回避中・既に攻撃中など）から入力を受け付けるか判定
	if (!CanAcceptAttackInput()) return;

	//攻撃開始に伴い、進行中のダッシュ状態を強制終了する
	m_MovementComponent->EndDash();

	//===空中での弱攻撃処理===
	if (IsJumping())
	{
		if (!m_CanAirAttack)
		{
			//高度が足りていない場合は発動させない
			return;
		}

		//アビリティが設定されていれば空中弱攻撃1段目を発動
		if (m_Player->m_AbilityPlayer_AirAttackLight)
		{
			bool AirAttackLight = m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AirAttackLight);

			//発動に成功したら空中攻撃開始フラグを立てる
			if (AirAttackLight)
			{
				m_IsAirAttackStart = true;
			}
		}
		return;
	}

	//===地上での弱攻撃処理===
	//完全に接地していない場合は弾く
	if (!m_Player->GetCharacterMovement()->IsMovingOnGround()) return;

	//攻撃ステートへ移行
	m_IsAttack = true;

	//ジャスト回避直後、または強化攻撃状態であれば専用のワープ攻撃へ派生
	if (m_EvasiveComponent->GetIsJustEvasive() || m_Player->GetIsEnhancedAttack()) {
		JustEvasiveAttack();
	}

	//===コンボの先行入力（バッファ）処理===
	//現在攻撃モーション中で、次の入力が許可されている期間の場合
	if (m_CanBufferAttack)
	{
		//すでに次のコンボが予約済みなら重複して予約しない
		if (m_NextAttackRequested) return;

		//現在1段目を実行中なら2段目のアビリティを予約セット
		if (m_ComboIndex == 1)
		{
			m_Player->m_BufferedNextAbility = m_Player->m_AbilityPlayer_AttackLight02;
			m_NextAttackRequested = true;
			m_ComboIndex = 2; //次に発動するコンボ段数を更新
		}
		//現在2段目を実行中なら3段目のアビリティを予約セット
		else if (m_ComboIndex == 2)
		{
			m_Player->m_BufferedNextAbility = m_Player->m_AbilityPlayer_AttackLight03;
			m_NextAttackRequested = true;
			m_ComboIndex = 0; //3段目でコンボが終了するためインデックスをリセット
		}
		return;
	}

	//コンボ継続中ではない（初撃の）場合は弱攻撃1段目を直接発動する
	if (m_Player->m_AbilityPlayer_AttackLight01)
	{
		m_ComboIndex = 1; //1段目を発動したことを記録
		m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AttackLight01);
	}
}

//強攻撃の入力処理
void UPlayer_AttackComponent::Input_AttackHeavy(const FInputActionValue& Value)
{
	if (!m_CanAttack) return;

	//攻撃を受け付けない状態、または既に別の攻撃モーション中なら弾く
	if (!CanAcceptAttackInput()) return;
	if (m_IsAttack) return;

	//アクション実行をプレイヤー本体へ通知
	m_Player->OnActionCommitted(EPlayerActionCommit::OtherAttack);

	//ダッシュ終了および回避の禁止
	m_MovementComponent->EndDash();
	m_EvasiveComponent->SetCanEvasive(false);

	//===空中での強攻撃処理===
	if (IsJumping())
	{
		if (!m_CanAirAttack)
		{
			//高度不足のため発動不可
			return;
		}

		//アビリティが設定されていれば発動を試みる
		if (m_Player->m_AbilityPlayer_AirAttackHeavy)
		{
			bool AirAttackHeavy = m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AirAttackHeavy);

			//発動成功時、たたき落とし専用の落下処理を開始する
			if (AirAttackHeavy)
			{
				AirFallAttack();
				m_IsAirAttackStart = true;
			}
		}
		return;
	}

	//===地上での強攻撃処理===
	if (!m_Player->GetCharacterMovement()->IsMovingOnGround()) return;

	//ジャスト回避からの派生の場合、即座に最大溜め（ロング）状態へ移行させるフラグを立てる
	if (m_EvasiveComponent && m_EvasiveComponent->GetIsJustEvasive())
	{
		m_JustEvasiveLongCharge = true;
	}

	//溜め計測の初期化と開始
	m_IsHeavyCharging = true;
	m_HeavyChargeTime = 0.f;

	//まだ強攻撃が開始されていなければ、溜め用のアビリティを起動する
	if (m_Player->m_AbilityPlayer_AttackHeavyCharge && !m_HeavyAttackStart)
	{
		m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AttackHeavyCharge);
	}
}

//強攻撃の溜め入力解放（ボタンを離した）時の処理
void UPlayer_AttackComponent::Completed_AttackHeavy(const FInputActionValue& Value)
{
	if (!GetCanAttack()) return;

	//カメラ揺れなどの演出が残っていれば終了させる
	if (m_CameraComponent)
	{
		m_CameraComponent->CameraShakEnd();
	}

	//溜め中でなければ処理しない（単発押しや既に解放済みの場合など）
	if (!m_IsHeavyCharging) return;

	//溜め状態を解除し、強攻撃の実行フェーズへ移行
	m_IsHeavyCharging = false;
	m_IsAttack = false;
	m_HeavyAttackStart = true;

	//ジャスト回避派生による即ロングチャージ要求があればワープ攻撃処理を挟む
	if (m_JustEvasiveLongCharge)
	{
		JustEvasiveAttack();
	}

	//溜め解放に紐づく実際のアタックアビリティを発動
	m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AttackHeavy);

	//次回の溜めに備えて時間をリセット
	m_HeavyChargeTime = 0.f;
}

//ジャスト回避成功時専用のワープ攻撃処理
void UPlayer_AttackComponent::JustEvasiveAttack()
{
	//UI演出の非表示
	m_EvasiveComponent->HideJustEvasiveUI();

	//回避対象となった攻撃の主（アタッカー）を取得
	const AActor* AttackerActor = m_Player->GetJustEvasive_Attacker();
	if (!IsValid(AttackerActor)) return;

	//距離の取得など固有関数を呼ぶためAEnemyBaseにキャスト
	const AEnemyBase* TargetEnemy = Cast<AEnemyBase>(AttackerActor);
	if (!TargetEnemy) return;

	//ワープ攻撃中は無敵状態を付与する
	m_Player->SetInvincible(true);

	//自機と対象の座標を取得
	FVector AttackerLocation = TargetEnemy->GetActorLocation();
	FVector PlayerLocation = m_Player->GetActorLocation();

	//プレイヤーから敵へ向かう方向ベクトルを算出
	FVector DirectionToEnemy = (AttackerLocation - PlayerLocation).GetSafeNormal();

	//空中にワープするのを防ぐため、方向ベクトルのZ軸を潰して水平にする
	DirectionToEnemy.Z = 0.f;
	DirectionToEnemy.Normalize();

	//敵側で定義されている「接近時の適切な距離」を取得
	float WarpDist = TargetEnemy->GetWarpOffsetDistance();

	//敵の位置から、プレイヤーの方向へ適切な距離だけ戻した位置を目標座標とする
	FVector TargetLocation = AttackerLocation - (DirectionToEnemy * WarpDist);

	//高さは現在のプレイヤーのものを維持
	TargetLocation.Z = PlayerLocation.Z;

	//計算した座標へワープ移動させる（めり込み防止コリジョン考慮なしで強制移動）
	m_Player->SetActorLocation(TargetLocation, false);

	//ワープ後、敵の方を向かせる
	m_Player->SetActorRotation(DirectionToEnemy.Rotation());
}

//空中攻撃に伴う滞空処理の開始
void UPlayer_AttackComponent::AirAttackStart()
{
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		//ムーブメントを飛行モードに変更し、重力と現在の速度を0にして空中でピタッと止める
		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->GravityScale = 0.f;
		MoveComp->Velocity = FVector::ZeroVector;
	}
}

//空中攻撃の滞空処理の終了
void UPlayer_AttackComponent::AirAttackEnd()
{
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		//飛行モードを解除して落下モードに戻し、重力スケールも通常値に復帰させる
		MoveComp->SetMovementMode(MOVE_Falling);
		MoveComp->GravityScale = m_PlayerParam.GravityScale;
	}
}

//空中強攻撃（たたき落とし）の開始処理
void UPlayer_AttackComponent::AirFallAttack()
{
	if (!m_Player) return;

	//敵との予期せぬ衝突を防ぐため一時的にコリジョンを消去
	m_Player->DeleteCollision();

	//たたき落とし用のステートを有効化し、溜め（滞空）時間をリセット
	m_IsAirFallAttack = true;
	m_IsAirFallCharging = true;
	m_AirFallChargeTime = 0.f;

	//一旦空中で停止させる
	AirAttackStart();

	//アクションの勢いをつけるため、Z軸方向にのみ急上昇の速度を与える
	FVector RiseVelocity = m_Player->GetCharacterMovement()->Velocity;
	RiseVelocity.Z = AirFallRiseVelocityZ;
	m_Player->GetCharacterMovement()->Velocity = RiseVelocity;
}

//空中強攻撃（たたき落とし）の終了・着地処理
void UPlayer_AttackComponent::AirFallAttackEnd()
{
	//消去していたコリジョンを復活させる
	m_Player->RevivalCollision();

	//たたき落とし系のステートを解除
	m_IsAirFallAttack = false;
	m_IsAirFallCharging = false;

	//再度ジャンプするまで空中攻撃を禁止する
	m_CanAirAttack = false;
	m_JumpStartZ = 0.f;

	//飛行モードなどを解除して通常の落下モードへ
	AirAttackEnd();
}

//空中での弱攻撃（ダッシュ斬り）処理
void UPlayer_AttackComponent::AirDashAttack()
{
	//===安全確認と初期化===
	if (!m_Player) return;

	m_IsAirDashAttack = true;
	//移動中の引っ掛かりを防止するためコリジョンを一時消去
	m_Player->DeleteCollision();

	//===入力情報の取得===
	FVector MoveDirection = FVector::ZeroVector;
	m_MovementComponent->GetDesiredMoveDirection(MoveDirection);
	const bool bHasMoveInput = m_MovementComponent->GetIsMoveInput();

	//===基準となる進行方向（BaseForward）の決定===
	FVector BaseForward;

	if (bHasMoveInput && !MoveDirection.IsNearlyZero())
	{
		//スティック等の入力がある場合は、その方向を水平化して進行基準にする
		MoveDirection.Z = 0.f;
		BaseForward = MoveDirection.GetSafeNormal();
	}
	else
	{
		//入力がない場合は、現在プレイヤーが向いている方向を基準にする
		BaseForward = m_Player->GetActorForwardVector();
		BaseForward.Z = 0.f;
		BaseForward.Normalize();
	}

	//===ターゲット候補の取得===
	const FVector PlayerLocation = m_Player->GetActorLocation();
	const AEnemyBase* TargetEnemy = nullptr; //最終的に攻撃対象とする敵

	//ワールド全体を管理する敵マネージャーを取得
	UEnemyManager* EnemyManager = m_Player->GetWorld()->GetSubsystem<UEnemyManager>();
	if (EnemyManager)
	{
		//プレイヤー座標から一番近い敵を取得（死体が含まれる可能性があることに注意）
		const AEnemyBase* ClosestEnemy = EnemyManager->GetClosestActiveEnemyFromCoordinates(PlayerLocation);

		//敵が見つかった場合の詳細な検証
		if (ClosestEnemy)
		{
			//完全に死んでいる、あるいは死亡演出中の敵は除外する
			const bool bIsAlive = !ClosestEnemy->GetIsDying() && ClosestEnemy->GetIsActive();

			if (bIsAlive)
			{
				//プレイヤーから敵への相対ベクトルと距離を計算
				const FVector EnemyLocation = ClosestEnemy->GetActorLocation();
				FVector ToEnemy = EnemyLocation - PlayerLocation;
				ToEnemy.Z = 0.f; //水平距離でのみ判定する
				const float Distance = ToEnemy.Size();

				if (!ToEnemy.IsNearlyZero())
				{
					//進行基準方向と敵の方向との内積を計算し、どれくらい向きが一致しているか判定
					const FVector ToEnemyNormal = ToEnemy.GetSafeNormal();
					const float Dot = FVector::DotProduct(BaseForward, ToEnemyNormal);

					//パラメーターで設定された許容角度（Cos値）
					const float DirectionThreshold = FMath::Cos(FMath::DegreesToRadians(m_PlayerParam.AttackInputAngle));

					//入力の有無によるターゲット条件の分岐
					//入力あり：指定された角度の範囲内にいて、かつ射程距離内ならOK
					//入力なし：角度は問わず、射程距離内にいれば自動で吸い付く
					const bool bCanTargetEnemy =
						(bHasMoveInput && Dot >= DirectionThreshold && Distance <= m_PlayerParam.AttackRange) ||
						(!bHasMoveInput && Distance <= m_PlayerParam.AttackRange);

					if (bCanTargetEnemy)
					{
						TargetEnemy = ClosestEnemy;
					}
				}
			}
		}
	}

	//===ターゲットとなる敵が確定した場合の吸着処理===
	if (TargetEnemy)
	{
		const FVector EnemyLocation = TargetEnemy->GetActorLocation();

		//敵の背後に回り込むため、敵の正面方向のベクトルを取得
		FVector EnemyForward = FRotationMatrix(FRotator(0.f, TargetEnemy->GetActorRotation().Yaw, 0.f)).GetUnitAxis(EAxis::X);

		//目標地点を計算（敵の座標から正面方向へ距離分マイナスした背後座標）
		FVector TargetLocation = EnemyLocation - EnemyForward * m_PlayerParam.AttackEnemyBack;
		TargetLocation.Z = EnemyLocation.Z; //高さは敵のZ座標に合わせる

		//プレイヤーを敵の方へ振り向かせる
		FVector LookDirection = EnemyLocation - PlayerLocation;
		LookDirection.Z = 0.f;
		if (!LookDirection.IsNearlyZero())
		{
			m_Player->SetActorRotation(LookDirection.Rotation());
		}

		//計算した目標地点をメンバ変数に保存し、移動追従フラグを立てる
		m_AttackTargetLocation = TargetLocation;
		m_HasAttackTargetLocation = true;
		return;
	}

	//===ターゲットがいない場合（空振り・空中移動のみ）の処理===
	//入力方向または正面方向（BaseForward）へ向かってそのまま飛んでいく

	//ダッシュ時に少し斜め下へ向かうための下降角度をラジアン変換
	const float DownRad = FMath::DegreesToRadians(m_PlayerParam.AirDashDownAngle);

	//前方へのベクトルと下方向へのベクトルを合成し、進行ベクトルを作成
	FVector DashDirection = (BaseForward * FMath::Cos(DownRad)) + (FVector::DownVector * FMath::Sin(DownRad));

	//固定距離分移動した先を目標地点とする
	FVector TargetLocation = PlayerLocation + DashDirection * m_PlayerParam.AirDashDistance;

	//進行方向へプレイヤーを向ける
	m_Player->SetActorRotation(BaseForward.Rotation());

	//計算した目標地点を保存し、移動追従フラグを立てる
	m_AttackTargetLocation = TargetLocation;
	m_HasAttackTargetLocation = true;
}

//地上攻撃での間合い詰め（ステップ）開始処理
void UPlayer_AttackComponent::AttackFirstStepBegin()
{
	if (!m_Player) return;

	//ジャスト回避派生の場合はワープによる移動を行うため、ステップ処理はスキップする
	if (m_EvasiveComponent->GetIsJustEvasive()) return;

	m_HasAttackTargetLocation = false;
	const FVector PlayerLocation = m_Player->GetActorLocation();

	//現在の入力方向と、入力の有無を取得
	FVector MoveDirection = FVector::ZeroVector;
	m_MovementComponent->GetDesiredMoveDirection(MoveDirection);
	const bool bHasMoveInput = m_MovementComponent->GetIsMoveInput();

	//敵を自動検索してターゲット設定を試みる。成功した場合はここで処理終了
	if (TryTargetAutoSearch(PlayerLocation, MoveDirection, bHasMoveInput)) return;

	//ターゲットが見つからなかった場合は前方への空振りステップ処理を行う
	TargetForward(PlayerLocation, MoveDirection, bHasMoveInput);
}

//敵の自動検索とロックオン処理
bool UPlayer_AttackComponent::TryTargetAutoSearch(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput)
{
	//別の方向へ入力があった場合は、以前の敵へのソフトロック状態を解除する
	if (bHasMoveInput) ClearLockedAttackTarget();

	//入力がなく、すでにロック済みの敵がいれば、そのままその敵を目標にする
	if (!bHasMoveInput && HasLockedAttackTarget())
	{
		const AEnemyBase* Enemy = m_LockedAttackTarget.Get();
		//敵への方向ベクトル（水平のみ）
		FVector ToEnemy = (Enemy->GetActorLocation() - PlayerLocation).GetSafeNormal2D();

		//敵のコリジョンにめり込まない位置を目標座標として算出
		FVector TargetLocation = Enemy->GetActorLocation() - (ToEnemy * Enemy->GetWarpOffsetDistance());
		ApplyTargetLocation(TargetLocation, ToEnemy);

		//カメラを敵の方へ向ける
		if (m_CameraComponent) m_CameraComponent->OnJEnemyDirection(Enemy, false);
		return true;
	}

	//敵マネージャーから新規に一番近い敵を検索する
	UEnemyManager* EnemyManager = m_Player->GetWorld()->GetSubsystem<UEnemyManager>();
	if (!EnemyManager) return false;

	const AEnemyBase* ClosestEnemy = EnemyManager->GetClosestActiveEnemyFromCoordinates(PlayerLocation);

	//敵が存在しないか、死んでいる等の理由でアクティブでなければ失敗
	if (!ClosestEnemy || !ClosestEnemy->GetIsActive()) return false;

	FVector EnemyLocation = ClosestEnemy->GetActorLocation();
	FVector DirectionToEnemy = (EnemyLocation - PlayerLocation).GetSafeNormal2D();
	float Distance = FVector::Dist2D(PlayerLocation, EnemyLocation);

	//設定された攻撃の射程範囲外であれば無視する
	if (Distance > m_PlayerParam.AttackRange) return false;

	//入力がある場合は、入力方向と敵のいる方向の角度をチェックする
	if (bHasMoveInput)
	{
		//カメラの視界に入っていない敵には吸い付かない
		if (!m_CameraComponent->IsLocationInCameraView(EnemyLocation)) return false;

		FVector MoveDirNorm = MoveDirection.GetSafeNormal2D();
		float Dot = FVector::DotProduct(MoveDirNorm, DirectionToEnemy);
		float DirectionThreshold = FMath::Cos(FMath::DegreesToRadians(m_PlayerParam.AttackInputAngle));

		//入力方向と敵の方向が許容角度以上ズレている場合は失敗
		if (Dot < DirectionThreshold) return false;
	}

	//すべての条件をクリアしたため、対象をロックオンする
	SetLockedAttackTarget(ClosestEnemy);

	//目標座標を計算し適用
	FVector TargetLocation = EnemyLocation - (DirectionToEnemy * ClosestEnemy->GetWarpOffsetDistance());
	ApplyTargetLocation(TargetLocation, DirectionToEnemy);

	//カメラを対象に向ける
	if (m_CameraComponent) m_CameraComponent->OnJEnemyDirection(ClosestEnemy, false);

	return true;
}

//ターゲットがいない空振り時の移動先設定
void UPlayer_AttackComponent::TargetForward(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput)
{
	//入力があればその方向、なければプレイヤーの現在の正面方向を取得
	FVector Forward = (bHasMoveInput && !MoveDirection.IsNearlyZero())
		? MoveDirection.GetSafeNormal2D()
		: m_Player->GetActorForwardVector().GetSafeNormal2D();

	//固定距離分進んだ位置を目標とする
	FVector TargetLocation = PlayerLocation + Forward * m_PlayerParam.AttackEnemyNothing;
	ApplyTargetLocation(TargetLocation, Forward);
}

//計算した目標座標とプレイヤーの向きをコンポーネントに適用する共通処理
void UPlayer_AttackComponent::ApplyTargetLocation(const FVector& TargetLocation, const FVector& DirectionToTarget)
{
	m_AttackTargetLocation = TargetLocation;
	m_AttackTargetLocation.Z = m_Player->GetActorLocation().Z; //Z座標（高さ）は現状を維持
	m_Player->SetActorRotation(DirectionToTarget.Rotation()); //目標の方向を向く
	m_HasAttackTargetLocation = true; //追従フラグをON
}

//攻撃間合い詰め（ステップ）の毎フレーム移動処理
void UPlayer_AttackComponent::AttackFirstStepTick()
{
	if (!m_Player) return;
	if (!m_HasAttackTargetLocation) return;

	//現在位置から目標位置への残りのベクトルを毎フレーム再計算し補正する
	const FVector CurrentLocation = m_Player->GetActorLocation();

	//目標位置への残りの距離ベクトル
	FVector TargetLocation = m_AttackTargetLocation - CurrentLocation;

	if (!m_IsAirDashAttack) {
		//地上ステップの場合は高さ方向を無視
		TargetLocation.Z = 0.f;
	}
	const float Distance = TargetLocation.Size();

	//残りの距離が閾値未満（ほぼ到達）なら、誤差を消すため目標位置へスナップして終了
	if (Distance < StopDistanceThreshold)
	{
		m_Player->SetActorLocation(m_AttackTargetLocation, true);
		return;
	}

	//現在の向かうべき方向
	const FVector MoveDirection = TargetLocation.GetSafeNormal();

	//フレームレートに依存しない移動量を計算
	if (m_IsAirDashAttack) {
		//空中攻撃用の移動速度を使用
		m_MoveStep = m_PlayerParam.AirAttackSpeed * GetWorld()->GetDeltaSeconds();
	}
	else {
		//地上ステップ用の移動速度を使用
		m_MoveStep = m_PlayerParam.AttackSpeed * GetWorld()->GetDeltaSeconds();
	}

	//今回のフレームでの移動予定先
	FVector NextLocation = CurrentLocation + MoveDirection * m_MoveStep;

	//移動予定量が残り距離を上回る（行き過ぎる）場合は目標位置で止める
	if (m_MoveStep >= Distance)
	{
		NextLocation = m_AttackTargetLocation;
	}

	//プレイヤーを移動させる
	m_Player->SetActorLocation(NextLocation, true);
}

//間合い詰め（ステップ）終了処理
void UPlayer_AttackComponent::AttackFirstStepEnd()
{
	//攻撃開始時に消去していた場合はコリジョンを復活させる
	m_Player->RevivalCollision();

	//強攻撃以外であれば、ステップ中に付与していた無敵状態を解除する
	if (!m_HeavyAttackStart) {
		m_Player->SetInvincible(false);
	}

	//追従フラグと目標地点をリセット
	m_HasAttackTargetLocation = false;
	m_IsAirDashAttack = false;
	m_AttackTargetLocation = FVector::ZeroVector;
}

//ターゲットを固定（ロックオン）する
void UPlayer_AttackComponent::SetLockedAttackTarget(const AEnemyBase* Enemy)
{
	m_LockedAttackTarget = Enemy;
}

//ジャンプ開始時に初期高度を保存し、空中攻撃のフラグをリセットする
void UPlayer_AttackComponent::OnJumpStarted()
{
	m_JumpStartZ = m_Player->GetActorLocation().Z;
	m_CanAirAttack = false;
}

//ロックオン対象をクリアする
void UPlayer_AttackComponent::ClearLockedAttackTarget()
{
	m_LockedAttackTarget = nullptr;
}

//有効なロックオン対象が存在するか確認する
bool UPlayer_AttackComponent::HasLockedAttackTarget() const
{
	return m_LockedAttackTarget.IsValid() && m_LockedAttackTarget->GetIsActive();
}

//===リセット関連処理===
void UPlayer_AttackComponent::ResetAttack()
{
	if (!m_Player) return;

	ResetAttackFlags();
	ResetAirAttackFlags();
	ResetMovementState();
	m_Player->RevivalCollision();
	ResetEvasiveState();
	CancelAttackAbilities();
}

//地上攻撃・コンボに関するフラグの初期化
void UPlayer_AttackComponent::ResetAttackFlags()
{
	m_CanAttack = true;
	m_CanAirAttack = true;

	//---攻撃・コンボ系---
	m_IsAttack = false;
	m_ComboIndex = 0;
	m_NextAttackRequested = false;
	m_CanBufferAttack = false;

	//---強攻撃---
	m_IsHeavyCharging = false;
	m_HeavyAttackStart = false;

	m_HeavyChargeTime = 0.f;

	//次に予約されている先行入力(バッファ)のアビリティを破棄する
	if (m_Player)
	{
		m_Player->m_BufferedNextAbility = nullptr;
	}
}

//空中攻撃に関するフラグの初期化
void UPlayer_AttackComponent::ResetAirAttackFlags()
{
	//---空中攻撃系---
	m_IsAirAttackStart = false;
	m_IsAirDashAttack = false;
	m_IsAirFallAttack = false;
	m_IsAirFallCharging = false;

	m_AirFallChargeTime = 0.f;
}

//移動ステップや状態の初期化・強制停止処理
void UPlayer_AttackComponent::ResetMovementState()
{
	//---踏み込み移動---
	m_HasAttackTargetLocation = false;
	m_AttackTargetLocation = FVector::ZeroVector;
	m_MoveStep = 0.f;

	//---移動状態を強制的に通常へ戻す---
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->GravityScale = m_PlayerParam.GravityScale;

		//飛行状態なら落下モードに書き換える
		if (MoveComp->MovementMode == MOVE_Flying)
		{
			MoveComp->SetMovementMode(MOVE_Falling);
		}
	}
}

//回避状態の初期化
void UPlayer_AttackComponent::ResetEvasiveState()
{
	//---回避再許可---
	if (m_EvasiveComponent)
	{
		m_EvasiveComponent->SetCanEvasive(true);
	}
}

//実行中の攻撃アビリティの強制キャンセル
void UPlayer_AttackComponent::CancelAttackAbilities()
{
	if (UAbilitySystemComponent* ASC = m_Player->GetAbilitySystemComponent())
	{
		ASC->CancelAbilities(nullptr, nullptr);
	}
}

//現在何らかの攻撃ステートにいるかどうかを一括で判定する
bool UPlayer_AttackComponent::GetIsInAttackState() const
{
	return m_IsAirAttackStart || m_IsHeavyCharging || m_HeavyAttackStart || m_IsAirDashAttack || m_IsAirFallAttack;
}

//各種フラグから、新規の攻撃入力を受け付けてよい状態か判定する
bool UPlayer_AttackComponent::CanAcceptAttackInput() const
{
	//システム的に禁止されている
	if (!m_CanAttack) return false;
	//空中攻撃実行中
	if (m_IsAirAttackStart) return false;
	//空中ダッシュ斬り中
	if (m_IsAirDashAttack) return false;
	//強攻撃実行中
	if (m_HeavyAttackStart) return false;
	//強攻撃溜め中
	if (m_IsHeavyCharging) return false;
	//空中たたき落とし実行中
	if (m_IsAirFallAttack) return false;
	//回避アクション中
	if (m_EvasiveComponent->GetIsEvasive()) return false;

	return true;
}

//プレイヤーがジャンプ中（空中）か判定する
bool UPlayer_AttackComponent::IsJumping() const
{
	return m_MovementComponent && m_MovementComponent->GetIsJump();
}
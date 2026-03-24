// Fill out your copyright notice in the Description page of Project Settings.


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


// Sets default values for this component's properties
UPlayer_AttackComponent::UPlayer_AttackComponent()
	:m_Player(nullptr)
	, m_MovementComponent(nullptr)
	, m_EvasiveComponent(nullptr)
	, m_CameraComponent(nullptr)
	, m_ElectroComponent(nullptr)
	, m_CanAttack(true)
	, m_ComboIndex(0)
	, m_NextAttackRequested(false)
	, m_CanBufferAttack(false)
	, m_IsAirAttackStart(false)
	, m_HasAttackTargetLocation(false)
	, m_MoveStep(0.f)
	, m_IsAirDashAttack(false)
	, m_IsAirFallAttack(false)
	, m_IsAirFallCharging(false)
	, m_IsHeavyCharging(false)
	, m_HeavyChargeTime(0.f)
	, m_AirFallChargeTime(0.f)
	, m_HeavyAttackStart(false)
	, m_JumpStartZ(0.f)
	, m_CanAirAttack(false)
	, m_JustEvasiveLongCharge(false)
	, m_AttackTargetLocation(FVector::ZeroVector)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayer_AttackComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player)		return;

	m_MovementComponent = m_Player->FindComponentByClass<UPlayer_MovementComponent>();
	if (!m_MovementComponent)return;

	m_EvasiveComponent = m_Player->FindComponentByClass<UPlayer_EvasiveComponent>();
	if (!m_EvasiveComponent)return;

	m_CameraComponent = m_Player->FindComponentByClass<UPlayer_CameraComponent>();
	if (!m_CameraComponent)return;

	m_ElectroComponent = m_Player->FindComponentByClass<UPlayer_ElectroGaugeComponent>();
	if (!m_ElectroComponent)return;

	// ...

}


// Called every frame
void UPlayer_AttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	_updateAttackHeavy(DeltaTime);

	_updateAirAttackUnlock();
	// ...
}

void UPlayer_AttackComponent::_updateAttackHeavy(float DeltaTime)
{
	//===地上の溜め===
	if (m_IsHeavyCharging)
	{
		m_HeavyChargeTime += DeltaTime;

		//最大溜め時間
		m_HeavyChargeTime = FMath::Min(m_HeavyChargeTime, PlayerParam.HeavyChargeMaxTime);

		return;
	}


	if (!m_IsAirFallAttack) return;

	// === 溜めフェーズ ===
	if (m_IsAirFallCharging)
	{
		m_AirFallChargeTime += DeltaTime;

		if (m_AirFallChargeTime >= PlayerParam.AirFallChargeDuration)
		{
			m_IsAirFallCharging = false;
		}
		return;
	}

	// === 叩きつけフェーズ ===
	if (UCharacterMovementComponent* MoveComp =
		m_Player->GetCharacterMovement())
	{
		// 真下のみ
		MoveComp->Velocity =
			FVector(0.f, 0.f, -PlayerParam.AirFallSpeed);
	}

}

void UPlayer_AttackComponent::_updateAirAttackUnlock()
{
	if (!IsJumping()) return;
	if (m_CanAirAttack) return;

	const float CurrentZ = m_Player->GetActorLocation().Z;
	const float DeltaZ = CurrentZ - m_JumpStartZ;

	if (DeltaZ >= PlayerParam.AirAttackMinHeight)
	{
		m_CanAirAttack = true;
	}
}

//弱攻撃
void UPlayer_AttackComponent::Input_AttackLight(const FInputActionValue& Value)
{
	if (!m_CanAttack)return;

	//攻撃入力を受け付けるか
	if (!CanAcceptAttackInput())return;

	//ダッシュ終了
	m_MovementComponent->EndDash();


	//空中攻撃
	if (IsJumping())
	{
		if (!m_CanAirAttack)
		{
			// 高さ不足 → 何も起きない
			return;
		}

		//空中弱１
		if (m_Player->m_AbilityPlayer_AirAttackLight)
		{
			bool AirAttackLight = m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AirAttackLight);

			//空中攻撃の挙動
			if (AirAttackLight)
			{
				m_IsAirAttackStart = true;
			}
		}
		return;
	}

	if (!m_Player->GetCharacterMovement()->IsMovingOnGround())return;

	//攻撃開始
	m_IsAttack = true;
	//m_EvasiveComponent->SetCanEvasive(false);

	if (m_EvasiveComponent->GetIsJustEvasive() || m_Player->GetIsEnhancedAttack()) {
		JustEvasiveAttack();
	}

	// コンボ予約受付中かチェック
	if (m_CanBufferAttack)
	{
		// すでに次の攻撃が予約済みであれば、これ以上何もしない
		if (m_NextAttackRequested) return;

		// 現在1段目の攻撃中なら、2段目を予約する
		if (m_ComboIndex == 1)
		{
			m_Player->m_BufferedNextAbility = m_Player->m_AbilityPlayer_AttackLight02;
			m_NextAttackRequested = true;
			m_ComboIndex = 2; // 次に発動するのは2段目
		}
		// 現在2段目の攻撃中なら、3段目を予約する
		else if (m_ComboIndex == 2)
		{
			m_Player->m_BufferedNextAbility = m_Player->m_AbilityPlayer_AttackLight03;
			m_NextAttackRequested = true;
			m_ComboIndex = 0; // 3段目でコンボ終了（またはループ）のためリセット
		}

		return;
	}

	//予約できない（コンボ中ではない）場合は弱攻撃1段目を発動
	if (m_Player->m_AbilityPlayer_AttackLight01)
	{
		m_ComboIndex = 1; // 1段目を発動したことを記録
		m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AttackLight01);
	}
}

void UPlayer_AttackComponent::Input_AttackHeavy(const FInputActionValue& Value)
{
	if (!m_CanAttack)return;


	//攻撃入力を受け付けるか
	if (!CanAcceptAttackInput())return;
	if (m_IsAttack) return;

	

	m_Player->OnActionCommitted(EPlayerActionCommit::OtherAttack);

	//ダッシュ終了
	m_MovementComponent->EndDash();
	m_EvasiveComponent->SetCanEvasive(false);

	//空中攻撃
	if (IsJumping())
	{
		if (!m_CanAirAttack)
		{
			// 高さ不足 → 何も起きない
			return;
		}
		//空中弱１
		if (m_Player->m_AbilityPlayer_AirAttackHeavy)
		{
			bool AirAttackLight = m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AirAttackHeavy);

			//空中攻撃強の挙動
			if (AirAttackLight)
			{
				AirFallAttack();
				m_IsAirAttackStart = true;
			}
		}
		return;
	}

	if (!m_Player->GetCharacterMovement()->IsMovingOnGround())return;

	//ジャスト回避中に強攻撃入力されたら即ロング
	if (m_EvasiveComponent && m_EvasiveComponent->GetIsJustEvasive())
	{
		m_JustEvasiveLongCharge = true;
	}

	m_IsHeavyCharging = true;
	m_HeavyChargeTime = 0.f;

	if (m_Player->m_AbilityPlayer_AttackHeavyCharge && !m_HeavyAttackStart)
	{
		//溜め開始
		// 長押し強攻撃
		m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AttackHeavyCharge);
	}
}

//溜め離したとき
void UPlayer_AttackComponent::Completed_AttackHeavy(const FInputActionValue& Value)
{
	if (!GetCanAttack())return;

	if (m_CameraComponent)
	{
		m_CameraComponent->CameraShakEnd();
	}

	if (!m_IsHeavyCharging)return;


	m_IsHeavyCharging = false;
	m_IsAttack = false;
	m_HeavyAttackStart = true;
	//m_Player->SetIsEnhancedAttack(true);

	if (m_JustEvasiveLongCharge)
	{
		JustEvasiveAttack();
	}

	//長押し強攻撃
	m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_AttackHeavy);


	m_HeavyChargeTime = 0.f;
}

//ジャスト回避攻撃
void UPlayer_AttackComponent::JustEvasiveAttack()
{
	m_EvasiveComponent->HideJustEvasiveUI();

	// まずAActorとして取得
	const AActor* AttackerActor = m_Player->GetJustEvasive_Attacker();
	if (!IsValid(AttackerActor)) return;

	// AEnemyBase型にキャスト（変換）して、専用の距離関数を呼べるようにする
	const AEnemyBase* TargetEnemy = Cast<AEnemyBase>(AttackerActor);
	if (!TargetEnemy) return; 

	//無敵状態
	m_Player->SetInvincible(true);
	// 敵の位置
	FVector AttackerLocation = TargetEnemy->GetActorLocation();
	FVector PlayerLocation = m_Player->GetActorLocation();

	// プレイヤーから敵への向き（方向ベクトル）を計算
	FVector DirectionToEnemy = (AttackerLocation - PlayerLocation).GetSafeNormal();

	// 高さは無視して水平方向のみの向きにする場合（推奨）
	DirectionToEnemy.Z = 0.f;
	DirectionToEnemy.Normalize();

	float WarpDist = TargetEnemy->GetWarpOffsetDistance();

	// 目標地点の計算
	FVector TargetLocation = AttackerLocation - (DirectionToEnemy * WarpDist);

	// 高さはプレイヤー維持
	TargetLocation.Z = PlayerLocation.Z;

	// ワープ（コリジョン考慮あり）
	m_Player->SetActorLocation(TargetLocation, false);

	// 敵の方向を向く
	m_Player->SetActorRotation(DirectionToEnemy.Rotation());
}



//空中停滞開始
void UPlayer_AttackComponent::AirAttackStart()
{
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->GravityScale = 0.f;
		MoveComp->Velocity = FVector::ZeroVector;
	}
}

//空中停滞終了
void UPlayer_AttackComponent::AirAttackEnd()
{
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Falling);     // 飛行解除して落下モードに戻す
		MoveComp->GravityScale = PlayerParam.GravityScale;               // 重力を通常値に戻す

	}
}

//強空中攻撃発動
void UPlayer_AttackComponent::AirFallAttack()
{
	if (!m_Player) return;

	m_Player->DeleteCollision();

	m_IsAirFallAttack = true;
	m_IsAirFallCharging = true;
	m_AirFallChargeTime = 0.f;

	AirAttackStart();

	FVector RiseVelocity = m_Player->GetCharacterMovement()->Velocity;
	RiseVelocity.Z = 400.f;
	m_Player->GetCharacterMovement()->Velocity = RiseVelocity;

}

void UPlayer_AttackComponent::AirFallAttackEnd()
{
	m_Player->RevivalCollision();

	m_IsAirFallAttack = false;
	m_IsAirFallCharging = false;

	m_CanAirAttack = false;
	m_JumpStartZ = 0.f;

	AirAttackEnd();
}


void UPlayer_AttackComponent::AirDashAttack()
{
	// ===安全確認と初期化 ===
	if (!m_Player) return;

	m_IsAirDashAttack = true;
	m_Player->DeleteCollision();

	// ===入力情報の取得 ===
	FVector MoveDirection = FVector::ZeroVector;
	m_MovementComponent->GetDesiredMoveDirection(MoveDirection);
	const bool bHasMoveInput = m_MovementComponent->GetIsMoveInput();

	// ===基準となる進行方向（BaseForward）の決定 ===
	FVector BaseForward;

	if (bHasMoveInput && !MoveDirection.IsNearlyZero())
	{
		// 入力があるなら、その方向を基準（水平化して正規化）
		MoveDirection.Z = 0.f;
		BaseForward = MoveDirection.GetSafeNormal();
	}
	else
	{
		// 入力がないなら、プレイヤーの現在正面を基準
		BaseForward = m_Player->GetActorForwardVector();
		BaseForward.Z = 0.f;
		BaseForward.Normalize();
	}


	// ===ターゲット候補の取得 ===
	const FVector PlayerLocation = m_Player->GetActorLocation();
	const AEnemyBase* TargetEnemy = nullptr; // 最終的に攻撃する敵

	// 敵マネージャー取得
	UEnemyManager* EnemyManager = m_Player->GetWorld()->GetSubsystem<UEnemyManager>();
	if (EnemyManager)
	{
		// 前方で一番近い敵を取得（※ここで死体が含まれる可能性がある）
		const AEnemyBase* ClosestEnemy = EnemyManager->GetClosestActiveEnemyFromCoordinates(PlayerLocation);

		// 敵が見つかった場合の検証
		if (ClosestEnemy)
		{
			//死にかけている（演出中など）敵はターゲットにしない
			const bool bIsAlive = !ClosestEnemy->GetIsDying() && ClosestEnemy->GetIsActive();

			if (bIsAlive)
			{
				// 敵の位置ベクトル計算
				const FVector EnemyLocation = ClosestEnemy->GetActorLocation();
				FVector ToEnemy = EnemyLocation - PlayerLocation;
				ToEnemy.Z = 0.f;
				const float Distance = ToEnemy.Size();

				if (!ToEnemy.IsNearlyZero())
				{
					// 正規化して内積（向きの一致度）を計算
					const FVector ToEnemyNormal = ToEnemy.GetSafeNormal();
					const float Dot = FVector::DotProduct(BaseForward, ToEnemyNormal);

					// 許容角度の閾値
					const float DirectionThreshold = FMath::Cos(FMath::DegreesToRadians(PlayerParam.AttackInputAngle));

					//入力優先
					//入力あり：角度（Dot）が閾値以上 かつ 距離内ならOK
					//入力なし：距離内ならOK（角度不問で吸着）
					const bool bCanTargetEnemy =
						(bHasMoveInput && Dot >= DirectionThreshold && Distance <= PlayerParam.AttackRange) ||
						(!bHasMoveInput && Distance <= PlayerParam.AttackRange);

					if (bCanTargetEnemy)
					{
						TargetEnemy = ClosestEnemy;
					}
				}
			}
		}
	}


	// ===ターゲットがいる場合の攻撃処理 ===
	if (TargetEnemy)
	{
		const FVector EnemyLocation = TargetEnemy->GetActorLocation();

		// 敵の前方向を取得（敵の背後に回り込むため）
		FVector EnemyForward = FRotationMatrix(FRotator(0.f, TargetEnemy->GetActorRotation().Yaw, 0.f)).GetUnitAxis(EAxis::X);

		// 目標地点：敵の背後
		FVector TargetLocation = EnemyLocation - EnemyForward * PlayerParam.AttackEnemyBack;
		TargetLocation.Z = EnemyLocation.Z; // 高さは敵に合わせる

		// プレイヤーを敵方向へ向ける
		FVector LookDirection = EnemyLocation - PlayerLocation;
		LookDirection.Z = 0.f;
		if (!LookDirection.IsNearlyZero())
		{
			m_Player->SetActorRotation(LookDirection.Rotation());
		}

		// ターゲット設定
		m_AttackTargetLocation = TargetLocation;
		m_HasAttackTargetLocation = true;
		return;
	}


	// ===ターゲットがいない場合（空振り・移動のみ） ===
	// 入力方向（BaseForward）へ飛ぶ

	// 下方向へ少し傾ける
	const float DownRad = FMath::DegreesToRadians(PlayerParam.AirDashDownAngle);

	// 進行ベクトル作成
	FVector DashDirection = (BaseForward * FMath::Cos(DownRad)) + (FVector::DownVector * FMath::Sin(DownRad));

	// 目標地点（固定距離移動）
	FVector TargetLocation = PlayerLocation + DashDirection * PlayerParam.AirDashDistance;

	// 向きは入力方向（水平）に合わせる
	m_Player->SetActorRotation(BaseForward.Rotation());

	m_AttackTargetLocation = TargetLocation;
	m_HasAttackTargetLocation = true;
}

//攻撃踏み込み開始
void UPlayer_AttackComponent::AttackFirstStepBegin()
{
	//プレイヤーが存在するか
	if (!m_Player) return;

	//ロック中の敵が死亡中なら即解除
	if (m_LockedAttackTarget && m_LockedAttackTarget->GetIsDying())
	{
		ClearLockedAttackTarget();
	}


	if (m_EvasiveComponent->GetIsJustEvasive())return;

	m_HasAttackTargetLocation = false;

	//プレイヤーの位置を取得
	const FVector PlayerLocation = m_Player->GetActorLocation();



	//---ロックオン中---------------------------------------------------------------------------------
	//前方に踏み込む
	if (m_CameraComponent->GetIsTargetLockedOn())
	{
		//ロックオンカメラの回転取得
		const FRotator CameraRotation = m_CameraComponent->GetLockOnCamera()->GetComponentRotation();

		//カメラの前方ベクトルを計算
		FVector Forward = FRotationMatrix(FRotator(0.f, CameraRotation.Yaw, 0.f)).GetUnitAxis(EAxis::X);

		//目標地点ヘ踏み込む
		m_AttackTargetLocation = PlayerLocation + Forward * PlayerParam.AttackEnemyNothing;

		//踏み込みは常に水平方向
		m_AttackTargetLocation.Z = PlayerLocation.Z;
		m_HasAttackTargetLocation = true;

		//プレイヤーの向きを踏み込み方向へ回転
		m_Player->SetActorRotation(Forward.Rotation());
		return;
	}



	//===入力取得 ===
	FVector MoveDirection = FVector::ZeroVector;
	m_MovementComponent->GetDesiredMoveDirection(MoveDirection);

	//入力があるかどうか
	const bool bHasMoveInput = m_MovementComponent->GetIsMoveInput();

	//===入力なし　ロック済みの敵がいるとき ===
	if (!bHasMoveInput && HasLockedAttackTarget())
	{
		const FVector EnemyLocation = m_LockedAttackTarget->GetActorLocation();

		FVector ToEnemy = EnemyLocation - PlayerLocation;
		ToEnemy.Z = 0.f;

		//万が一ゼロ長さ対策
		if (!ToEnemy.IsNearlyZero())
		{
			ToEnemy.Normalize();
		}
		float WarpDist = m_LockedAttackTarget->GetWarpOffsetDistance();
		m_AttackTargetLocation = EnemyLocation - (ToEnemy * WarpDist);

		m_AttackTargetLocation.Z = PlayerLocation.Z;

		m_Player->SetActorRotation(ToEnemy.Rotation());
		m_HasAttackTargetLocation = true;

		//攻撃開始時にカメラを補間して敵に向ける
		if (m_CameraComponent)
		{
			m_CameraComponent->OnJEnemyDirection(m_LockedAttackTarget,false);
		}

		return;
	}

	//=== 新しい敵を探す・入力があれば既存ロック解除 ===
	if (bHasMoveInput)
	{
		ClearLockedAttackTarget();
	}


	//敵マネージャーを取得
	UEnemyManager* EnemyManager = m_Player->GetWorld()->GetSubsystem<UEnemyManager>();
	//もっとも近い敵を探す
	const AEnemyBase* NewEnemyTarget = nullptr;

	//=== 敵探索 ===
	//入力無し
	if (EnemyManager)
	{
		if (!bHasMoveInput)
		{
			//まずは候補を取得
			const AEnemyBase* CandidateEnemy = EnemyManager->GetClosestActiveEnemyFromCoordinates(PlayerLocation);

			// 候補が有効かチェック
			if (CandidateEnemy && CandidateEnemy->GetIsActive())
			{
				//距離チェックを追加
				//どんなに近くても、攻撃範囲外ならターゲットにしない
				float Dist = FVector::Dist(PlayerLocation, CandidateEnemy->GetActorLocation());

				if (Dist <= PlayerParam.AttackRange)
				{
					NewEnemyTarget = CandidateEnemy;
				}
			}

		}
		else
		{
			MoveDirection.Z = 0.f;

			if (!MoveDirection.IsNearlyZero())
			{
				MoveDirection.Normalize();

				const AEnemyBase* ClosestEnemy = EnemyManager->GetClosestActiveEnemyFromCoordinates(PlayerLocation);
				if (ClosestEnemy && ClosestEnemy->GetIsActive())
				{
					//敵の位置を取得
					const FVector EnemyLocation = ClosestEnemy->GetActorLocation();
					//敵の位置がカメラの視野内に存在するか
					if (m_CameraComponent->IsLocationInCameraView(EnemyLocation))
					{
						//プレイヤーから敵への水平ベクトル
						FVector DirectionToEnemy = EnemyLocation - PlayerLocation;
						DirectionToEnemy.Z = 0.f;
						//敵との距離
						const float Distance = DirectionToEnemy.Size();

						// 敵が間合い内なら敵方向へ
						if (Distance <= PlayerParam.AttackRange)
						{
							//敵方向ベクトル正規化
							DirectionToEnemy.Normalize();

							//入力方向との角度判定
							const float Dot = FVector::DotProduct(MoveDirection, DirectionToEnemy);

							//入力があるときの敵を見つける角度
							const float DirectionThreshold = FMath::Cos(FMath::DegreesToRadians(PlayerParam.AttackInputAngle));
							if (Dot >= DirectionThreshold)
							{
								NewEnemyTarget = ClosestEnemy;
							}
						}
					}
				}

			}
		}
	}
	//=== 敵が決定・ロック ===
	if (NewEnemyTarget && NewEnemyTarget->GetIsActive())
	{
		SetLockedAttackTarget(NewEnemyTarget);

		const FVector EnemyLocation = NewEnemyTarget->GetActorLocation();

		FVector ToEnemy = EnemyLocation - PlayerLocation;
		ToEnemy.Z = 0.f;

		//万が一ゼロ長さ対策
		if (!ToEnemy.IsNearlyZero())
		{
			ToEnemy.Normalize();
		}

		float WarpDist = NewEnemyTarget->GetWarpOffsetDistance();
		m_AttackTargetLocation = EnemyLocation - (ToEnemy * WarpDist);

		m_AttackTargetLocation.Z = PlayerLocation.Z;

		m_Player->SetActorRotation(ToEnemy.Rotation());
		m_HasAttackTargetLocation = true;

		//攻撃開始時にカメラを補間して敵に向ける
		if (m_CameraComponent)
		{
			m_CameraComponent->OnJEnemyDirection(NewEnemyTarget,false);
		}
		return;

	}

	//=== 敵なし ===
	FVector Forward;

	if (bHasMoveInput && !MoveDirection.IsNearlyZero())
	{
		Forward = MoveDirection.GetSafeNormal();
	}
	else
	{
		Forward = m_Player->GetActorForwardVector();
	}

	m_AttackTargetLocation = PlayerLocation + Forward * PlayerParam.AttackEnemyNothing;
	m_AttackTargetLocation.Z = PlayerLocation.Z;

	m_Player->SetActorRotation(Forward.Rotation());
	m_HasAttackTargetLocation = true;
}

//攻撃踏み込み移動
void UPlayer_AttackComponent::AttackFirstStepTick()
{
	if (!m_Player) return;
	if (!m_HasAttackTargetLocation) return;

	// 現在位置から目標位置へのベクトルを毎フレーム再計算 補正
	const FVector CurrentLocation = m_Player->GetActorLocation();

	//目標位置
	FVector TargetLocation =
		m_AttackTargetLocation - CurrentLocation;

	if (!m_IsAirDashAttack) {
		// 高さ方向は無視
		TargetLocation.Z = 0.f;
		//UE_LOG(LogTemp, Warning, TEXT("TargetLocation.Z = 0.f"));
	}
	const float Distance = TargetLocation.Size();

	// ほぼ到達していたら停止
	if (Distance < 5.f)
	{
		m_Player->SetActorLocation(m_AttackTargetLocation, true);
		return;
	}

	// 進行方向
	const FVector MoveDirection = TargetLocation.GetSafeNormal();

	// フレーム依存しない移動量
	//空中攻撃であればより早く
	if (m_IsAirDashAttack) {
		m_MoveStep = PlayerParam.AirAttackSpeed * GetWorld()->GetDeltaSeconds();
	}
	else {
		m_MoveStep = PlayerParam.AttackSpeed * GetWorld()->GetDeltaSeconds();
	}

	FVector NextLocation = CurrentLocation + MoveDirection * m_MoveStep;

	// 行き過ぎ防止
	if (m_MoveStep >= Distance)
	{
		NextLocation = m_AttackTargetLocation;
	}

	m_Player->SetActorLocation(NextLocation, true);
}

//間合い詰め終了
void UPlayer_AttackComponent::AttackFirstStepEnd()
{
	m_Player->RevivalCollision();

	//無敵状態
	if (!m_HeavyAttackStart) {
		m_Player->SetInvincible(false);

	}


	m_HasAttackTargetLocation = false;
	m_IsAirDashAttack = false;
	m_AttackTargetLocation = FVector::ZeroVector;
}

void UPlayer_AttackComponent::SetLockedAttackTarget(const AEnemyBase* Enemy)
{
	m_LockedAttackTarget = Enemy;
}

void UPlayer_AttackComponent::OnJumpStarted()
{
	m_JumpStartZ = m_Player->GetActorLocation().Z;
	m_CanAirAttack = false;
}

void UPlayer_AttackComponent::ClearLockedAttackTarget()
{
	m_LockedAttackTarget = nullptr;
}

bool UPlayer_AttackComponent::HasLockedAttackTarget() const
{
	return m_LockedAttackTarget && m_LockedAttackTarget->GetIsActive();
}


//===リセット===
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

void UPlayer_AttackComponent::ResetAttackFlags()
{
	m_CanAttack = true;
	m_CanAirAttack = true;

	// --- 攻撃・コンボ系 ---
	m_IsAttack = false;
	m_ComboIndex = 0;
	m_NextAttackRequested = false;
	m_CanBufferAttack = false;

	//--- 強攻撃 ---
	m_IsHeavyCharging = false;
	m_HeavyAttackStart = false;

	m_HeavyChargeTime = 0.f;


	//m_Player->SetIsEnhancedAttack(false);

	// 次に予約されている Ability を破棄
	if (m_Player)
	{
		m_Player->m_BufferedNextAbility = nullptr;
	}
}

void UPlayer_AttackComponent::ResetAirAttackFlags()
{
	// --- 空中攻撃系 ---
	m_IsAirAttackStart = false;
	m_IsAirDashAttack = false;
	m_IsAirFallAttack = false;
	m_IsAirFallCharging = false;


	m_AirFallChargeTime = 0.f;
}

void UPlayer_AttackComponent::ResetMovementState()
{
	// --- 踏み込み移動 ---
	m_HasAttackTargetLocation = false;
	m_AttackTargetLocation = FVector::ZeroVector;
	m_MoveStep = 0.f;

	// --- 移動状態を強制的に通常へ ---
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->GravityScale = PlayerParam.GravityScale;

		if (MoveComp->MovementMode == MOVE_Flying)
		{
			MoveComp->SetMovementMode(MOVE_Falling);
		}
	}
}


void UPlayer_AttackComponent::ResetEvasiveState()
{
	// --- 回避再許可 ---
	if (m_EvasiveComponent)
	{
		m_EvasiveComponent->SetCanEvasive(true);
	}
}

void UPlayer_AttackComponent::CancelAttackAbilities()
{
	if (UAbilitySystemComponent* ASC = m_Player->GetAbilitySystemComponent())
	{
		ASC->CancelAbilities(nullptr, nullptr);
	}
}


bool UPlayer_AttackComponent::GetIsInAttackState() const
{
	return 	m_IsAirAttackStart || m_IsHeavyCharging || m_HeavyAttackStart || m_IsAirDashAttack || m_IsAirFallAttack;
}


bool UPlayer_AttackComponent::CanAcceptAttackInput() const
{
	if (!m_CanAttack) return false;
	if (m_IsAirAttackStart)return false;
	if (m_IsAirDashAttack)return false;
	if (m_HeavyAttackStart) return false;
	if (m_IsHeavyCharging) return false;
	if (m_IsAirFallAttack)return false;
	if (m_EvasiveComponent->GetIsEvasive())return false;
	return true;
}

bool UPlayer_AttackComponent::IsJumping() const
{
	return m_MovementComponent && m_MovementComponent->GetIsJump();
}
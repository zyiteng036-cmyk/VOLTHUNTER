#include "Player_EvasiveComponent.h"
#include "Player_MovementComponent.h"
#include "Player_AttackComponent.h"
#include "Player_CameraComponent.h"
#include "Camera/CameraComponent.h"
#include "Player_ElectroGaugeComponent.h"
#include "Player_SkillComponent.h"
#include "../HitStopComponent/HitStopComponent.h"
#include "../PlayerCharacter.h"
#include "../PlayerNotifySubSystem/PlayerNotifySubsystem.h"
#include "../CollisionChannelName.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "../AttackCollisionDetection/AttackCollisionDetection.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"

//===マジックナンバー排除用の定数===
namespace EvasiveConstants
{
	//スローモーションがこの秒数(実時間)以上続いたら強制解除する
	constexpr float SafetySlowMotionLimit = 3.0f;
	//スローモーション判定の上限値
	constexpr float TimeDilationUpperThreshold = 0.95f;
	//スローモーション判定の下限値
	constexpr float TimeDilationLowerThreshold = 0.05f;
	//通常の時間の進み具合(等倍)
	constexpr float NormalTimeDilation = 1.0f;
	//移動入力判定の閾値(二乗値)
	constexpr float MoveInputThresholdSquared = 0.1f;
	//回避コリジョンの半径
	constexpr float EvasiveCapsuleRadius = 60.0f;
	//回避コリジョンの半分の高さ
	constexpr float EvasiveCapsuleHalfHeight = 94.0f;
	//ジャスト回避時のスローモーション倍率
	constexpr float SlowMotionTimeDilation = 0.2f;
	//ジャスト回避時のスローモーション継続時間
	constexpr float SlowMotionDuration = 0.2f;
	//ジャスト回避成功時に加算する電力ゲージ量
	constexpr float ElectroGaugeAdditionValue = 15.0f;
}

UPlayer_EvasiveComponent::UPlayer_EvasiveComponent()
{
	//毎フレームのTick処理を有効化する設定
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayer_EvasiveComponent::BeginPlay()
{
	//親クラスのBeginPlayを呼び出す
	Super::BeginPlay();

	//オーナーアクターをプレイヤーキャラクターとしてキャストして取得
	m_Player = Cast<APlayerCharacter>(GetOwner());
	//プレイヤーが正しく取得できなかった場合は以降の処理を中断
	if (!m_Player)
	{
		return;
	}

	//ストック数をパラメータから取得して初期化
	m_CurrentEvasiveStock = m_PlayerParam.EvasiveStock;

	//各必須コンポーネントを検索して参照を保持
	m_MovementComponent = m_Player->FindComponentByClass<UPlayer_MovementComponent>();
	if (!m_MovementComponent) return;

	m_CameraComponent = m_Player->FindComponentByClass<UPlayer_CameraComponent>();
	if (!m_CameraComponent) return;

	m_AttackComponent = m_Player->FindComponentByClass<UPlayer_AttackComponent>();
	if (!m_AttackComponent) return;

	m_ElectroComponent = m_Player->FindComponentByClass<UPlayer_ElectroGaugeComponent>();
	if (!m_ElectroComponent) return;

	m_SkillComponent = m_Player->FindComponentByClass<UPlayer_SkillComponent>();
	if (!m_SkillComponent) return;
}

void UPlayer_EvasiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//親クラスのTickComponentを呼び出す
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//回避状態の更新処理を実行する
	_updateEvasive(DeltaTime);

	//スローモーション安全装置の処理
	//現在のグローバルな時間の進み具合を取得する
	float CurrentTimeDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());

	//時間が通常よりも遅くなっている(スロー中)か判定する
	if (CurrentTimeDilation < EvasiveConstants::TimeDilationUpperThreshold && CurrentTimeDilation > EvasiveConstants::TimeDilationLowerThreshold)
	{
		//スロー中はDeltaTimeも小さくなっているため実時間に変換して加算する
		//0除算防止のためCurrentTimeDilationが極端に小さい場合は計算しない
		if (CurrentTimeDilation > KINDA_SMALL_NUMBER)
		{
			//実時間を算出して監視用タイマーに加算
			m_CurrentSlowMotionWatchTime += (DeltaTime / CurrentTimeDilation);
		}
		else
		{
			//時間がほぼ停止している場合はDeltaTimeをそのまま加算する
			m_CurrentSlowMotionWatchTime += DeltaTime;
		}

		//監視タイマーが安全限界時間を超えたか判定する
		if (m_CurrentSlowMotionWatchTime >= EvasiveConstants::SafetySlowMotionLimit)
		{
			//制限時間を超えたため時間を通常速度に強制リセットする
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), EvasiveConstants::NormalTimeDilation);
			//監視用タイマーをリセットする
			m_CurrentSlowMotionWatchTime = 0.0f;

#if !UE_BUILD_SHIPPING
			//デバッグ用にエラーログを出力する
			UE_LOG(LogTemp, Error, TEXT("TimeDilation was stuck! Force reset to 1.0f by Fail-Safe."));
#endif
		}
	}
	else
	{
		//スロー中ではないため監視タイマーをリセットする
		m_CurrentSlowMotionWatchTime = 0.0f;
	}
}

void UPlayer_EvasiveComponent::_updateEvasive(float DeltaTime)
{
	//ジャスト回避中でなければ更新不要のため終了する
	if (!m_IsJustEvasive)
	{
		return;
	}

	//ジャスト回避の経過時間を加算する
	m_CurrentJustEvasiveTime += DeltaTime;

	//経過時間がジャスト回避の無敵・強化時間を過ぎたか判定する
	if (m_CurrentJustEvasiveTime >= m_JustEvasiveLimitTime)
	{
		//ジャスト回避状態を解除する
		m_IsJustEvasive = false;
		//経過時間をリセットする
		m_CurrentJustEvasiveTime = 0.0f;

		//プレイヤーが存在すれば無敵状態を解除する
		if (m_Player)
		{
			m_Player->SetInvincible(false);
		}

		//ジャスト回避用のUIを非表示にする
		HideJustEvasiveUI();

		//スキルコンポーネントが存在すればスキルの使用を再度許可する
		if (m_SkillComponent)
		{
			m_SkillComponent->SetCanSkillActive(true);
		}
	}
}

void UPlayer_EvasiveComponent::Input_Evasive(const FInputActionValue& Value)
{
	//既に回避中であれば入力を無視する
	if (m_IsEvasive)
	{
		return;
	}

	//空中判定を行う
	//プレイヤーが空中にいる場合は回避できないため終了する
	if (!m_Player->GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	//回避可能フラグの判定を行う
	//回避できない状態であれば終了する
	if (!GetCanEvasive())
	{
		return;
	}

	//プレイヤーのアクションコミットを回避状態として通知する
	m_Player->OnActionCommitted(EPlayerActionCommit::Evasive);

	//攻撃コンポーネントの状態をリセットして攻撃を中断させる
	m_AttackComponent->SetIsAttack(false);
	m_AttackComponent->SetCanBufferAttack(false);
	m_AttackComponent->SetNextAttackRequested(false);

	//ダッシュ中であればダッシュを終了させる
	if (m_MovementComponent->GetIsDash())
	{
		m_MovementComponent->EndDash();
	}
	//移動コンポーネントの通常の移動入力を無効化する
	m_MovementComponent->SetCanMovement(false);

	//回避状態フラグを有効にする
	m_IsEvasive = true;
	//回避の経過時間をリセットする
	m_EvasiveTime = 0.0f;

	//回避中はスキルの使用を不可にする
	m_SkillComponent->SetCanSkillActive(false);

	//回避方向を算出するためコントロールの回転を取得する
	FRotator EvasiveRotation = m_MovementComponent->GetControlRotationFlat();

	//回避時にプレイヤーの既存コリジョンを削除(または無効化)する
	m_Player->DeleteCollision();

	//コントロール回転から前方向ベクトルと右方向ベクトルを取得する
	FVector Forward = FRotationMatrix(EvasiveRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(EvasiveRotation).GetUnitAxis(EAxis::Y);

	//移動入力の有無を二乗値の閾値で判定する
	if (m_MovementComponent->GetCurrentMoveInput().SizeSquared() > EvasiveConstants::MoveInputThresholdSquared)
	{
		//移動入力がある場合は入力方向のベクトルを合成して正規化し回避方向とする
		m_EvasiveDirection = (Forward * m_MovementComponent->GetCurrentMoveInput().X + Right * m_MovementComponent->GetCurrentMoveInput().Y).GetSafeNormal();
	}
	else
	{
		//入力がない場合はプレイヤーの後ろ方向を回避方向とする
		m_EvasiveDirection = -m_Player->GetActorForwardVector().GetSafeNormal();
	}

	//回避の速度ベクトルを計算する
	//回避方向に設定された回避速度を掛ける
	FVector Launch = m_EvasiveDirection * m_PlayerParam.EvasiveSpeed;
	//Z軸方向に垂直方向の速度を加算する
	Launch.Z += m_PlayerParam.VerticalSpeed;
	//計算した速度を用いてキャラクターを打ち出す(LaunchCharacterで瞬間的な速度を与える)
	m_Player->LaunchCharacter(Launch, true, true);

	//アビリティシステムを用いた回避アクションの発動処理
	if (UAbilitySystemComponent* AbilitySystemComp = m_Player->GetAbilitySystemComponent())
	{
		//入力がありかつ前方等への通常回避アビリティが設定されているか判定する
		if (m_MovementComponent->GetCurrentMoveInput().SizeSquared() > EvasiveConstants::MoveInputThresholdSquared && m_Player->m_AbilityPlayer_Evasive)
		{
			//通常回避アビリティをアクティベートする
			AbilitySystemComp->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_Evasive);
		}
		//入力がなくかつ後方回避アビリティが設定されているか判定する
		else if (m_Player->m_AbilityPlayer_BackEvasive)
		{
			//後方回避アビリティをアクティベートする
			AbilitySystemComp->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_BackEvasive);
		}
	}
}

void UPlayer_EvasiveComponent::CreateJustEvasiveCollision()
{
	//既にジャスト回避用のコリジョンが存在する場合は二重生成を防ぐため終了する
	if (m_JustEvasiveCollision)
	{
		return;
	}

	//回避コリジョンのコンポーネントを新規生成する
	m_JustEvasiveCollision = NewObject<UCapsuleComponent>(this);

	//まずプレイヤーの位置に合わせて配置する
	m_JustEvasiveCollision->SetWorldLocation(m_Player->GetActorLocation());
	//生成したコンポーネントをワールドに登録する
	m_JustEvasiveCollision->RegisterComponent();
	//カプセルのサイズ(半径と半分の高さ)を設定する
	m_JustEvasiveCollision->InitCapsuleSize(EvasiveConstants::EvasiveCapsuleRadius, EvasiveConstants::EvasiveCapsuleHalfHeight);
	//コリジョン判定をQueryOnly(物理シミュレーションなし、判定のみ)に設定する
	m_JustEvasiveCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//ワールド上でどのカテゴリーに属する物体かを設定する
	m_JustEvasiveCollision->SetCollisionObjectType(JustEvasive_Collision);
	//全てのチャンネルに対するデフォルトの応答を無視(Ignore)に設定する
	m_JustEvasiveCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	//攻撃判定チャンネルに対してのみOverlapで反応するように設定する
	m_JustEvasiveCollision->SetCollisionResponseToChannel(AttackDetection_Collision, ECR_Overlap);
	//オーバーラップイベントの生成を有効にする
	m_JustEvasiveCollision->SetGenerateOverlapEvents(true);

	//設定したコリジョン判定を即時反映させる
	m_JustEvasiveCollision->UpdateOverlaps();

	//オーバーラップ開始時のイベントに自身の関数をバインドする
	m_JustEvasiveCollision->OnComponentBeginOverlap.AddDynamic(this, &UPlayer_EvasiveComponent::OnJustEvasiveOverlap);
}

void UPlayer_EvasiveComponent::DestroyJustEvasiveCollision()
{
	//ジャスト回避用コリジョンが存在しない場合は削除処理をスキップする
	if (!m_JustEvasiveCollision)
	{
		return;
	}

	//コンポーネントを破棄する
	m_JustEvasiveCollision->DestroyComponent();
	//ポインタを安全のためnullにリセットする
	m_JustEvasiveCollision = nullptr;
}

void UPlayer_EvasiveComponent::OnJustEvasiveOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//相手のアクターが存在しない、または自分自身の場合は処理しない
	if (!OtherActor || OtherActor == m_Player) return;
	//既にジャスト回避中の場合は重複して処理しない
	if (m_IsJustEvasive) return;
	//プレイヤーがダメージを受けている最中であればジャスト回避を発生させない
	if (m_Player->GetIsDamage()) return;

	//ヒットストップコンポーネントを取得し判定する
	if (UHitStopComponent* HitStopComp = m_Player->FindComponentByClass<UHitStopComponent>())
	{
		//ヒットストップ中であればジャスト回避処理を行わない
		if (HitStopComp->IsHitStopActive()) return;
	}

	//重なった相手のコンポーネントが攻撃判定用コリジョンであるか判定する
	if (OtherComp && OtherComp->GetCollisionObjectType() == AttackDetection_Collision)
	{
		//ジャスト回避成功フラグを立てる
		m_IsJustEvasive = true;
		//プレイヤーを無敵状態にする
		m_Player->SetInvincible(true);
		//プレイヤーの攻撃強化状態を有効にする
		m_Player->SetIsEnhancedAttack(true);

		//ジャスト回避用のUIクラスが設定されておりコントローラーが存在するか判定する
		if (m_JustEvasiveWidgetClass && m_Player->GetController())
		{
			//既にUIが表示されていたら一旦親から削除して初期化する
			if (m_JustEvasiveWidgetInstance)
			{
				m_JustEvasiveWidgetInstance->RemoveFromParent();
				m_JustEvasiveWidgetInstance = nullptr;
			}

			//プレイヤーコントローラーを取得する
			if (APlayerController* PC = Cast<APlayerController>(m_Player->GetController()))
			{
				//取得したコントローラーを所有者としてWidgetを生成する
				m_JustEvasiveWidgetInstance = CreateWidget<UUserWidget>(PC, m_JustEvasiveWidgetClass);
				if (m_JustEvasiveWidgetInstance)
				{
					//生成に成功したら画面(ビューポート)に追加して表示する
					m_JustEvasiveWidgetInstance->AddToViewport();
				}
			}
		}

		//スロー演出開始時に監視タイマーを0にリセットしておく
		m_CurrentSlowMotionWatchTime = 0.0f;

		//画面全体をスローモーションにする演出を開始する
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), EvasiveConstants::SlowMotionTimeDilation);

		//ワールドのポインタを取得して安全にキャプチャする
		UWorld* CurrentWorld = GetWorld();

		//スローモーション解除用のタイマーハンドルを用意する
		FTimerHandle SlowMoTimer;
		//指定した実時間後にスローモーションを解除する処理をタイマーにセットする
		CurrentWorld->GetTimerManager().SetTimer(
			SlowMoTimer,
			[CurrentWorld]()
			{
				//ワールドがまだ有効であれば時間を通常に戻す
				if (IsValid(CurrentWorld))
				{
					UGameplayStatics::SetGlobalTimeDilation(CurrentWorld, EvasiveConstants::NormalTimeDilation);
				}
			},
			EvasiveConstants::SlowMotionDuration, //スローの実時間
			false
		);

		//ジャスト回避成功時の電力ゲージ加算処理を呼び出す
		OnJustEvasiveSuccess();

		//ジャスト回避の対象となった攻撃者が有効か判定する
		if (!IsValid(m_Player->GetJustEvasive_Attacker())) return;

		//移動コンポーネントにジャスト回避の成功と攻撃者を通知する
		m_MovementComponent->OnJustEvasive(m_Player->GetJustEvasive_Attacker());
		//カメラコンポーネントに攻撃者の方を向くように通知する
		m_CameraComponent->OnJEnemyDirection(m_Player->GetJustEvasive_Attacker(), true);

		//プレイヤーが存在すればジャスト回避成功イベントを全体にブロードキャストする
		if (m_Player)
		{
			m_Player->BroadcastJustEvasiveSuccess(m_Player->GetJustEvasive_Attacker());
		}
	}
}

void UPlayer_EvasiveComponent::OnJustEvasiveSuccess()
{
	//プレイヤーが存在しない場合は終了する
	if (!m_Player) return;

	//電力ゲージコンポーネントに定数分のゲージ量を加算する
	m_ElectroComponent->AddElectroGauge(EvasiveConstants::ElectroGaugeAdditionValue);
	//電力コンポーネント側のジャスト回避成功処理を呼び出す
	m_ElectroComponent->OnJustEvasiveSuccess();
}

void UPlayer_EvasiveComponent::HideJustEvasiveUI()
{
	//生成済みのUIインスタンスが存在するか判定する
	if (m_JustEvasiveWidgetInstance)
	{
		//UIを画面から取り除く
		m_JustEvasiveWidgetInstance->RemoveFromParent();
		//ポインタを安全のためnullにリセットする
		m_JustEvasiveWidgetInstance = nullptr;
	}
}
#include "PlayerAttackComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "CustomCharacterMovementComponent.h"
#include "CameraRigComponent.h"
#include "Attack/PlayerAttackComboComponent.h"
#include "Attack/PlayerAttackTargetComponent.h"
#include "Attack/PlayerAttackMotionComponent.h"
#include "PlayerEvasiveComponent.h"
#include "PlayerElectroGaugeComponent.h"
#include "Solver/PlayerAttackSolver.h"
#include "../DataAssets/Player/PlayerAttackParameter.h"
#include "../DataAssets/Player/PlayerComboTable.h"
#include "AbilitySystemComponent.h"

namespace
{
	//攻撃Abilityタグ
	const FName Tag_Ability_Attack = TEXT("Ability.Attack");

	//攻撃中タグ
	const FName Tag_State_Attack_Active = TEXT("State.Attack.Active");

	//強攻撃溜め中タグ
	const FName Tag_State_Attack_HeavyCharging = TEXT("State.Attack.HeavyCharging");

	//強攻撃最大溜め完了タグ
	const FName Tag_State_Attack_HeavyCharged = TEXT("State.Attack.HeavyCharged");
}

//初期化
UPlayerAttackComponent::UPlayerAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	m_AttackAbilityTag = FGameplayTag::RequestGameplayTag(Tag_Ability_Attack);
	m_AttackActiveStateTag = FGameplayTag::RequestGameplayTag(Tag_State_Attack_Active);
	m_HeavyChargingStateTag = FGameplayTag::RequestGameplayTag(Tag_State_Attack_HeavyCharging);
	m_HeavyChargedStateTag = FGameplayTag::RequestGameplayTag(Tag_State_Attack_HeavyCharged, false);

}

//開始時
void UPlayerAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player) return;

	CacheComponents();
	InitializeSubComponents();
	BindSubComponentEvents();
}

//参照を取得
void UPlayerAttackComponent::CacheComponents()
{
	if (!m_Player) return;

	m_ComboComponent = m_Player->FindComponentByClass<UPlayerAttackComboComponent>();
	m_TargetComponent = m_Player->FindComponentByClass<UPlayerAttackTargetComponent>();
	m_MotionComponent = m_Player->FindComponentByClass<UPlayerAttackMotionComponent>();
	m_EvasiveComponent = m_Player->FindComponentByClass<UPlayerEvasiveComponent>();
	m_ElectroGaugeComponent = m_Player->FindComponentByClass<UPlayerElectroGaugeComponent>();

	m_CameraRigComponent = m_Player->FindComponentByClass<UCameraRigComponent>();
}

//サブコンポーネントを初期化
void UPlayerAttackComponent::InitializeSubComponents()
{
	if (m_ComboComponent)
	{
		m_ComboComponent->InitializeComboComponent(
			m_Player,
			this,
			m_ComboTable
		);
	}

	if (m_TargetComponent)
	{
		m_TargetComponent->InitializeTargetComponent(
			m_Player,
			m_AttackParameter
		);
	}

	if (m_MotionComponent)
	{
		m_MotionComponent->InitializeMotionComponent(
			m_Player,
			this,
			m_TargetComponent,
			m_AttackParameter
		);
	}
}

//サブコンポーネントの通知を登録
void UPlayerAttackComponent::BindSubComponentEvents()
{
	if (m_TargetComponent)
	{
		m_TargetComponent->OnAttackTargetChanged.AddUniqueDynamic(
			this,
			&UPlayerAttackComponent::HandleAttackTargetChanged
		);
	}

	if (m_MotionComponent)
	{
		m_MotionComponent->OnAttackMotionTargetChanged.AddUniqueDynamic(
			this,
			&UPlayerAttackComponent::HandleAttackTargetChanged
		);
	}

	if (m_ComboComponent)
	{
		m_ComboComponent->OnComboReserveWindowChanged.AddUniqueDynamic(
			this,
			&UPlayerAttackComponent::HandleComboReserveWindowChanged
		);

		m_ComboComponent->OnComboActivationWindowChanged.AddUniqueDynamic(
			this,
			&UPlayerAttackComponent::HandleComboActivationWindowChanged
		);
	}
}

//毎フレーム更新
void UPlayerAttackComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateHeavyCharge(DeltaTime);

	if (m_MotionComponent)
	{
		m_MotionComponent->TickAttackMotion(DeltaTime);
	}
}

//攻撃入力タグを処理
bool UPlayerAttackComponent::HandleAttackInputByTag(const FGameplayTag& InputTag)
{
	if (!m_ComboComponent) return false;

	const FGameplayTag LightInputTag =
		FGameplayTag::RequestGameplayTag(TEXT("Input.Attack.Light"), false);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AttackInputCacheRequest] InputTag:%s CurrentAttack:%s LastMove:%s"),
		*InputTag.ToString(),
		*GetCurrentAttackTag().ToString(),
		*m_Player->GetLastMoveInputVector().ToString()
	);

	//弱攻撃入力時は、コンボ中でも現在の移動入力を攻撃方向用に保存する
	if (
		m_Player &&
		LightInputTag.IsValid() &&
		InputTag.MatchesTagExact(LightInputTag)
		)
	{
		if (UCustomCharacterMovementComponent* MovementComponent =
			m_Player->FindComponentByClass<UCustomCharacterMovementComponent>())
		{
			MovementComponent->CacheAttackStartInputFromRaw(
				m_Player->GetLastMoveInputVector()
			);
		}
	}

	return m_ComboComponent->HandleAttackInputByTag(InputTag);
}
//弱攻撃要求
bool UPlayerAttackComponent::RequestLightAttack()
{
	if (!m_ComboComponent) return false;

	return m_ComboComponent->RequestLightAttack();
}

//強攻撃押下要求
bool UPlayerAttackComponent::RequestHeavyAttackPressed()
{
	if (!m_ComboComponent) return false;

	return m_ComboComponent->RequestHeavyAttackPressed();
}

//強攻撃離し要求
bool UPlayerAttackComponent::RequestHeavyAttackReleased()
{
	if (!m_ComboComponent) return false;

	return m_ComboComponent->RequestHeavyAttackReleased();
}

//攻撃時に敵を見るカメラを要求
void UPlayerAttackComponent::RequestAttackLookCamera(AActor* TargetActor)
{
	if (!TargetActor) return;
	if (!m_CameraRigComponent) return;

	m_CameraRigComponent->RequestAttackLookCamera(TargetActor);
}


//攻撃Ability開始
void UPlayerAttackComponent::BeginAttackByTag(
	const FGameplayTag& AttackTag,
	EPlayerActionType ActionType,
	EPlayerAttackType AttackType
)
{
	if (!m_Player) return;
	if (!AttackTag.IsValid()) return;



	ApplyAttackStartState(AttackTag,ActionType,AttackType);

	AddAttackTagsToASC(AttackTag);

	CommitPlayerAction(ActionType,AttackType);

}

//攻撃開始時の状態を設定
void UPlayerAttackComponent::ApplyAttackStartState(
	const FGameplayTag& AttackTag,
	EPlayerActionType ActionType,
	EPlayerAttackType AttackType
)
{
	m_RuntimeState.bComboTransitioning = false;
	m_RuntimeState.CurrentAttackTag = AttackTag;
	m_RuntimeState.CurrentActionType = ActionType;
	m_RuntimeState.CurrentAttackType = AttackType;
	m_RuntimeState.bIsAttack = true;

	switch (AttackType)
	{
	case EPlayerAttackType::HeavyCharge:
		m_RuntimeState.bIsHeavyCharging = true;
		m_RuntimeState.HeavyChargeTime = 0.f;
		SetAttackState(EPlayerAttackState::HeavyCharging);
		OnHeavyChargeChanged.Broadcast(true, m_RuntimeState.HeavyChargeTime);
		break;

	case EPlayerAttackType::Heavy:
		m_RuntimeState.bIsHeavyCharging = false;
		m_RuntimeState.bHeavyAttackStarted = true;
		SetAttackState(EPlayerAttackState::HeavyAttack);
		break;

	case EPlayerAttackType::AirLight:
	case EPlayerAttackType::AirHeavy:
		SetAttackState(EPlayerAttackState::AirAttack);
		break;

	case EPlayerAttackType::AirDash:
		SetAttackState(EPlayerAttackState::AirDashAttack);
		break;

	case EPlayerAttackType::AirFall:
		SetAttackState(EPlayerAttackState::AirFallAttack);
		break;

	case EPlayerAttackType::JustEvasiveAttack:
		if (m_MotionComponent)
		{
			m_MotionComponent->ExecuteJustEvasiveWarp();
		}

		SetAttackState(EPlayerAttackState::JustEvasiveAttack);
		break;

	case EPlayerAttackType::Light01:
	case EPlayerAttackType::Light02:
	case EPlayerAttackType::Light03:
	default:
		SetAttackState(EPlayerAttackState::LightAttack);
		break;
	}
}

//攻撃Ability終了
void UPlayerAttackComponent::EndAttackByTag(
	const FGameplayTag& AttackTag,
	bool bWasCancelled
)
{

	//すでに別の攻撃や別状態へ移行済みなら、古い攻撃終了で現在状態を消さない
	if (AttackTag.IsValid() &&
		m_RuntimeState.CurrentAttackTag.IsValid() &&
		!AttackTag.MatchesTagExact(m_RuntimeState.CurrentAttackTag))
	{

		RemoveAttackTagsFromASC(AttackTag);
		return;
	}

	if (bWasCancelled && m_RuntimeState.bComboTransitioning)
	{
		RemoveAttackTagsFromASC(AttackTag);
		return;
	}

	FinishAttack();
}
//攻撃を完全終了
void UPlayerAttackComponent::FinishAttack()
{

	const EPlayerAttackType FinishedAttackType =
		m_RuntimeState.CurrentAttackType;

	if (m_RuntimeState.CurrentAttackTag.IsValid())
	{
		RemoveAttackTagsFromASC(m_RuntimeState.CurrentAttackTag);
	}

	RemoveCommonAttackTagsFromASC();

	if (m_ComboComponent)
	{
		m_ComboComponent->CloseComboReserveWindow();
		m_ComboComponent->CloseComboActivationWindow();
		m_ComboComponent->CloseJustEvasiveCounterReserveWindow();
		m_ComboComponent->CloseJustEvasiveCounterActivationWindow();
		m_ComboComponent->ClearBufferedInput();
	}

	m_bJustEvasiveCounterReady = false;

	ResetRuntimeState();

	if (m_Player)
	{
		m_Player->SetIsEnhancedAttack(false);

		if (UCustomCharacterMovementComponent* MovementComponent =
			m_Player->FindComponentByClass<UCustomCharacterMovementComponent>())
		{
			MovementComponent->ClearAttackStartInput();
		}
	}

	OnAttackFinished.Broadcast(FinishedAttackType);
}

//コンボ遷移用に現在攻撃だけ終了
void UPlayerAttackComponent::EndCurrentAttackForComboTransition()
{
	if (m_RuntimeState.CurrentAttackTag.IsValid())
	{
		RemoveAttackTagsFromASC(m_RuntimeState.CurrentAttackTag);
	}

	m_RuntimeState.CurrentAttackTag = FGameplayTag();
	m_RuntimeState.CurrentAttackType = EPlayerAttackType::None;
}

//コンボ予約受付開始
void UPlayerAttackComponent::OpenComboReserveWindow()
{
	if (!m_ComboComponent) return;

	m_ComboComponent->OpenComboReserveWindow();
}

//コンボ予約受付終了
void UPlayerAttackComponent::CloseComboReserveWindow()
{
	if (!m_ComboComponent) return;

	m_ComboComponent->CloseComboReserveWindow();
}

//コンボ発動受付開始
void UPlayerAttackComponent::OpenComboActivationWindow()
{
	if (!m_ComboComponent) return;

	m_ComboComponent->OpenComboActivationWindow();
}

//コンボ発動受付終了
void UPlayerAttackComponent::CloseComboActivationWindow()
{
	if (!m_ComboComponent) return;

	m_ComboComponent->CloseComboActivationWindow();
}

//予約済み入力からコンボを発動
bool UPlayerAttackComponent::TryActivateBufferedCombo()
{
	if (!m_ComboComponent) return false;

	return m_ComboComponent->TryActivateBufferedCombo();
}

//予約入力をクリア
void UPlayerAttackComponent::ClearBufferedInput()
{
	if (!m_ComboComponent) return;

	m_ComboComponent->ClearBufferedInput();
}

//ジャスト回避カウンター予約受付開始
void UPlayerAttackComponent::OpenJustEvasiveCounterReserveWindow()
{
	if (!m_ComboComponent) return;

	m_ComboComponent->OpenJustEvasiveCounterReserveWindow();
}

//ジャスト回避カウンター予約受付終了
void UPlayerAttackComponent::CloseJustEvasiveCounterReserveWindow()
{
	if (!m_ComboComponent) return;

	m_ComboComponent->CloseJustEvasiveCounterReserveWindow();
}

//ジャスト回避カウンター発動受付開始
void UPlayerAttackComponent::OpenJustEvasiveCounterActivationWindow()
{
	if (!m_ComboComponent) return;

	m_ComboComponent->OpenJustEvasiveCounterActivationWindow();
}

//ジャスト回避カウンター発動受付終了
void UPlayerAttackComponent::CloseJustEvasiveCounterActivationWindow()
{
	if (!m_ComboComponent) return;

	m_ComboComponent->CloseJustEvasiveCounterActivationWindow();
}

//予約済み入力からジャスト回避カウンターを発動
bool UPlayerAttackComponent::TryActivateBufferedJustEvasiveCounter()
{
	if (!m_ComboComponent) return false;

	return m_ComboComponent->TryActivateBufferedJustEvasiveCounter();
}


//攻撃リセット
void UPlayerAttackComponent::ResetAttack()
{
	CancelAttackAbilities();

	if (m_ComboComponent)
	{
		m_ComboComponent->CloseComboReserveWindow();
		m_ComboComponent->CloseComboActivationWindow();
		m_ComboComponent->CloseJustEvasiveCounterReserveWindow();
		m_ComboComponent->CloseJustEvasiveCounterActivationWindow();
		m_ComboComponent->ClearBufferedInput();
	}

	m_bJustEvasiveCounterReady = false;

	if (m_MotionComponent)
	{
		m_MotionComponent->ResetAttackMotion();
	}

	if (m_EvasiveComponent)
	{
		m_EvasiveComponent->SetCanEvasive(true);
	}

	if (m_RuntimeState.CurrentAttackTag.IsValid())
	{
		RemoveAttackTagsFromASC(m_RuntimeState.CurrentAttackTag);
	}

	RemoveCommonAttackTagsFromASC();
	ResetRuntimeState();

	if (m_Player)
	{
		m_Player->RevivalCollision();
		m_Player->SetIsEnhancedAttack(false);
	}
}
//回避用に攻撃をキャンセル
void UPlayerAttackComponent::CancelAttackForEvasive()
{

	//攻撃Abilityだけキャンセル
	CancelAttackAbilities();

	//コンボ窓と予約入力を破棄
	if (m_ComboComponent)
	{
		m_ComboComponent->CloseComboReserveWindow();
		m_ComboComponent->CloseComboActivationWindow();
		m_ComboComponent->CloseJustEvasiveCounterReserveWindow();
		m_ComboComponent->CloseJustEvasiveCounterActivationWindow();
		m_ComboComponent->ClearBufferedInput();
	}

	m_bJustEvasiveCounterReady = false;

	//攻撃移動を破棄
	if (m_MotionComponent)
	{
		m_MotionComponent->ResetAttackMotion();
	}

	//攻撃タグを消す
	if (m_RuntimeState.CurrentAttackTag.IsValid())
	{
		RemoveAttackTagsFromASC(m_RuntimeState.CurrentAttackTag);
	}

	RemoveCommonAttackTagsFromASC();

	//攻撃状態をリセット
	ResetRuntimeState();

	if (m_Player)
	{
		m_Player->SetIsEnhancedAttack(false);
		m_Player->RevivalCollision();
	}
}

//攻撃踏み込み開始
void UPlayerAttackComponent::BeginAttackStep()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->BeginAttackStep();
}

//攻撃踏み込み更新
void UPlayerAttackComponent::TickAttackStep()
{
	if (!m_MotionComponent) return;

	const UWorld* World = GetWorld();
	if (!World) return;

	m_MotionComponent->TickAttackStep(World->GetDeltaSeconds());
}

//攻撃踏み込み終了
void UPlayerAttackComponent::EndAttackStep()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->EndAttackStep();
}

//空中攻撃開始
void UPlayerAttackComponent::BeginAirAttack()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->BeginAirAttack();
}

//空中攻撃終了
void UPlayerAttackComponent::EndAirAttack()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->EndAirAttack();
}

//空中弱攻撃のダッシュ斬り開始
void UPlayerAttackComponent::BeginAirDashAttack()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->BeginAirDashAttack();
}

//空中弱攻撃のダッシュ斬り終了
void UPlayerAttackComponent::EndAirDashAttack()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->EndAirDashAttack();
}

//空中強攻撃の叩き落とし開始
void UPlayerAttackComponent::BeginAirFallAttack()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->BeginAirFallAttack();
}

//空中強攻撃の叩き落とし終了
void UPlayerAttackComponent::EndAirFallAttack()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->EndAirFallAttack();
}

//ジャンプ開始時
void UPlayerAttackComponent::OnJumpStarted()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->OnJumpStarted();
}

//攻撃踏み込み開始
void UPlayerAttackComponent::AttackFirstStepBegin()
{
	if (!m_MotionComponent) return;

	m_MotionComponent->BeginCurrentAttackStep();
}

//攻撃踏み込み更新
void UPlayerAttackComponent::AttackFirstStepTick()
{
	TickAttackStep();
}

//攻撃踏み込み終了
void UPlayerAttackComponent::AttackFirstStepEnd()
{
	EndAttackStep();
}

//空中弱ダッシュ斬り
void UPlayerAttackComponent::AirDashAttack()
{
	BeginAirDashAttack();
}

//空中強叩き落とし
void UPlayerAttackComponent::AirFallAttack()
{
	BeginAirFallAttack();
}

//空中強叩き落とし終了
void UPlayerAttackComponent::AirFallAttackEnd()
{
	EndAirFallAttack();
}

//空中攻撃開始
void UPlayerAttackComponent::AirAttackStart()
{
	BeginAirAttack();
}

//空中攻撃終了
void UPlayerAttackComponent::AirAttackEnd()
{
	EndAirAttack();
}

//固定攻撃ターゲットを設定
void UPlayerAttackComponent::SetLockedAttackTarget(AEnemyBase* Enemy)
{
	if (!m_TargetComponent) return;

	m_TargetComponent->SetLockedAttackTarget(Enemy);
}

//空中攻撃開始済みか
bool UPlayerAttackComponent::GetIsAirAttackStart() const
{
	return GetIsAirAttackStarted();
}

//空中攻撃開始済みか
bool UPlayerAttackComponent::GetIsAirAttackStarted() const
{
	return m_MotionComponent
		? m_MotionComponent->GetIsAirAttackStarted()
		: false;
}

//空中ダッシュ攻撃中か
bool UPlayerAttackComponent::GetIsAirDashAttack() const
{
	return m_MotionComponent
		? m_MotionComponent->GetIsAirDashAttack()
		: false;
}

//空中叩き落とし中か
bool UPlayerAttackComponent::GetIsAirFallAttack() const
{
	return m_MotionComponent
		? m_MotionComponent->GetIsAirFallAttack()
		: false;
}

//攻撃中状態か
bool UPlayerAttackComponent::GetIsInAttackState() const
{
	return
		m_RuntimeState.bIsAttack ||
		m_RuntimeState.bIsHeavyCharging ||
		m_RuntimeState.bHeavyAttackStarted ||
		GetIsAirAttackStarted() ||
		GetIsAirDashAttack() ||
		GetIsAirFallAttack() ||
		m_RuntimeState.AttackState != EPlayerAttackState::None;
}

//攻撃入力を受け付けられるか
bool UPlayerAttackComponent::CanAcceptAttackInput() const
{
	if (!m_RuntimeState.bCanAttack) return false;
	if (m_RuntimeState.bIsAttack) return false;
	if (m_RuntimeState.bIsHeavyCharging) return false;
	if (m_RuntimeState.bHeavyAttackStarted) return false;
	if (GetIsAirAttackStarted()) return false;
	if (GetIsAirDashAttack()) return false;
	if (GetIsAirFallAttack()) return false;

	if (m_EvasiveComponent && m_EvasiveComponent->GetIsEvasive())
	{
		return false;
	}

	return true;
}

//強攻撃溜め更新
void UPlayerAttackComponent::UpdateHeavyCharge(float DeltaTime)
{
	if (!m_RuntimeState.bIsHeavyCharging) return;
	if (!m_AttackParameter) return;

	m_RuntimeState.HeavyChargeTime =
		FPlayerAttackSolver::UpdateHeavyChargeTime(
			m_RuntimeState.HeavyChargeTime,
			DeltaTime,
			m_AttackParameter
		);

	if (
		m_HeavyChargedStateTag.IsValid() &&
		m_RuntimeState.HeavyChargeTime >= m_AttackParameter->HeavyChargeMaxTime
		)
	{
		if (UAbilitySystemComponent* ASC = GetASC())
		{
			FGameplayTagContainer OwnedTags;
			ASC->GetOwnedGameplayTags(OwnedTags);

			if (!OwnedTags.HasTagExact(m_HeavyChargedStateTag))
			{
				ASC->AddLooseGameplayTag(m_HeavyChargedStateTag);

				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[HeavyCharge] FullCharged Time:%f"),
					m_RuntimeState.HeavyChargeTime
				);
			}
		}
	}

	OnHeavyChargeChanged.Broadcast(
		true,
		m_RuntimeState.HeavyChargeTime
	);
}


//攻撃状態を設定
void UPlayerAttackComponent::SetAttackState(EPlayerAttackState NewState)
{
	if (m_RuntimeState.AttackState == NewState) return;

	const EPlayerAttackState PreviousState =
		m_RuntimeState.AttackState;

	m_RuntimeState.AttackState = NewState;

	OnAttackStateChanged.Broadcast(
		PreviousState,
		NewState
	);
}

//攻撃終了時の状態を初期化
void UPlayerAttackComponent::ResetRuntimeState()
{
	SetAttackState(EPlayerAttackState::None);

	m_RuntimeState.Reset();
}

//プレイヤーアクション確定を通知
void UPlayerAttackComponent::CommitPlayerAction(
	EPlayerActionType ActionType,
	EPlayerAttackType AttackType
)
{
	if (!m_Player) return;
	if (ActionType != EPlayerActionType::Attack) return;

	switch (AttackType)
	{
	case EPlayerAttackType::Light01:
	case EPlayerAttackType::Light02:
	case EPlayerAttackType::Light03:
		m_Player->OnActionCommitted(EPlayerActionCommit::LightAttack);
		break;

	default:
		m_Player->OnActionCommitted(EPlayerActionCommit::OtherAttack);
		break;
	}
}

//攻撃タグをASCへ付与
void UPlayerAttackComponent::AddAttackTagsToASC(const FGameplayTag& AttackTag)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	if (AttackTag.IsValid())
	{
		ASC->AddLooseGameplayTag(AttackTag);
	}

	//State.Attack.Activeは攻撃AbilityのActivationOwnedTagsで管理する
	//ここでAddするとコンボ段数ぶんカウントが増えて、攻撃後も移動不能になる

	if (
		m_RuntimeState.bIsHeavyCharging &&
		m_HeavyChargingStateTag.IsValid()
		)
	{
		ASC->AddLooseGameplayTag(m_HeavyChargingStateTag);
	}
}


//攻撃タグをASCから削除
void UPlayerAttackComponent::RemoveAttackTagsFromASC(const FGameplayTag& AttackTag)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	if (AttackTag.IsValid() && OwnedTags.HasTagExact(AttackTag))
	{
		ASC->RemoveLooseGameplayTag(AttackTag);
	}

	ClearHeavyChargeStateTagsFromASC();
}

//強攻撃溜め状態タグを解除
void UPlayerAttackComponent::ClearHeavyChargeStateTagsFromASC()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	if (
		m_HeavyChargingStateTag.IsValid() &&
		OwnedTags.HasTagExact(m_HeavyChargingStateTag)
		)
	{
		ASC->RemoveLooseGameplayTag(m_HeavyChargingStateTag);
	}

	if (
		m_HeavyChargedStateTag.IsValid() &&
		OwnedTags.HasTagExact(m_HeavyChargedStateTag)
		)
	{
		ASC->RemoveLooseGameplayTag(m_HeavyChargedStateTag);
	}
}

//ジャスト回避後の強攻撃をFull状態にする
void UPlayerAttackComponent::PrepareJustEvasiveFullCharge()
{
	m_RuntimeState.bJustEvasiveLongCharge = true;

	if (m_AttackParameter)
	{
		m_RuntimeState.HeavyChargeTime =
			m_AttackParameter->HeavyChargeMaxTime;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	if (!m_HeavyChargedStateTag.IsValid()) return;

	if (!ASC->HasMatchingGameplayTag(m_HeavyChargedStateTag))
	{
		ASC->AddLooseGameplayTag(m_HeavyChargedStateTag);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[JustEvasiveHeavy] FullCharge Prepared Time:%f Tag:%s"),
		m_RuntimeState.HeavyChargeTime,
		*m_HeavyChargedStateTag.ToString()
	);
}

//ジャスト回避Full状態を解除
void UPlayerAttackComponent::ClearJustEvasiveFullCharge()
{
	m_RuntimeState.bJustEvasiveLongCharge = false;
	m_RuntimeState.HeavyChargeTime = 0.f;

	ClearHeavyChargeStateTagsFromASC();
}

//現在の攻撃Abilityをキャンセル
void UPlayerAttackComponent::CancelCurrentAttackAbility()
{
	CancelAttackAbilities();
}

//空中強攻撃中の着地を処理
void UPlayerAttackComponent::HandleAirFallLanded()
{
	if (!m_MotionComponent) return;
	if (!m_MotionComponent->GetIsAirFallAttack()) return;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AirHeavy] Landing Finish Current:%s"),
		*m_RuntimeState.CurrentAttackTag.ToString()
	);

	//先に空中攻撃の移動状態を戻す
	m_MotionComponent->EndAirFallAttack();

	//着地待ちで残っている攻撃Abilityを終了する
	CancelAttackAbilities();
}

//攻撃共通タグを削除
void UPlayerAttackComponent::RemoveCommonAttackTagsFromASC()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	//State.Attack.Activeは攻撃AbilityのActivationOwnedTagsで管理する
	//ここでRemoveLooseGameplayTagしない

	if (m_HeavyChargingStateTag.IsValid())
	{
		ASC->RemoveLooseGameplayTag(m_HeavyChargingStateTag);
	}
}


//攻撃Abilityをキャンセル
void UPlayerAttackComponent::CancelAttackAbilities()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	if (!m_AttackAbilityTag.IsValid()) return;

	FGameplayTagContainer CancelTags;
	CancelTags.AddTag(m_AttackAbilityTag);

	ASC->CancelAbilities(&CancelTags);
}

//ASCを取得
UAbilitySystemComponent* UPlayerAttackComponent::GetASC() const
{
	if (!m_Player) return nullptr;

	return m_Player->GetAbilitySystemComponent();
}

//攻撃ターゲット変更を受け取る
void UPlayerAttackComponent::HandleAttackTargetChanged(AActor* TargetActor)
{
	OnAttackTargetChanged.Broadcast(TargetActor);

	if (!TargetActor) return;

	RequestAttackLookCamera(TargetActor);
}
//コンボ予約受付変更を受け取る
void UPlayerAttackComponent::HandleComboReserveWindowChanged(bool bOpen)
{
	OnComboReserveWindowChanged.Broadcast(bOpen);
}

//コンボ発動受付変更を受け取る
void UPlayerAttackComponent::HandleComboActivationWindowChanged(bool bOpen)
{
	OnComboActivationWindowChanged.Broadcast(bOpen);
}
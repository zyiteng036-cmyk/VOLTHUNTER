#include "GA_Dash.h"
#include "../../PlayerComponent/CustomCharacterMovementComponent.h"

//初期化
UGA_Dash::UGA_Dash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag DashAbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Dash"), false);

	if (DashAbilityTag.IsValid())
	{
		FGameplayTagContainer AssetTags = GetAssetTags();
		AssetTags.AddTag(DashAbilityTag);
		SetAssetTags(AssetTags);
	}

	const FGameplayTag DashStateTag = FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Dash"), false);

	if (DashStateTag.IsValid())
	{
		ActivationOwnedTags.AddTag(DashStateTag);
	}

	const FGameplayTag EvasiveStateTag = FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Evasive"), false);

	if (EvasiveStateTag.IsValid())
	{
		ActivationBlockedTags.AddTag(EvasiveStateTag);
	}
}

//ダッシュを開始
void UGA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	m_bIsEndingDashAbility = false;

	if (!m_CustomMovementComponent)
	{
		EndPlayerAbility(true);
		return;
	}

	if (m_CustomMovementComponent->GetIsDash())
	{
		m_CustomMovementComponent->RequestStopDash();
		return;
	}

	if (!m_CustomMovementComponent->StartDash())
	{
		EndPlayerAbility(true);
		return;
	}

	BindMovementStateEvent();

	//ダッシュ中はAbilityをActiveのまま維持する
}

//再入力時にダッシュ停止を要求
void UGA_Dash::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (!m_CustomMovementComponent) return;
	if (!m_CustomMovementComponent->GetIsDash()) return;

	m_CustomMovementComponent->RequestStopDash();
}

//ダッシュAbilityを終了
void UGA_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!m_bIsEndingDashAbility)
	{
		m_bIsEndingDashAbility = true;

		UnbindMovementStateEvent();

		//外部キャンセル時にダッシュ状態を残さない
		if (m_CustomMovementComponent && m_CustomMovementComponent->GetIsDash())
		{
			m_CustomMovementComponent->CancelDash();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//移動状態変更イベントを登録
void UGA_Dash::BindMovementStateEvent()
{
	if (!m_CustomMovementComponent) return;

	m_CustomMovementComponent->OnMovementStateChanged.RemoveDynamic(this, &UGA_Dash::OnMovementStateChanged);
	m_CustomMovementComponent->OnMovementStateChanged.AddDynamic(this, &UGA_Dash::OnMovementStateChanged);
}

//移動状態変更イベントを解除
void UGA_Dash::UnbindMovementStateEvent()
{
	if (!m_CustomMovementComponent) return;

	m_CustomMovementComponent->OnMovementStateChanged.RemoveDynamic(this, &UGA_Dash::OnMovementStateChanged);
}

//移動状態変更時にAbility終了を判定
void UGA_Dash::OnMovementStateChanged(EPlayerMovementState PreviousState, EPlayerMovementState NewState)
{
	if (!m_CustomMovementComponent) return;

	if (NewState == EPlayerMovementState::Dash || NewState == EPlayerMovementState::DashDecel)
	{
		return;
	}

	EndPlayerAbility(false);
}
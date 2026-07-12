//担当
//伊藤直樹

#include "GA_Damage.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"

//初期化
UGA_Damage::UGA_Damage()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag DamageAbilityTag =
		FGameplayTag::RequestGameplayTag(TEXT("Ability.Damage"), false);

	if (DamageAbilityTag.IsValid())
	{
		FGameplayTagContainer AssetTags = GetAssetTags();
		AssetTags.AddTag(DamageAbilityTag);
		SetAssetTags(AssetTags);
	}

	const FGameplayTag DamagedStateTag =
		FGameplayTag::RequestGameplayTag(TEXT("State.Player.Damaged"), false);

	if (DamagedStateTag.IsValid())
	{
		ActivationOwnedTags.AddTag(DamagedStateTag);
	}

	const FGameplayTag DeadStateTag =
		FGameplayTag::RequestGameplayTag(TEXT("State.Player.Dead"), false);

	if (DeadStateTag.IsValid())
	{
		ActivationBlockedTags.AddTag(DeadStateTag);
	}
}

//Ability開始時
void UGA_Damage::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	m_bDamageAbilityEnded = false;

	if (!m_PlayerCharacter)
	{
		FinishDamageAbility(true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		FinishDamageAbility(true);
		return;
	}

	m_PlayerCharacter->SetActionState(EPlayerActionState::Damaged);
	m_PlayerCharacter->SetIsDamage(true);

	if (m_bLockInputDuringDamage)
	{
		m_PlayerCharacter->SetInputLocked(true);
	}

	if (!m_DamageMontage)
	{
		FinishDamageAbility(false);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			m_DamageMontage,
			m_PlayRate
		);

	if (!MontageTask)
	{
		FinishDamageAbility(true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Damage::OnDamageMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Damage::OnDamageMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Damage::OnDamageMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Damage::OnDamageMontageInterrupted);

	MontageTask->ReadyForActivation();
}

//Ability終了時
void UGA_Damage::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (m_PlayerCharacter && !m_PlayerCharacter->GetDie())
	{
		m_PlayerCharacter->SetIsDamage(false);
		m_PlayerCharacter->SetActionState(EPlayerActionState::None);

		if (m_bLockInputDuringDamage)
		{
			m_PlayerCharacter->SetInputLocked(false);
		}
	}

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

//Montage正常終了
void UGA_Damage::OnDamageMontageCompleted()
{
	FinishDamageAbility(false);
}

//Montage中断
void UGA_Damage::OnDamageMontageInterrupted()
{
	FinishDamageAbility(true);
}

//Damage終了
void UGA_Damage::FinishDamageAbility(bool bWasCancelled)
{
	if (m_bDamageAbilityEnded) return;

	m_bDamageAbilityEnded = true;

	EndPlayerAbility(bWasCancelled);
}
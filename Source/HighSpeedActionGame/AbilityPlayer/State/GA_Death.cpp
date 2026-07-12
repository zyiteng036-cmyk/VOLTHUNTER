//担当
//伊藤直樹

#include "GA_Death.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/CameraRigComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"

//初期化
UGA_Death::UGA_Death()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag DeathAbilityTag =
		FGameplayTag::RequestGameplayTag(TEXT("Ability.Death"), false);

	if (DeathAbilityTag.IsValid())
	{
		FGameplayTagContainer AssetTags = GetAssetTags();
		AssetTags.AddTag(DeathAbilityTag);
		SetAssetTags(AssetTags);
	}

	const FGameplayTag DeadStateTag =
		FGameplayTag::RequestGameplayTag(TEXT("State.Player.Dead"), false);

	if (DeadStateTag.IsValid())
	{
		ActivationOwnedTags.AddTag(DeadStateTag);
	}
}

//Ability開始時
void UGA_Death::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	m_bDeathAbilityEnded = false;

	if (!m_PlayerCharacter)
	{
		FinishDeathAbility();
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		FinishDeathAbility();
		return;
	}

	m_PlayerCharacter->SetActionState(EPlayerActionState::Dead);

	//死亡中は入力と移動を完全停止
	m_PlayerCharacter->DisableControl();

	if (m_PlayerCharacter->GetCameraRigComponent())
	{
		m_PlayerCharacter->GetCameraRigComponent()->RequestDieCamera();
	}

	if (!m_DeathMontage)
	{
		FinishDeathAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			m_DeathMontage,
			m_PlayRate
		);

	if (!MontageTask)
	{
		FinishDeathAbility();
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Death::OnDeathMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Death::OnDeathMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Death::OnDeathMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Death::OnDeathMontageCompleted);

	MontageTask->ReadyForActivation();
}
//Ability終了時
void UGA_Death::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

//Montage終了
void UGA_Death::OnDeathMontageCompleted()
{
	FinishDeathAbility();
}

//死亡Ability終了
void UGA_Death::FinishDeathAbility()
{
	if (m_bDeathAbilityEnded) return;

	m_bDeathAbilityEnded = true;

	if (m_PlayerCharacter && m_bCallPlayerDiedOnMontageEnd)
	{
		m_PlayerCharacter->PlayerDied();
	}

	EndPlayerAbility(false);
}
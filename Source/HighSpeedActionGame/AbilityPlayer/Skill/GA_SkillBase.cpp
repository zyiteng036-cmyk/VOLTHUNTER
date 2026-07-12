#include "GA_SkillBase.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerSkillComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"

//初期化
UGA_SkillBase::UGA_SkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag SkillAbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Skill"), false);

	if (SkillAbilityTag.IsValid())
	{
		FGameplayTagContainer AssetTags = GetAssetTags();
		AssetTags.AddTag(SkillAbilityTag);
		SetAssetTags(AssetTags);
	}

	const FGameplayTag SkillActiveTag = FGameplayTag::RequestGameplayTag(TEXT("State.Skill.Active"), false);

	if (SkillActiveTag.IsValid())
	{
		ActivationOwnedTags.AddTag(SkillActiveTag);
	}

	const FGameplayTag ActionBlockedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Input.ActionBlocked"), false);

	if (ActionBlockedTag.IsValid())
	{
		ActivationOwnedTags.AddTag(ActionBlockedTag);
	}
}

//スキルAbilityを開始
void UGA_SkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	m_SkillComponent = nullptr;
	m_bSkillAbilityEnded = false;

	if (!m_PlayerCharacter)
	{
		EndPlayerAbility(true);
		return;
	}

	m_SkillComponent = m_PlayerCharacter->FindComponentByClass<UPlayerSkillComponent>();

	if (!m_SkillComponent)
	{
		EndPlayerAbility(true);
		return;
	}

	if (!m_SkillTag.IsValid())
	{
		EndPlayerAbility(true);
		return;
	}

	if (m_SkillType == EPlayerSkillType::None)
	{
		EndPlayerAbility(true);
		return;
	}

	if (!m_SkillComponent->CanStartSkill(m_SkillType))
	{
		EndPlayerAbility(true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndPlayerAbility(true);
		return;
	}

	m_SkillComponent->BeginSkillByType(m_SkillTag, m_SkillType);

	if (!m_SkillMontage)
	{
		FinishSkillAbility(false);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, m_SkillMontage, m_PlayRate, m_StartSectionName);

	if (!MontageTask)
	{
		FinishSkillAbility(true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_SkillBase::OnSkillMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_SkillBase::OnSkillMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_SkillBase::OnSkillMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_SkillBase::OnSkillMontageCancelled);
	MontageTask->ReadyForActivation();
}

//スキルAbilityを終了
void UGA_SkillBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (m_SkillComponent)
	{
		m_SkillComponent->EndSkillByType(m_SkillTag, m_SkillType, bWasCancelled);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//Montage正常終了時にAbilityを終了
void UGA_SkillBase::OnSkillMontageCompleted()
{
	FinishSkillAbility(false);
}

//Montageブレンドアウト時にAbilityを終了
void UGA_SkillBase::OnSkillMontageBlendOut()
{
	FinishSkillAbility(false);
}

//Montage中断時にAbilityを終了
void UGA_SkillBase::OnSkillMontageInterrupted()
{
	FinishSkillAbility(true);
}

//Montageキャンセル時にAbilityを終了
void UGA_SkillBase::OnSkillMontageCancelled()
{
	FinishSkillAbility(true);
}

//スキルAbilityの終了を要求
void UGA_SkillBase::FinishSkillAbility(bool bWasCancelled)
{
	if (m_bSkillAbilityEnded) return;

	m_bSkillAbilityEnded = true;

	EndPlayerAbility(bWasCancelled);
}
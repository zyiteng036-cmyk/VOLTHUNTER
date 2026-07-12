#include "GA_SkillInput.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerSkillComponent.h"
#include "GameplayTagContainer.h"

//初期化
UGA_SkillInput::UGA_SkillInput()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag SkillInputTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Skill.Input"), false);

	if (SkillInputTag.IsValid())
	{
		FGameplayTagContainer AssetTags = GetAssetTags();
		AssetTags.AddTag(SkillInputTag);
		SetAssetTags(AssetTags);
	}
}

//選択中スキルの発動を要求
void UGA_SkillInput::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const bool bSucceeded = ExecuteSkillInput();

	EndPlayerAbility(!bSucceeded);
}

//スキル入力処理を実行
bool UGA_SkillInput::ExecuteSkillInput()
{
	if (!m_PlayerCharacter) return false;

	UPlayerSkillComponent* SkillComponent = m_PlayerCharacter->FindComponentByClass<UPlayerSkillComponent>();
	if (!SkillComponent) return false;

	return SkillComponent->RequestActivateSelectedSkill();
}
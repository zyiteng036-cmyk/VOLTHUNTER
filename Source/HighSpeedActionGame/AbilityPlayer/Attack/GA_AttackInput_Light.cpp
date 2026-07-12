#include "GA_AttackInput_Light.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerAttackComponent.h"

//初期化
UGA_AttackInput_Light::UGA_AttackInput_Light()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	m_LightInputTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Attack.Light"), false);
}

//弱攻撃入力を処理
void UGA_AttackInput_Light::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!m_PlayerCharacter)
	{
		EndPlayerAbility(true);
		return;
	}

	UPlayerAttackComponent* AttackComponent = m_PlayerCharacter->FindComponentByClass<UPlayerAttackComponent>();
	if (!AttackComponent)
	{
		EndPlayerAbility(true);
		return;
	}

	const bool bAccepted = AttackComponent->HandleAttackInputByTag(m_LightInputTag);

	EndPlayerAbility(!bAccepted);
}
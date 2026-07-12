#include "GA_PlayerBase.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../PlayerComponent/CustomCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

//初期化
UGA_PlayerBase::UGA_PlayerBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

//Ability開始時に共通参照を取得
void UGA_PlayerBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CachePlayerReferences(ActorInfo))
	{
		EndPlayerAbility(true);
	}
}

//共通参照を取得
bool UGA_PlayerBase::CachePlayerReferences(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo) return false;

	m_PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!m_PlayerCharacter) return false;

	m_AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!m_AbilitySystemComponent) return false;

	m_CustomMovementComponent = m_PlayerCharacter->FindComponentByClass<UCustomCharacterMovementComponent>();
	if (!m_CustomMovementComponent) return false;

	return true;
}

//現在のAbilityを終了
void UGA_PlayerBase::EndPlayerAbility(bool bWasCancelled)
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bWasCancelled);
}
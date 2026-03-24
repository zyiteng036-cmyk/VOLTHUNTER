// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityPlayer_AttackLight03.h"
#include "AbilitySystemComponent.h"
#include"../../../PlayerCharacter.h"
#include "../../../PlayerComponent/Player_AttackComponent.h"
#include "../../../PlayerComponent/Player_SkillComponent.h"


void UAbilityPlayer_AttackLight03::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
        {
            if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
            {
                AttackComp->SetIsAttack(false);
                AttackComp->SetComboIndex(0);
            }
            if (UPlayer_SkillComponent* SkillComp = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>())
            {
                SkillComp->SetCanSkillActive(true);
            }

            PlayerCharacter->SetIsEnhancedAttack(false);
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
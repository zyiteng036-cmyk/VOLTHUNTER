// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityPlayer_Damage.h"
#include "../../PlayerCharacter.h"
#include "../../PlayerComponent/Player_AttackComponent.h"
#include "../../PlayerComponent/Player_EvasiveComponent.h"


void UAbilityPlayer_Damage::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
        {
            if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
            {
                AttackComp->SetCanAttack(true);
            }
            PlayerCharacter->SetIsDamage(false);
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

}

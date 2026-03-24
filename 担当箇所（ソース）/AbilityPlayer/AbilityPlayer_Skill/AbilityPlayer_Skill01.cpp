// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityPlayer_Skill01.h"
#include "../../PlayerCharacter.h"
#include "../../PlayerComponent/Player_CameraComponent.h"
#include "../../PlayerComponent/Player_SkillComponent.h"


void UAbilityPlayer_Skill01::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
        {
            if (UPlayer_CameraComponent* CameraComp = PlayerCharacter->FindComponentByClass<UPlayer_CameraComponent>())
            {
                CameraComp->EndSkillCamera();
            }
            if (UPlayer_SkillComponent* SkillComp = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>())
            {
                SkillComp->EndSkill();
            }

        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

}

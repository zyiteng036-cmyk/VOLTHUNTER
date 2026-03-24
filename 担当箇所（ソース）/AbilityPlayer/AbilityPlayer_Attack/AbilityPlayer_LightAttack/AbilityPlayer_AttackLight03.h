//担当
//伊藤直樹

//弱３段目モーションGASクラス

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "../../AbilityPlayer_Base.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilityPlayer_AttackLight03.generated.h"

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAbilityPlayer_AttackLight03 : public UAbilityPlayer_Base
{
    GENERATED_BODY()
public:


    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled)override;
};

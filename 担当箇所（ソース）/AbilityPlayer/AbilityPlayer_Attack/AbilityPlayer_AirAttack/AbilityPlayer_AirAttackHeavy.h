//担当
//伊藤直樹

//空中強攻撃モーションGASクラス

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "../../AbilityPlayer_Base.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilityPlayer_AirAttackHeavy.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAbilityPlayer_AirAttackHeavy : public UAbilityPlayer_Base
{
	GENERATED_BODY()
public:

    
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

   

    virtual void OnMontageEnded()override;

};

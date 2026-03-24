//担当
//伊藤直樹

//移動回避アニメーションGASクラス

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityPlayer_Evasive.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAbilityPlayer_Evasive : public UGameplayAbility
{
	GENERATED_BODY()

public:

	
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    UFUNCTION()
    void OnMontageEnded();

    bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags,
        const FGameplayTagContainer* TargetTags,
        FGameplayTagContainer* OptionalRelevantTags) const override;

    //回避アニメ
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* EvasiveMontage;

};

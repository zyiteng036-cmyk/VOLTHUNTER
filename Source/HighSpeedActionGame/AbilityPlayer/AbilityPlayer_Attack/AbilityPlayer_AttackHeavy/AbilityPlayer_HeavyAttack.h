//担当
//伊藤直樹

//強攻撃モーションGASクラス

#pragma once

#include "CoreMinimal.h"
#include "../../../PlayerParam.h"
#include "GameplayTagContainer.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityPlayer_HeavyAttack.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAbilityPlayer_HeavyAttack : public UGameplayAbility
{
	GENERATED_BODY()
public:
    //
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    // 
    UFUNCTION()
    virtual void OnMontageEnded();


protected:
    // 

    //長い入力があったとき
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    UAnimMontage* HeavyLongkMontage;

    //短い入力があった時
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    UAnimMontage* HeavyShortMontage;

    //アニメをセット
    UAnimMontage* SetPlayMontage;

private:
    FPlayerParam PlayerParam;
};

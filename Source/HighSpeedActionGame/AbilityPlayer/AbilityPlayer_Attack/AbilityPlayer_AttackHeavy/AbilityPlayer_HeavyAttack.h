//担当
//伊藤直樹

//強攻撃モーションGASクラス

#pragma once

#include "CoreMinimal.h"
#include "../../../PlayerParam.h"
#include "../../AbilityPlayer_Base.h"
#include "AbilityPlayer_HeavyAttack.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAbilityPlayer_HeavyAttack : public UAbilityPlayer_Base
{
	GENERATED_BODY()
public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void OnMontageEnded() override;


protected:

    //長い入力があったとき
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Attack")
    UAnimMontage* HeavyLongMontage;

    // 短い入力時に再生するアニメ
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Attack")
    UAnimMontage* HeavyShortMontage;

private:
    FPlayerParam PlayerParam;
};

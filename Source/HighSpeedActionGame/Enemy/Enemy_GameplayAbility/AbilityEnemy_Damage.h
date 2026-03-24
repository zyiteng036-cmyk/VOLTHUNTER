// 佐々木奏太担当
//ダメージをアビリティGA

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityEnemy_Damage.generated.h"


class UAbilityTask_PlayMontageAndWait;
/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAbilityEnemy_Damage : public UGameplayAbility
{
	GENERATED_BODY()
	//アビリティアクティブ時に呼ばれるメソッド
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnMontageInterruptedOrCancelled();
public:

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TObjectPtr<UAnimMontage> DamageMontage;
	// 現在再生中のモンタージュタスクを保持し、上書き制御に利用

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;
};

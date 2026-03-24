//佐々木奏太担当
//その場攻撃用GAクラス

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityEnemy_Scratch.generated.h"


class UAbilityTask_PlayMontageAndWait;
/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAbilityEnemy_Scratch : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UAbilityEnemy_Scratch();

	//アビリティアクティブ時に呼ばれるメソッド
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttack")
	TObjectPtr<UAnimMontage> AttackMontage;

	//アニメモンタージュ終了時の処理
	UFUNCTION()
	void OnMontageEnded();
	
};

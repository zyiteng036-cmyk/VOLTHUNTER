// 佐々木奏太担当
//アビリティ使用BTT

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpec.h"
#include "BTT_ActivateAttackAbility.generated.h"


class AEnemyBase;
class UGameplayAbility;
class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_ActivateAttackAbility : public UBTTaskNode
{
	GENERATED_BODY()
public:
    UBTT_ActivateAttackAbility();

    //攻撃アビリティ
    UPROPERTY(EditAnywhere, Category = "Ability")
    TSubclassOf<UGameplayAbility> AttackAbility;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
    //アビリティ終了時に呼ばれるメソッド
    UFUNCTION()
    void OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);

    UPROPERTY()
    UAbilitySystemComponent* CachedASC;

    UPROPERTY()
    UBehaviorTreeComponent* CachedOwnerComp;

    //アビリティハンドル
    FGameplayAbilitySpecHandle ActivatedAbilityHandle;

    //デリゲードハンドル
    FDelegateHandle AbilityEndedDelegateHandle;

};

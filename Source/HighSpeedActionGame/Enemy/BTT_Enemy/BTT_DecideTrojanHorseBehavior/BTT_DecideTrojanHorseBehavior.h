// 佐々木奏太担当
//次のボスの行動決定BTT

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_DecideTrojanHorseBehavior.generated.h"

struct FBlackboardKeySelector;

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_DecideTrojanHorseBehavior : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_DecideTrojanHorseBehavior();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	//次の行動決定
	void JudgmentDecideTrojanHorseBehavior(AAIController* _contoroller, UBlackboardComponent* BB, int32& _nextState);
protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector m_EnemyStateKeyName;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector m_TargetActorKeyName;

	int32 ShortDistanceAttackCount = 0;//近距離攻撃カウント
	int32 LongDistanceAttackCount = 0;//遠距離攻撃カウント


};

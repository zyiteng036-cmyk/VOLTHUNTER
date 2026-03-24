// 佐々木奏太担当
//次のスプラインポイント設定BTT

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SelectNextSplinePoint.generated.h"

struct FBlackboardKeySelector;

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_SelectNextSplinePoint : public UBTTaskNode
{
	GENERATED_BODY()

	UBTT_SelectNextSplinePoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")

	FBlackboardKeySelector PatrolLocationKey;//パトロール座標

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolIndexKey;//パトロール指数

};

// 佐々木奏太担当
//スプライン間を移動するBTT

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_MoveToSplinePoint.generated.h"


struct FPathFollowingResult;
/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_MoveToSplinePoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
    UBTT_MoveToSplinePoint();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory,EBTNodeResult::Type TaskResult) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetLocationKey;
protected:
    UPROPERTY(EditAnywhere, Category = "MoveTo")
    float AcceptanceRadius = 50.f;//到達半径

private:
    FDelegateHandle MoveCompletedHandle;//移動終了デリゲード

    // 移動終了時に呼ばれる
    void OnMoveCompleted(FAIRequestID RequestID,const FPathFollowingResult& Result,UBehaviorTreeComponent* OwnerComp);
};

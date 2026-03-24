//担当：佐々木奏太
//瞬時にターゲットの方向を向くBTT

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_InstantFaceTarget.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_InstantFaceTarget : public UBTTaskNode
{
	GENERATED_BODY()
public:
    UBTT_InstantFaceTarget();

    // TargetActor用キー
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector TargetActorKey;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

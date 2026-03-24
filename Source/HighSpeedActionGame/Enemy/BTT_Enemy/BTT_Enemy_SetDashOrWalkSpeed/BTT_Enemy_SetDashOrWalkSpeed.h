// 佐々木奏太担当
//スピードをダッシュか歩きかえるBTT

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Enemy_SetDashOrWalkSpeed.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_Enemy_SetDashOrWalkSpeed : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_Enemy_SetDashOrWalkSpeed();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
private:

	UPROPERTY(EditAnywhere, Category = "Timer")
	bool IsDashing;
};

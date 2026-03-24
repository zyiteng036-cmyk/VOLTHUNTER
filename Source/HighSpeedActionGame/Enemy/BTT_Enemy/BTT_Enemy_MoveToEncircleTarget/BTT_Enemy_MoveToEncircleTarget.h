// 佐々木奏太担当
//ターゲットを囲むように移動するBTT

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_Enemy_MoveToEncircleTarget.generated.h"

struct FPathFollowingResult;

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_Enemy_MoveToEncircleTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTT_Enemy_MoveToEncircleTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;


	//プレイヤーとの距離
	UPROPERTY(EditAnywhere, Category = "AI")
	float m_PlayerProximityRadius = 100.f;
private:
	//移動終了
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

	//デリゲードバインド関数
	void BindDelegate(AAIController* AIController);

	// デリゲートアンバインド関数
	void UnBindDelegate(AAIController* AIController);

	FVector LastTargetLocation;

	float m_AcceptanceRadius;

	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;
};

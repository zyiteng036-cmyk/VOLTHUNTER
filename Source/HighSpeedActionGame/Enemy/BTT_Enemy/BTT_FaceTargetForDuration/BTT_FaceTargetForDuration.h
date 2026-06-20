// 佐々木奏太担当
// 指定時間ターゲットの方向向くBTT（補間なし）

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FaceTargetForDuration.generated.h"

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_FaceTargetForDuration : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_FaceTargetForDuration();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	//ターゲットの方向向く
	void FacingTheTargetActor();

	//使用者のポインタ
	AActor* UseActor;

	//向くターゲットのポインタ
	const AActor* TargetActor;

	//タイマー
	float Timer;


	UPROPERTY(EditAnywhere, Category = "Timer")
	FName TargetActorKeyName;

	//ターゲットの方向を向く時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float FaceTargetForDuration;
};

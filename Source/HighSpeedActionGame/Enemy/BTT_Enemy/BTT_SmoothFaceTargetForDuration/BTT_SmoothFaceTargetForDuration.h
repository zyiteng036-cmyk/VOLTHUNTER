// 佐々木奏太担当
//指定時間の間ターゲットの方向を向くBTT(補間あり)

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SmoothFaceTargetForDuration.generated.h"

class AEnemyBase;

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_SmoothFaceTargetForDuration : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_SmoothFaceTargetForDuration();

	UPROPERTY(EditAnywhere, Category = "Timer")
	float FaceTargetForDuration;
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	//ターゲットの方向向く
	void FacingTheTargetActor(const float _deltaTime);

	AEnemyBase* UseActor;//使用者
	const AActor* TargetActor;

	float Timer;//タイマー

	UPROPERTY(EditAnywhere, Category = "Timer")
	FName TargetActorKeyName;
};

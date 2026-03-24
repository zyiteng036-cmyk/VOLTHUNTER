// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_MoveToSplinePoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../../../SplinePatrolActor/SplinePatrolActor.h"
#include "../../EnemyBase.h"
#include "Navigation/PathFollowingComponent.h" 

UBTT_MoveToSplinePoint::UBTT_MoveToSplinePoint()
{
	NodeName = "Move To Spline Point";
}

EBTNodeResult::Type UBTT_MoveToSplinePoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)return EBTNodeResult::Failed;

	FVector TargetLocation = BB->GetValueAsVector(TargetLocationKey.SelectedKeyName);

	// MoveTo 実行
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(TargetLocation);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

	FNavPathSharedPtr Path;
	const EPathFollowingRequestResult::Type Result = AIController->MoveTo(MoveRequest, &Path);

	if (Result == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	// 完了イベント登録
	UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent();
	if (!PathComp)
	{
		return EBTNodeResult::Failed;
	}

	// 移動完了通知を登録
	MoveCompletedHandle = PathComp->OnRequestFinished.AddUObject(this, &UBTT_MoveToSplinePoint::OnMoveCompleted, &OwnerComp);

	return EBTNodeResult::InProgress;
}

void UBTT_MoveToSplinePoint::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent();

	//デリゲート解除
	if (PathComp && MoveCompletedHandle.IsValid())
	{
		PathComp->OnRequestFinished.Remove(MoveCompletedHandle);
	}
}

void UBTT_MoveToSplinePoint::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result, UBehaviorTreeComponent* OwnerComp) {
	//タスクが正常に終了したか
	const EBTNodeResult::Type TaskResult = Result.IsSuccess() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;

	FinishLatentTask(*OwnerComp, TaskResult);
}
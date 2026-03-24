// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_SelectNextSplinePoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../../EnemyBase.h"
#include "../../../SplinePatrolActor/SplinePatrolActor.h"


UBTT_SelectNextSplinePoint::UBTT_SelectNextSplinePoint() {
	NodeName = TEXT("Select Next Spline Point");
}

EBTNodeResult::Type UBTT_SelectNextSplinePoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AEnemyBase* Enemy = Cast<AEnemyBase>(AIController->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	ASplinePatrolActor* SplineActor = Enemy->GetSplinePatrolActor();
	if (!SplineActor) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	const int32 PointCount = SplineActor->GetSplinePointCount();
	if (PointCount <= 0) return EBTNodeResult::Failed;

	// 現在のインデックス取得
	int32 CurrentIndex = BB->GetValueAsInt(PatrolIndexKey.SelectedKeyName);

	// 次のインデックス
	const int32 NextIndex = (CurrentIndex + 1) % PointCount;

	// 座標取得
	const FVector NextLocation = SplineActor->GetSplinePointLocation(NextIndex);

	// Blackboard更新
	BB->SetValueAsInt(PatrolIndexKey.SelectedKeyName, NextIndex);
	BB->SetValueAsVector(PatrolLocationKey.SelectedKeyName, NextLocation);

	return EBTNodeResult::Succeeded;
}



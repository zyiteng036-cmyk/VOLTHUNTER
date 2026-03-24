// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Enemy_MoveToEncircleTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Actor.h"
#include "../../EnemyBase.h"

UBTT_Enemy_MoveToEncircleTarget::UBTT_Enemy_MoveToEncircleTarget()
	:m_AcceptanceRadius(50.f)
{
	NodeName = TEXT("Offset Move To");
	bNotifyTick = true; // TickTaskを有効化
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_Enemy_MoveToEncircleTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = &OwnerComp;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AEnemyBase* Enemy = Cast<AEnemyBase>(AIController->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	FVector TargetLocation = Enemy->GetMoveTargetLocation();
	LastTargetLocation = TargetLocation;

	BindDelegate(AIController);

	AIController->MoveToLocation(TargetLocation, m_AcceptanceRadius);

	return EBTNodeResult::InProgress;
}

void UBTT_Enemy_MoveToEncircleTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	AEnemyBase* Enemy = Cast<AEnemyBase>(AIController->GetPawn());
	if (!Enemy) return;

	FVector CurrentLocation = Enemy->GetActorLocation();
	FVector TargetLocation = FVector(Enemy->GetMoveTargetLocation().X, Enemy->GetMoveTargetLocation().Y, CurrentLocation.Z);

	// 到達判定
	float TargetLength = (CurrentLocation - TargetLocation).Size();

	if (TargetLength <= m_AcceptanceRadius) {

		UnBindDelegate(AIController);
		AIController->StopMovement();
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// プレイヤーとの距離判定
	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
	if (BlackboardComp)
	{
		UObject* PlayerObj = BlackboardComp->GetValueAsObject(TEXT("TargetActor"));
		AActor* PlayerActor = Cast<AActor>(PlayerObj);
		if (PlayerActor)
		{

			float DistanceToPlayer = FVector::Dist(CurrentLocation, PlayerActor->GetActorLocation());
			if (DistanceToPlayer <= m_PlayerProximityRadius)
			{
				UnBindDelegate(AIController);
				AIController->StopMovement();
				FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
				return;
			}
		}
	}

	// 目的地が更新されたら再発行
	if (FVector::DistSquared(TargetLocation, LastTargetLocation) > FMath::Square(10.f))
	{
		AIController->MoveToLocation(TargetLocation, m_AcceptanceRadius);
		LastTargetLocation = TargetLocation;
	}
}


EBTNodeResult::Type UBTT_Enemy_MoveToEncircleTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
		UnBindDelegate(AIController);

	}
	return EBTNodeResult::Aborted;
}

void UBTT_Enemy_MoveToEncircleTarget::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) {
	if (!CachedOwnerComp) return;

	// Abort / Repath は無視（移動継続）
	if (Result.Code == EPathFollowingResult::Aborted ||
		Result.Code == EPathFollowingResult::Invalid)
	{
		return;
	}

	AAIController* AIController = CachedOwnerComp->GetAIOwner();
	if (AIController)
	{
		UnBindDelegate(AIController);
	}

	FinishLatentTask(*CachedOwnerComp, Result.Code == EPathFollowingResult::Success ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}

void UBTT_Enemy_MoveToEncircleTarget::BindDelegate(AAIController* AIController) {
	if (AIController && AIController->GetPathFollowingComponent())
	{
		AIController->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &UBTT_Enemy_MoveToEncircleTarget::OnMoveCompleted);
	}
}

void UBTT_Enemy_MoveToEncircleTarget::UnBindDelegate(AAIController* AIController) {
	if (AIController && AIController->GetPathFollowingComponent())
	{
		AIController->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
	}
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_InstantFaceTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UBTT_InstantFaceTarget::UBTT_InstantFaceTarget()
{
	NodeName = TEXT("Instantly Face Target Actor");

	TargetActorKey.SelectedKeyName = FName(TEXT("TargetActor"));
}

EBTNodeResult::Type UBTT_InstantFaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)return EBTNodeResult::Failed;

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)return EBTNodeResult::Failed;

	FVector PawnLocation = Pawn->GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();

	// TargetActor•ûŒü‚Ö‚ÌYaw‰ñ“]‚ðŒvŽZ
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PawnLocation, TargetLocation);
	FRotator NewRotation(0.f, LookAtRotation.Yaw, 0.f);

	Pawn->SetActorRotation(NewRotation);

	return EBTNodeResult::Succeeded;
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_DecideTrojanHorseBehavior.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

namespace {
	constexpr float ShortDistanceLength = 600.f;
	constexpr int8 MaxAttackCount = 4;
	constexpr int8 MaxShortDistanceActionCount = 2;
	constexpr int8 MaxLongDistanceActionCount = 2;
}


UBTT_DecideTrojanHorseBehavior::UBTT_DecideTrojanHorseBehavior() {
	NodeName = TEXT("Decide TrojanHorse Behavior");
}

EBTNodeResult::Type UBTT_DecideTrojanHorseBehavior::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	int32 NextStateNum = 0;

	JudgmentDecideTrojanHorseBehavior(AIController, BB, NextStateNum);

	BB->SetValueAsEnum(m_EnemyStateKeyName.SelectedKeyName, NextStateNum);

	return EBTNodeResult::Succeeded;
}

void UBTT_DecideTrojanHorseBehavior::JudgmentDecideTrojanHorseBehavior(AAIController* _contoroller, UBlackboardComponent* BB, int32& _nextState) {
	if (!_contoroller || !BB)return;

	APawn* ControllerPawn = _contoroller->GetPawn();
	if (!ControllerPawn)return;

	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(m_TargetActorKeyName.SelectedKeyName));
	if (!TargetActor)return;

	float Distance = FVector::Dist(ControllerPawn->GetActorLocation(), TargetActor->GetActorLocation());


	if (Distance <= ShortDistanceLength) {
		if (ShortDistanceAttackCount % MaxShortDistanceActionCount == 0) {
			_nextState = 0;
		}
		else if (ShortDistanceAttackCount % MaxShortDistanceActionCount == 1) {
			_nextState = 1;
		}
		++ShortDistanceAttackCount;
	}
	else {
		if (LongDistanceAttackCount % MaxLongDistanceActionCount == 0) {
			_nextState = 2;
		}
		else if (LongDistanceAttackCount % MaxLongDistanceActionCount == 1) {
			_nextState = 3;
		}
		++LongDistanceAttackCount;
	}
}
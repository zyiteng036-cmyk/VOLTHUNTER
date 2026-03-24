// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Enemy_SetDashOrWalkSpeed.h"
#include "../../EnemyBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTT_Enemy_SetDashOrWalkSpeed::UBTT_Enemy_SetDashOrWalkSpeed()
	:IsDashing(false)
{

}

EBTNodeResult::Type UBTT_Enemy_SetDashOrWalkSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AEnemyBase* Enemy = Cast<AEnemyBase>(AIController->GetPawn());
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}

	if (IsDashing) {
		Enemy->Speed​​ChangeDash();
	}
	else {
		Enemy->SpeedChangeWalk();
	}

	return EBTNodeResult::Succeeded;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FaceTargetForDuration.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "Kismet//KismetMathLibrary.h"


UBTT_FaceTargetForDuration::UBTT_FaceTargetForDuration()
	:FaceTargetForDuration(1.f)
	, UseActor(nullptr)
	, TargetActor(nullptr)
	, Timer(0.f)
	, TargetActorKeyName("TargetActor")
{
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTT_FaceTargetForDuration::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UseActor = Cast<AActor>(AIController->GetPawn());
	if (!UseActor)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	UObject* ObjectValue = BlackboardComp->GetValueAsObject(TargetActorKeyName);
	TargetActor = Cast<AActor>(ObjectValue);

	return EBTNodeResult::InProgress;
}
void UBTT_FaceTargetForDuration::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) {
	Timer = 0.f;
	UseActor = nullptr;
	TargetActor = nullptr;
}
void UBTT_FaceTargetForDuration::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
	
	Timer += DeltaSeconds;

	FacingTheTargetActor();

	if (Timer > FaceTargetForDuration) {
		// タスク終了通知
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTT_FaceTargetForDuration::FacingTheTargetActor() {
	if (!UseActor || !TargetActor)return;

	//プレイヤーの方向を向く処理
	FVector ThisActorPos = UseActor->GetActorLocation();
	FVector TargetActorPos = TargetActor->GetActorLocation();
	FRotator ThisActorRot = UseActor->GetActorRotation();

	// プレイヤー方向の回転を計算
	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(ThisActorPos,TargetActorPos);

	// Yawのみ即時適用（Pitch, Rollは維持）
	UseActor->SetActorRotation(FRotator(ThisActorRot.Pitch, TargetRot.Yaw, ThisActorRot.Roll));
}
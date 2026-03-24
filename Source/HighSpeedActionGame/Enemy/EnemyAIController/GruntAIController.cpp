// Fill out your copyright notice in the Description page of Project Settings.


#include "GruntAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "../../GameUtility/GameUtility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"          // AI Perception用
#include "Perception/AISenseConfig_Sight.h"            // 視覚Sense設定用
#include "TimerManager.h"                      
#include "../EnemyNavigationManager/EnemyNavigationManager.h"
#include "../EnemyBase.h"

AGruntAIController::AGruntAIController() {
	// AI Perceptionコンポーネントを生成し、このControllerに紐づける
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	// 視覚Senseの設定オブジェクトを生成
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// 視認範囲
	SightConfig->SightRadius = 1500.f;

	// 見失い範囲
	SightConfig->LoseSightRadius = 1600.f;

	// 視野角を設定（左右90度ずつ、合計180度の視野）
	SightConfig->PeripheralVisionAngleDegrees = 100.f;

	// 視覚情報の最大有効時間（検出保持時間）
	SightConfig->SetMaxAge(5.f);

	// 敵・味方・中立いずれも検知可能に設定
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// 視覚SenseをPerceptionコンポーネントに登録
	AIPerceptionComp->ConfigureSense(*SightConfig);
}

void AGruntAIController::BeginPlay() {
	Super::BeginPlay();

	if (m_BehaviorTreeAsset)
	{
		RunBehaviorTree(m_BehaviorTreeAsset);
	}

	EnablePerceptionUpdates();

	// BlackboardにプレイヤーをTaragetActorとして登録
	auto pBlackBoard = GetBlackboardComponent();
	if (pBlackBoard)
	{
		pBlackBoard->SetValueAsObject(TEXT("TargetActor"), CGameUtility::GetActorFromTag(this->GetWorld(), "Player"));
	}

}

void AGruntAIController::EnablePerceptionUpdates() {
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AGruntAIController::OnTargetPerceptionUpdated);
	}
}

void AGruntAIController::DisablePerceptionUpdates() {
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &AGruntAIController::OnTargetPerceptionUpdated);
	}
}


void AGruntAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (m_BehaviorTreeAsset)
	{
		RunBehaviorTree(m_BehaviorTreeAsset);
	}
}
void AGruntAIController::SetAIActive(const bool _isActive) {
	UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (_isActive) {
		if (BT && m_BehaviorTreeAsset)
		{
			BT->StartTree(*m_BehaviorTreeAsset, EBTExecutionMode::Looped);
		}
	}
	else {
		if (BT && m_BehaviorTreeAsset)
		{
			BT->StopTree(EBTStopMode::Safe);
		}
	}
}

void AGruntAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) {

	// Nullチェック：引数のActorがnullptrなら処理を中断
	if (!Actor) return;

	// 所持Pawnを取得
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	AEnemyBase* Enemy = Cast<AEnemyBase>(ControlledPawn);
	if (!Enemy) return;

	if (Enemy->GetIsEventEnemy() || !Enemy->GetIsActive())return;

	// プレイヤー判定（タグ"Player"が付与されている前提とする）
	if (Actor->ActorHasTag(FName("Player")))
	{


		if (Stimulus.WasSuccessfullySensed())
		{
			//タイマークリア
			GetWorld()->GetTimerManager().ClearTimer(LostSightTimerHandle);
			if (!m_PlayerCurrentlySensed) {
				m_PlayerCurrentlySensed = true;
				OnPlayerFound();
			}

		}
		else
		{
			// 見失いタイマーが動いていなければ開始
			if (!GetWorld()->GetTimerManager().IsTimerActive(LostSightTimerHandle))
			{
				GetWorld()->GetTimerManager().SetTimer(
					LostSightTimerHandle,
					this,
					&AGruntAIController::HandleLostSightTimerExpired,
					m_LostSightDelay,
					false
				);
			}
		}
	}

}

// プレイヤーを検知した時の処理
void AGruntAIController::OnPlayerFound() {
	// 所持Pawnを取得
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	AEnemyBase* Enemy = Cast<AEnemyBase>(ControlledPawn);
	if (!Enemy) return;

	UEnemyNavigationManager* Subsystem = GetWorld()->GetSubsystem<UEnemyNavigationManager>();
	if (!Subsystem)return;

	Subsystem->AddChasingEnemy(Enemy);

	Enemy->SetCanSeeTarget(true);

	// Blackboardコンポーネントを取得（BT連携用）
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;
	// 見失いタイマーをクリア（追跡中はターゲット解除しない）
	GetWorld()->GetTimerManager().ClearTimer(LostSightTimerHandle);

	// Blackboardに「視界内(true)」をセット
	BB->SetValueAsBool(HasLineOfSightKey, true);

	UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BT && m_BehaviorTreeAsset)
	{
		BT->StopTree(EBTStopMode::Safe);
		BT->StartTree(*m_BehaviorTreeAsset, EBTExecutionMode::Looped);
	}



}

// プレイヤー見失い関数
void AGruntAIController::OnPlayerLost() {
	// Blackboardコンポーネント取得
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsBool(HasLineOfSightKey, false);
	}

	// 所持Pawnを取得
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	AEnemyBase* Enemy = Cast<AEnemyBase>(ControlledPawn);
	if (!Enemy) return;

	UEnemyNavigationManager* Subsystem = GetWorld()->GetSubsystem<UEnemyNavigationManager>();
	if (!Subsystem)return;
	Subsystem->RemoveChasingEnemy(Enemy);

	Enemy->SetCanSeeTarget(false);

	UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BT && m_BehaviorTreeAsset)
	{
		if (Enemy->GetBBCurrentState() != 1) {
			this->StopMovement(); // MoveToの移動を強制中断

			BT->StopTree(EBTStopMode::Safe);
			BT->StartTree(*m_BehaviorTreeAsset, EBTExecutionMode::Looped);
		}
	}
}

void AGruntAIController::HandleLostSightTimerExpired() {
	m_PlayerCurrentlySensed = false;
	OnPlayerLost();
}
void AGruntAIController::RestartBT() {
	UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BT && m_BehaviorTreeAsset)
	{
		this->StopMovement(); // MoveToの移動を強制中断

		BT->StopTree(EBTStopMode::Safe);
		BT->StartTree(*m_BehaviorTreeAsset, EBTExecutionMode::Looped);

	}
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAreaEventManager.h"
#include "../../WorldSettings/MyWorldSettings.h"
#include "../../DataAssets/EventData/EventEnemyClearConditionData.h"
#include "../../Enemy/EnemyManager/EnemyManager.h"
#include "../EventTriggerVolume/EventTriggerVolume.h"
#include "../../Enemy/EnemyTypeFlags.h"
#include "../../PlayerNotifySubSystem/PlayerNotifySubsystem.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"

bool UGameplayAreaEventManager::ShouldCreateSubsystem(UObject* Outer) const {
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	if (UWorld* WorldOuter = Cast<UWorld>(Outer))
	{
		if (AMyWorldSettings* MyWorldSettings = Cast<AMyWorldSettings>(WorldOuter->GetWorldSettings()))
		{
			return MyWorldSettings->m_GameplayAreaEventManager;
		}
	}

	return false;
}

void UGameplayAreaEventManager::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("UGameplayAreaEventManager::Initialize!!!"));
	}

	ResetParam();
}

void UGameplayAreaEventManager::Deinitialize() {
	ResetParam();

	Super::Deinitialize();

	UWorld* world = GetWorld();
	if (!world)return;

	ULocalPlayer* LocalPlayer = world->GetFirstLocalPlayerFromController();

	if (!LocalPlayer)return;

	UPlayerNotifySubsystem* PlayerSubsystem = LocalPlayer->GetSubsystem<UPlayerNotifySubsystem>();

	if (PlayerSubsystem) {
		PlayerSubsystem->OnPlayerDiedOccurred.RemoveDynamic(this, &UGameplayAreaEventManager::OnPlayerDied);
	}


	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("UGameplayAreaEventManager::Deinitialize!!!"));
	}
}

void UGameplayAreaEventManager::Tick(float DeltaTime)
{
	// 毎フレーム処理
	switch (m_EventMode)
	{
	case EventMode::None:
		break;
	case EventMode::EnemyClearCondition:
		_updateEnemyClearCondition(DeltaTime);
		break;
	default:
		break;
	}

}

void UGameplayAreaEventManager::_updateEnemyClearCondition(const float _deltaTime) {

}

TStatId UGameplayAreaEventManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMyTickableWorldSubsystem, STATGROUP_Tickables);
}

void UGameplayAreaEventManager::StartEvent(AEventTriggerVolume& _volume) {
	m_IsDuringTheEvent = true;

	m_EventEventTriggerVolume = &_volume;

	UEventDataBase* EventData = m_EventEventTriggerVolume->GetEventData();

	m_EventMode = EventData->GetDataEventMode();


	if (m_EventMode == EventMode::EnemyClearCondition) {
		StartEnemyClearConditionEvent(*EventData);
	}
}

bool UGameplayAreaEventManager::GetIsDuringTheEvent()const {
	return m_IsDuringTheEvent;
}

void UGameplayAreaEventManager::EventEnemyDeath() {
	if (!(m_EventMode == EventMode::EnemyClearCondition))return;

	++m_EnemyDeathNum;

	//敵のデス数が登録されている敵の数以上になったらイベント終了
	if (m_EnemyDeathNum >= m_EnemyClearConditionData->EventEnemyTypes.Num()) {
		m_EventEventTriggerVolume->EventEnd();
		EventEnd();
		return;
	}
	UWorld* World = GetWorld();
	if (!World)return;

	UEnemyManager* Subsystem = World->GetSubsystem<UEnemyManager>();
	if (!Subsystem)return;

	if (m_CurrentCount >= m_EnemyClearConditionData->EventEnemyTypes.Num()) return;

	FEnemyTypeFlags EnemyFlags;
	EnemyFlags.bIsEventEnemy = true;
	EnemyFlags.bIsBoss = m_EnemyClearConditionData->EventEnemyTypes[m_CurrentCount].m_IsBoss;

	Subsystem->ActivateEnemy(m_EnemyClearConditionData->EventEnemyTypes[m_CurrentCount].m_EnmeyType, m_EnemyClearConditionData->EnemySpawnPosition[m_EnemySpawnPositionsIndex], FRotator::ZeroRotator, EnemyFlags);
	AdvanceEnemySpawnPositionsIndex();
	++m_CurrentCount;
}

void UGameplayAreaEventManager::EventBossEnemyDeath() {
	NotifyBossDefeated();
}

void UGameplayAreaEventManager::StartEnemyClearConditionEvent(UEventDataBase& _data) {
	m_EnemyClearConditionData = Cast<UEventEnemyClearConditionData>(&_data);


	UWorld* World = GetWorld();
	if (!World)return;

	UEnemyManager* Subsystem = World->GetSubsystem<UEnemyManager>();
	if (!Subsystem)return;

	for (int i = 0; i < m_EnemyClearConditionData->MaximumNumberOfEnemiesPresent; ++i) {
		if (i >= m_EnemyClearConditionData->EventEnemyTypes.Num())break;

		FEnemyTypeFlags EnemyFlags;
		EnemyFlags.bIsEventEnemy = true;
		EnemyFlags.bIsBoss = m_EnemyClearConditionData->EventEnemyTypes[m_CurrentCount].m_IsBoss;

		//アクティブな敵
		Subsystem->ActivateEnemy(m_EnemyClearConditionData->EventEnemyTypes[m_CurrentCount].m_EnmeyType, m_EnemyClearConditionData->EnemySpawnPosition[m_EnemySpawnPositionsIndex], FRotator::ZeroRotator, EnemyFlags);

		//ボス生成時は通知
		if (EnemyFlags.bIsBoss) {
			NotifyBossActive();
		}

		AdvanceEnemySpawnPositionsIndex();
		++m_CurrentCount;
	}

}

void UGameplayAreaEventManager::EventEnd() {
	ResetParam();
	m_EventEventTriggerVolume = nullptr;
}

void UGameplayAreaEventManager::ResetParam() {
	m_IsDuringTheEvent = false;
	m_CurrentCount = 0;
	m_EnemyDeathNum = 0;
	m_EventMode = EventMode::None;
	m_EnemySpawnPositionsIndex = 0;
}

void UGameplayAreaEventManager::AdvanceEnemySpawnPositionsIndex() {
	++m_EnemySpawnPositionsIndex;
	if (m_EnemySpawnPositionsIndex >= m_EnemyClearConditionData->EnemySpawnPosition.Num()) {
		m_EnemySpawnPositionsIndex = 0;
	}
}

void UGameplayAreaEventManager::NotifyBossDefeated()
{
	//ゲームクリア指示を出す
	OnBossDead.Broadcast();
}

void UGameplayAreaEventManager::NotifyBossActive() {
	OnBossActive.Broadcast();
}

void UGameplayAreaEventManager::OnPlayerDied(AActor* _player) {
	if (m_IsDuringTheEvent) {
		if (m_EventEventTriggerVolume) {
			m_EventEventTriggerVolume->ResetEventExecutionFlag();
			m_EventEventTriggerVolume->EventEnd();
		}
		EventEnd();

		//イベントの敵を全て非アクティブに
		UEnemyManager* EnemyMng = GetWorld()->GetSubsystem<UEnemyManager>();
		if (EnemyMng)
		{
			EnemyMng->DeactivateAllEventEnemies();
		}
	}
}

void UGameplayAreaEventManager::OnBeginPlay() {
	UWorld* world = GetWorld();
	if (!world)return;

	ULocalPlayer* LocalPlayer = world->GetFirstLocalPlayerFromController();

	if (!LocalPlayer)return;

	UPlayerNotifySubsystem* PlayerSubsystem = LocalPlayer->GetSubsystem<UPlayerNotifySubsystem>();

	if (PlayerSubsystem) {
		PlayerSubsystem->OnPlayerDiedOccurred.AddDynamic(this, &UGameplayAreaEventManager::OnPlayerDied);
	}

}
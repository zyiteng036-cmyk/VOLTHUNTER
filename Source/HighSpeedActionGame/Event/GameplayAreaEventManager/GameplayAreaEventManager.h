//佐々木奏太担当
//イベント管理クラス

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "../EventMode.h"
#include "GameplayAreaEventManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDead);//ボスが死亡したときの通知クラス
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossActive);//ボスがアクティブになったときの通知クラス

/**
 *
 */

class UEventDataBase;
class UEventEnemyClearConditionData;
class AEventTriggerVolume;

UCLASS()
class HIGHSPEEDACTIONGAME_API UGameplayAreaEventManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//ゲームクリア用デリゲード
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBossDead OnBossDead;

	//ボスアクティブになったとき用のデリゲード
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBossActive OnBossActive;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	void StartEvent(AEventTriggerVolume& _data);
	void EventEnd();
	void StartEnemyClearConditionEvent(UEventDataBase& _data);
	bool GetIsDuringTheEvent()const;
	void EventEnemyDeath();
	void EventBossEnemyDeath();

	//BeginPlay時にGameModeから呼ばれる
	void OnBeginPlay();

private:
	void _updateEnemyClearCondition(const float _deltaTime);
	void ResetParam();
	void AdvanceEnemySpawnPositionsIndex();
	//ゲームクリア時に呼ばれるメソッド
	void NotifyBossDefeated();
	//ボスアクティブになったときに呼ばれる
	void NotifyBossActive();

	UFUNCTION()
	void OnPlayerDied(AActor* _player);
	
	UPROPERTY()
	bool m_IsDuringTheEvent;//イベント中か
	UPROPERTY()
	EventMode m_EventMode = EventMode::None;

	UPROPERTY()
	int32 m_CurrentCount;

	UPROPERTY()
	int32 m_EnemyDeathNum = 0;

	UPROPERTY()
	int32 m_EnemySpawnPositionsIndex = 0;

	UPROPERTY()
	UEventEnemyClearConditionData* m_EnemyClearConditionData;

	UPROPERTY()
	AEventTriggerVolume* m_EventEventTriggerVolume;
};

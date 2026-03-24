//佐々木奏太担当
//イベントを開始させるボリュームクラス

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../DataAssets/EventData/EventDataBase.h"
#include "EventTriggerVolume.generated.h"

class UBoxComponent;
class AEventActorBase;

UCLASS()
class HIGHSPEEDACTIONGAME_API AEventTriggerVolume : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEventTriggerVolume();

	virtual void Tick(float DeltaTime) override;

	UEventDataBase* GetEventData()const;

	void EventEnd();

	void ResetEventExecutionFlag() { m_IsEventExecution = false; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> m_TriggerBox;

	//イベント実行したかのフラグ
	bool m_IsEventExecution;

	//エディターで登録
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<TObjectPtr<AEventActorBase>> m_EventActor;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TObjectPtr<UEventDataBase> m_EventData;//イベントのデータ
};

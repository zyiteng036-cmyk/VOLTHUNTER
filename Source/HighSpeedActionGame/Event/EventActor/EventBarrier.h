//佐々木奏太担当
//イベントで使用できるバリア―クラス

#pragma once

#include "CoreMinimal.h"
#include "EventActorBase.h"
#include "EventBarrier.generated.h"

/**
 * 
 */

class UBoxComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API AEventBarrier : public AEventActorBase
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AEventBarrier();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetActive(const bool _isActive)override;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> m_BlockingVolume;
};

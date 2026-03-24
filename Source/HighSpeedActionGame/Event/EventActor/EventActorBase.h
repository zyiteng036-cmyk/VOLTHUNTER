//佐々木奏太担当
//イベントで使用するのActorの基底クラス

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EventActorBase.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API AEventActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEventActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetActive(const bool _isActive);

	virtual void SetVisible(const bool _isVisible);
protected:
	bool m_IsActive;

	// 可視化用Mesh
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> m_VisualMesh;



};

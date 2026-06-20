//²X–Ø‘t‘¾’S“–
//“G‚ªœpœj‚·‚éƒ|ƒCƒ“ƒgƒNƒ‰ƒX

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplinePatrolActor.generated.h"

class USplineComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API ASplinePatrolActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASplinePatrolActor();

	FVector GetSplinePointLocation(int32 _index)const;
	int32 GetSplinePointCount()const;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USplineComponent> m_Spline;
};

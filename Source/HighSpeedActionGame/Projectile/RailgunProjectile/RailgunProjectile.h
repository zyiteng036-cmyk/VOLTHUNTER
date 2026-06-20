//²X–Ø‘t‘¾’S“–
//ƒŒ[ƒ‹ƒKƒ“ƒNƒ‰ƒX

#pragma once

#include "CoreMinimal.h"
#include "../ProjectileBase.h"
#include "RailgunProjectile.generated.h"

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API ARailgunProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARailgunProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> m_RailgunBottom;

	FVector m_StartLocation;
};

//担当
//伊藤直樹

//スキルのエフェクト管理クラス

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "ThunderTrailEffect.generated.h"

class UNiagaraComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API AThunderTrailEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThunderTrailEffect();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Effect")
	void BeginFadeOut();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	UNiagaraComponent* NiagaraComp;

	UPROPERTY(EditDefaultsOnly, Category = "Effect Setup")
	float BaseEffectLength = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Effect Setup")
	float ThicknessScale = 0.1f;

	// 始点座標
	FVector StartLocation;

	// 最初の向き
	FRotator FixedRotation;

private:
	bool bIsFinished = false;
};

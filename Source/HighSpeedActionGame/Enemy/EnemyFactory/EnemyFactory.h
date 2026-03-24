//ç≤ÅXñÿëtëæíSìñ
//ìGê∂ê¨ÉNÉâÉX

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyFactory.generated.h"

class AEnemyBase;

UCLASS()
class HIGHSPEEDACTIONGAME_API AEnemyFactory : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyFactory();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TObjectPtr<AEnemyBase> CreateEnemy(UWorld* World, const FString EnemyName, const FVector& Location, const FRotator& Rotation);
	UDataTable* GetEnemyDataToGenerate();

	UPROPERTY(EditAnywhere, Category = "EnemyClass")
	TSubclassOf<AActor> Enemy_GruntClass;

	UPROPERTY(EditAnywhere, Category = "EnemyClass")
	TSubclassOf<AActor> Enemy_GruntGuardClass;

	UPROPERTY(EditAnywhere, Category = "EnemyClass")
	TSubclassOf<AActor> Enemy_TrojanHouseClass;
private:

	UPROPERTY(EditAnywhere, Category = "EnemyClass")
	TObjectPtr<UDataTable> m_EnemyDataToGenerate;
};

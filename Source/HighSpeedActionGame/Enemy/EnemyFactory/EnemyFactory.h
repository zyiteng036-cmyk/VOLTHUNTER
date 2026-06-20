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

	TObjectPtr<AEnemyBase> CreateEnemy(UWorld* _world, const FString& _enemyName, const FVector& _location, const FRotator& _rotation)const;
	UDataTable* GetEnemyDataToGenerate()const { return m_EnemyDataToGenerate; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyClass")
	TSubclassOf<AActor> Enemy_GruntClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyClass")
	TSubclassOf<AActor> Enemy_GruntGuardClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyClass")
	TSubclassOf<AActor> Enemy_TrojanHorseClass;
private:

	UPROPERTY(EditAnywhere, Category = "EnemyClass")
	TObjectPtr<UDataTable> m_EnemyDataToGenerate;
};

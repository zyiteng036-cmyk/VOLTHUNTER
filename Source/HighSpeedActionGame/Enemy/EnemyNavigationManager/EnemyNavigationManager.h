//佐々木奏太担当
//敵のナビゲーション用のクラス

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyNavigationManager.generated.h"

class AEnemyBase;

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UEnemyNavigationManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	virtual bool IsTickable() const override { return bTickEnabled; }

	void AddChasingEnemy(AEnemyBase* _chasingEnemy);
	void RemoveChasingEnemy(AEnemyBase* _chasingEnemy);

	void SetChasingEnemiesTargetLocations();
private:
	TArray<AEnemyBase*> m_ChasingEnemies;

	bool bTickEnabled = true;

	AActor* PlayerChara = nullptr;

	FVector LastPlayerPos;


};

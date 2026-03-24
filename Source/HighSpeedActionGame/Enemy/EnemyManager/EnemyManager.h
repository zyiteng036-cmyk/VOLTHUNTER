//佐々木奏太担当
//敵マネージャークラス

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "../EnemyType.h"
#include "../EnemyTypeFlags.h"
#include "EnemyManager.generated.h"

class UEnemyPool;
class AEnemyBase;

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UEnemyManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//プール初期化のメソッド
	void InitializePool();

	//プールから敵を生成するメソッド//レベルに配置したファクトリー側で呼ぶ
	void ActivateEnemy(const FString _name, const FVector _location, const FRotator _rotation, const FEnemyTypeFlags _enemyFlags);
	void ActivateEnemy(const EnemyType _type, const FVector _location, const FRotator _rotation, const FEnemyTypeFlags _enemyFlags);

	//アクティブな敵を全てポーズ状態にする
	void PauseAllActiveEnemies(const bool _isPause);

	//座標から一番近い敵取得
	const AEnemyBase* GetClosestActiveEnemyFromCoordinates(const FVector _location);

	//アクティブな敵配列から除外する
	void RemoveFromActiveEnemies(const AEnemyBase* _enemy);

	//イベントの敵を全て消す
	void DeactivateAllEventEnemies();

	UFUNCTION(BlueprintPure, Category = "Boss")
	const AEnemyBase* GetActiveBossEnemy()const;

private:
	//EnemyTypeから敵の名前に転換
	FString	ConvertEnemyTypeToEnmeyName(const EnemyType _type);

	//アクティブな敵の配列
	UPROPERTY()
	TArray<TObjectPtr<AEnemyBase>> m_ActiveEnemies;

	//敵のプール配列
	UPROPERTY()
	TMap<FString, TObjectPtr<UEnemyPool>> m_EnemyPools;

	UPROPERTY()
	bool m_IsPoolCreated;//プールが生成されているか


};

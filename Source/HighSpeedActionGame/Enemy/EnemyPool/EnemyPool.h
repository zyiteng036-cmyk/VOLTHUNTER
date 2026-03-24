//佐々木奏太担当
//敵のプールクラス

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../EnemyDataBase/EnemyDataBase.h"
#include "EnemyPool.generated.h"


class AEnemyBase;
class AEnemyFactory;

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UEnemyPool : public UObject
{
	GENERATED_BODY()

public:
	//プール作成//第一引数：敵生成クラス//第二引数：生成する敵の名前//第三引数：プールサイズ
	void GeneratePool(AEnemyFactory* _enemyFactiry, const FString _name, const int _poolSize);

    // オブジェクト破棄開始時の処理
	virtual void BeginDestroy() override;
   
	//プールから非アクティブな敵を探し、アクティブにして返す
	TObjectPtr<AEnemyBase> GetAndActivateEnemy();

	//配列に追加
	void AddEnemyPool(TObjectPtr<AEnemyBase> Enemy);

	//パラメーターをセット
	void SetEnemyParam(const FEnemyDataBase _enemydata);

private:
	//敵のプール
	UPROPERTY();
	TArray<TObjectPtr<AEnemyBase>> m_EnemyPool;

	//なんの敵を管理しているか
	UPROPERTY();
	FString m_EnemyType;
};

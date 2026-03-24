//担当佐々木奏太
//攻撃衝突判定用のプール管理クラス

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AttackCollisionPoolSubsystem.generated.h"

class AAttackCollisionFactory;
class AAttackCollisionDetection;
class FAttackCollisionHandle;
/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAttackCollisionPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//プール生成
	void CreatePool(const AAttackCollisionFactory* _factory);

	//ハンドル取得
	FAttackCollisionHandle AcquireHandle();

	//Handle からのみ呼ばれる返却処理
	void Release(AAttackCollisionDetection* Instance);

	AAttackCollisionDetection* GetInactiveAttackCollision()const;

private:
	UPROPERTY()
	TArray<TObjectPtr<AAttackCollisionDetection>> AttackCollisionPool;

	const int8 PoolSize = 10;
};

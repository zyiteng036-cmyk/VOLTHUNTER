//担当佐々木奏太
//攻撃衝突判定用のハンドルクラス

#pragma once

#include "CoreMinimal.h"

class UAttackCollisionPoolSubsystem;
class AAttackCollisionDetection;
class UHitJudgmentComponent;
struct FDamageInfo;

class FAttackCollisionHandle
{
public:
	//無効ハンドル（これでデフォルトの引数なしコンストラクタ宣言できるらしい）
	FAttackCollisionHandle() = default;

	// Subsystem からのみ生成される時に呼ばれるコンストラクタ
	FAttackCollisionHandle(UAttackCollisionPoolSubsystem* InSubsystem, AAttackCollisionDetection* InInstance);


	//有効か
	bool IsValid() const;

	//表示の有効無効
	void SetVisible(const bool _isVisble);

	//初期化用のメソッド
	void Initialize(const UHitJudgmentComponent& _hitJudgmentComponent, const FDamageInfo& _damage, const FVector& _location, const TArray<FString> _tags, const float _radius, const float _activeTime);

	//プールに返却時に呼ばれる処理をまとめるメソッド
	void Release();

	//位置設定
	void SetCollisionWorldLocation(const FVector _location);

private:
	//プールに返す
	void ReturnToPool();

private:
	void OnLifeTimeExpired();

private:
	TWeakObjectPtr<UAttackCollisionPoolSubsystem> m_AttackCollisionPoolSubsystem;
	TWeakObjectPtr<AAttackCollisionDetection> m_AttackCollisionDetectionInstance;

	FTimerHandle m_LifeTimerHandle;
	UWorld* m_CachedWorld = nullptr;
};
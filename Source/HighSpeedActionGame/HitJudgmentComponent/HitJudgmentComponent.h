//佐々木奏太担当
//ヒット判定コンポーネント

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../DamageInfo.h"
#include "../AttackCollisionDetection/AttackCollisionHandle/AttackCollisionHandle.h"
#include "HitJudgmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackHit, const AActor*, HitActor);


class AAttackCollisionDetection;
class FAttackCollisionHandle;
class UAttackCollisionPoolSubsystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UHitJudgmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//
	UHitJudgmentComponent();
	virtual ~UHitJudgmentComponent();
	//衝突判定開始
	void BeginHitDetection(FDamageInfo& _damageinfo, const float _radius, FVector& _relativeLocation, const AActor& _actor, const TArray<FString>& _tag,const float _activeTime);

	//衝突可能時間
	void PossibleHitTime(FVector& _relativeLocation);

	//衝突判定終了
	void EndHitDetection();

	//表示非表示セット
	void SetAttackCollisionDetectionVisible(const bool _isVisible);

	//半径セット
	void SetRadius(const float _radius);

	//ヒット時に通知を送るメソッド
	UFUNCTION()
	void NotifyAttackHit(const AActor* _hitActor)const;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//アクターの回転から相対的な座標配置
	void SetLocationRelativeToActorRotation(const FVector _relativeLocation);

protected:
	//衝突判定の使用者
	UPROPERTY()
	const AActor* m_CollisionDetectionUser;

	//攻撃用衝突判定ハンドル
	FAttackCollisionHandle m_CollisionHandle;

public:
	UPROPERTY(BlueprintAssignable)
	FOnAttackHit OnAttackHit;//デリケード
};

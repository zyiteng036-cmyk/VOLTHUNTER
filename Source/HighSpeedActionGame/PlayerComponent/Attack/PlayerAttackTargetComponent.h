//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー攻撃ターゲットコンポーネント
//
//攻撃対象の検索、固定ターゲット保持、画面内判定を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerAttackTargetComponent.generated.h"

class AEnemyBase;
class APlayerCharacter;
class UPlayerAttackParameter;

//攻撃ターゲット変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAttackTargetComponentChangedDelegate, AActor*, TargetActor);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayerAttackTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//初期化
	UPlayerAttackTargetComponent();

protected:
	//所有者をプレイヤーとして取得
	virtual void BeginPlay() override;

public:
	//使用する参照を初期化
	void InitializeTargetComponent(APlayerCharacter* Player, UPlayerAttackParameter* AttackParameter);

	//-----------------------------------------------------
	//ターゲット操作
	//-----------------------------------------------------

	//攻撃対象を検索して固定
	AEnemyBase* FindAttackTarget(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput);

	//カメラ演出用の攻撃対象を検索
	AEnemyBase* FindAttackCameraTarget(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput) const;

	//固定攻撃ターゲットを設定
	void SetLockedAttackTarget(AEnemyBase* Enemy);

	//const固定攻撃ターゲットを設定
	void SetLockedAttackTarget(const AEnemyBase* Enemy);

	//固定攻撃ターゲットを解除
	void ClearLockedAttackTarget();

	//固定攻撃ターゲットが有効か
	bool HasLockedAttackTarget() const;

	//固定攻撃ターゲットを取得
	AEnemyBase* GetLockedAttackTarget() const;

	//敵が画面内にいるか
	bool IsEnemyVisibleForAttack(const AEnemyBase* Enemy) const;

	//-----------------------------------------------------
	//Getter・Setter
	//-----------------------------------------------------

	//攻撃パラメータを設定
	void SetAttackParameter(UPlayerAttackParameter* AttackParameter) { m_AttackParameter = AttackParameter; }

	//攻撃パラメータを取得
	UPlayerAttackParameter* GetAttackParameter() const { return m_AttackParameter; }

public:
	//攻撃ターゲット変更通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Target")
	FOnPlayerAttackTargetComponentChangedDelegate OnAttackTargetChanged;

private:
	//条件に一致する最も近い攻撃対象を検索
	AEnemyBase* FindClosestAttackCandidate(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput) const;

private:
	//プレイヤー
	UPROPERTY()
	TObjectPtr<APlayerCharacter> m_Player = nullptr;

	//攻撃パラメータ
	UPROPERTY()
	TObjectPtr<UPlayerAttackParameter> m_AttackParameter = nullptr;

	//固定攻撃ターゲット
	UPROPERTY()
	TWeakObjectPtr<AEnemyBase> m_LockedAttackTarget = nullptr;
};
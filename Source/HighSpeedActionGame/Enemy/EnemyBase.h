//佐々木奏太担当
// 敵基底クラス
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyDataBase/EnemyDataBase.h"
#include "../IDamageable/Damageable.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "EnemyTypeFlags.h"
#include "EnemyBase.generated.h"


class APlayerCharacter;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UHitStopComponent;
class UHitJudgmentComponent;
class ASplinePatrolActor;

USTRUCT(BlueprintType)
struct FEnemyParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int64 m_Hp = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int64 m_MaxHp = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int64 AttackPower = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float m_WalkSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float m_DashSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float m_RotationSpeed = 2.f;

};


UCLASS()
class HIGHSPEEDACTIONGAME_API AEnemyBase : public ACharacter, public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	//セッター
	virtual void SetActive(const bool _isActive);
	void SetAIControllerActive(const bool _isActive);
	virtual void SetAIControllerIsActive(const bool _isActive);
	void SetCanSeeTarget(const bool _canSee) { m_CanSeeTarget = _canSee; }
	void SetIsEventEnemy(const bool _isEventEnemy);
	void SetMoveTargetLocation(const FVector& _location) { m_MoveTargetLocation = _location; }
	void SetBBMoveToLocation(const FVector _moveToLocation, FName _keyName);
	void SetEnemyActiveMoveComponent(const bool _isActive);
	void SetEnemyParam(FEnemyDataBase EnemyData);
	void SetIsBoss(const bool _isBoss) { m_EnemyTypeFlags.bIsBoss = _isBoss; }

	//ゲッター
	bool GetIsActive()const { return m_IsActive; }
	FEnemyParam GetEnemyParam()const { return m_EnemyParam; }
	ASplinePatrolActor* GetSplinePatrolActor()const { return m_SplinePatrolActor; }
	FVector GetMoveTargetLocation()const { return m_MoveTargetLocation; }
	float const GetDistanceToTarget()const { return m_DistanceToTarget; }
	bool GetIsEventEnemy() { return m_EnemyTypeFlags.bIsEventEnemy; }
	bool GetIsBossEnemy() { return m_EnemyTypeFlags.bIsBoss; }
	uint8 GetBBCurrentState(const FName _keyName = "EnemyState");
	bool GetBoolBBIsTakingDamage()const;								
	float GetWarpOffsetDistance()const { return m_WarpOffsetDistance; }	

	//死亡したか
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool GetIsDeath()const { return m_IsDeath; }

	//やられたかどうか
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool GetIsDying()const { return m_IsDying; }

	//ダメージインターフェース
	virtual void TakeDamage(const FDamageInfo& _damageInfo)override;

	//吹っ飛ばし処理
	void BlowEnemy(const FVector& _blowVector, const float _blowScale);

	void Speed​​ChangeDash();//ダッシュに変更時
	void SpeedChangeWalk();//歩きに変更時

	virtual void OnDamageEnd();//ダメージ終了時
	virtual void OnDying();//やられ終了時
	virtual void OnDeath();//死亡終了時

	//敵フラグリセット
	void ResetEnemyTypeFlags();

	//イベントモードに入るときのメソッド
	virtual void OnEnterEventMode();

	//ダメージ計算
	float CalculateEnemyDamage(const FDamageInfo& _damage);

	//ボス敵の場合召喚時に呼ばれるメソッド
	virtual void HandleBossCase();

	//ボスシーケンス終了メソッド
	UFUNCTION()
	void OnBossSequenceFinished();

	//敵ポーズ
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void PauseEnemy(bool _isPouse);

protected:
	//オパシティセット
	void SetOpacity(const float alpha);

	//パラメーターリセット
	virtual void ResetParam();

	//ダメージ情報使用
	virtual void UseDamageInformation(const FDamageInfo& _damageInfo);

	//プレイヤーの方向を瞬時に向く
	void FacePlayerDirection();

	//BBのダメージセット
	void SetBBIsTakingDamage(const bool _isTakingDamage);

	//ダメージ時のAI周りのセットアップ
	void HandleDamageTakenAI();

	//ターゲット視認時
	void OnSeeTarget();
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FEnemyParam m_EnemyParam;//敵の基本パラメーター

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FEnemyTypeFlags m_EnemyTypeFlags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TObjectPtr<UHitStopComponent> m_HitStopComponent;//ヒットストップコンポーネント

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> m_DamageMontage;//ダメージモンタージュクラス

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	TObjectPtr<UHitJudgmentComponent> m_HitJudgmentComponent;//攻撃時衝突判断用

	UPROPERTY(EditAnywhere, Category = "Enemy")
	TArray<TObjectPtr<UMaterialInstanceDynamic>> m_MaterialInstanceDynamics;//この敵のマテリアル

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	const APlayerCharacter* m_PlayerChara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	ASplinePatrolActor* m_SplinePatrolActor;//パトロールする際のSpline

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float m_Opacity;//不透明度

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float m_DistanceToTarget;//ターゲットとの距離

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float m_WarpOffsetDistance;//プレイヤーがワープするときにこの敵とどれくらい離れた位置にワープするか

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FVector m_MoveTargetLocation;//初期位置

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	bool m_IsActive;//アクティブか？

	bool m_IsDeath;//死亡したかの処理

	bool m_IsDying;//やられフラグ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	bool m_IsTakeDamage;//ダメージ与えられるかのフラグ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	bool m_IsTakingDamage;//ダメージ中かどうかのフラグ

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy")
	bool m_CanSeeTarget;//ターゲットを補足しているか

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float m_DamageTime;//ダメージ時間

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float m_DamageTimeTimer;//ダメージ時間経過タイマー

};

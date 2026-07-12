//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー攻撃共通定義
//
// 攻撃種別、攻撃状態、攻撃中に使用する一時状態を定義する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PlayerActionTypes.h"
#include "PlayerAttackTypes.generated.h"

//攻撃の細かい種別
UENUM(BlueprintType)
enum class EPlayerAttackType : uint8
{
	//なし
	None,

	//弱攻撃1段目
	Light01,

	//弱攻撃2段目
	Light02,

	//弱攻撃3段目
	Light03,

	//強攻撃溜め
	HeavyCharge,

	//強攻撃
	Heavy,

	//空中弱攻撃
	AirLight,

	//空中強攻撃
	AirHeavy,

	//空中ダッシュ攻撃
	AirDash,

	//空中叩き落とし
	AirFall,

	//ジャスト回避攻撃
	JustEvasiveAttack
};

//攻撃状態
UENUM(BlueprintType)
enum class EPlayerAttackState : uint8
{
	//なし
	None,

	//弱攻撃中
	LightAttack,

	//強攻撃溜め中
	HeavyCharging,

	//強攻撃中
	HeavyAttack,

	//空中攻撃中
	AirAttack,

	//空中ダッシュ攻撃中
	AirDashAttack,

	//空中叩き落とし中
	AirFallAttack,

	//ジャスト回避攻撃中
	JustEvasiveAttack,

	//硬直中
	Recovery,

	//無効
	Disabled
};

//攻撃踏み込み結果
USTRUCT(BlueprintType)
struct FPlayerAttackStepResult
{
	GENERATED_BODY()

public:
	//踏み込み先があるか
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	bool bHasTargetLocation = false;

	//踏み込み先
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	FVector TargetLocation = FVector::ZeroVector;

	//向く方向
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	FVector LookDirection = FVector::ForwardVector;

	//敵ターゲットを持つか
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	bool bHasEnemyTarget = false;

	//ターゲットActor
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	TObjectPtr<AActor> TargetActor = nullptr;
};

//攻撃中の一時状態
USTRUCT(BlueprintType)
struct FPlayerAttackRuntimeState
{
	GENERATED_BODY()

public:
	//攻撃可能か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	bool bCanAttack = true;

	//攻撃中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	bool bIsAttack = false;

	//コンボ遷移中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	bool bComboTransitioning = false;

	//現在のプレイヤーアクション種別
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	EPlayerActionType CurrentActionType = EPlayerActionType::None;

	//現在の攻撃種別
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	EPlayerAttackType CurrentAttackType = EPlayerAttackType::None;

	//現在の攻撃状態
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	EPlayerAttackState AttackState = EPlayerAttackState::None;

	//現在の攻撃タグ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	FGameplayTag CurrentAttackTag;

public:
	//強攻撃溜め中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Heavy")
	bool bIsHeavyCharging = false;

	//強攻撃開始済みか
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Heavy")
	bool bHeavyAttackStarted = false;

	//強攻撃溜め時間
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Heavy")
	float HeavyChargeTime = 0.f;

	//ジャスト回避後のロング溜め扱いか
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Heavy")
	bool bJustEvasiveLongCharge = false;

public:
	//空中攻撃開始済みか
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Air")
	bool bIsAirAttackStarted = false;

	//空中攻撃可能か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Air")
	bool bCanAirAttack = false;

	//ジャンプ開始時の高さ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Air")
	float JumpStartZ = 0.f;

	//空中ダッシュ攻撃中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Air")
	bool bIsAirDashAttack = false;

	//空中叩き落とし中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Air")
	bool bIsAirFallAttack = false;

	//空中叩き落とし溜め中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Air")
	bool bIsAirFallCharging = false;

	//空中叩き落とし溜め時間
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack|Air")
	float AirFallChargeTime = 0.f;

public:
	//攻撃状態を初期化
	void Reset()
	{
		bCanAttack = true;
		bIsAttack = false;
		bComboTransitioning = false;

		CurrentActionType = EPlayerActionType::None;
		CurrentAttackType = EPlayerAttackType::None;
		AttackState = EPlayerAttackState::None;
		CurrentAttackTag = FGameplayTag();

		bIsHeavyCharging = false;
		bHeavyAttackStarted = false;
		HeavyChargeTime = 0.f;
		bJustEvasiveLongCharge = false;

		bIsAirAttackStarted = false;
		bCanAirAttack = false;
		JumpStartZ = 0.f;
		bIsAirDashAttack = false;
		bIsAirFallAttack = false;
		bIsAirFallCharging = false;
		AirFallChargeTime = 0.f;
	}
};

//攻撃状態変更通知用
USTRUCT(BlueprintType)
struct FPlayerAttackStateChangeInfo
{
	GENERATED_BODY()

public:
	//変更前
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	EPlayerAttackState PreviousState = EPlayerAttackState::None;

	//変更後
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	EPlayerAttackState NewState = EPlayerAttackState::None;
};
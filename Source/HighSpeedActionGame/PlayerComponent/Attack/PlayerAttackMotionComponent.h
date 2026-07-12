//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー攻撃移動コンポーネント
//
//攻撃踏み込み、空中攻撃、ジャスト回避反撃ワープを管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Types/PlayerAttackTypes.h"
#include "PlayerAttackMotionComponent.generated.h"

class AEnemyBase;
class APlayerCharacter;
class UCustomCharacterMovementComponent;
class UPlayerAttackComponent;
class UPlayerAttackParameter;
class UPlayerAttackTargetComponent;
class UPlayerEvasiveComponent;

struct FPlayerAttackStepResult;

//攻撃移動ターゲット変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAttackMotionTargetChangedDelegate, AActor*, TargetActor);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayerAttackMotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//初期化
	UPlayerAttackMotionComponent();

protected:
	//所有者と関連コンポーネントを取得
	virtual void BeginPlay() override;

public:
	//-----------------------------------------------------
	//初期化
	//-----------------------------------------------------

	//使用する参照を初期化
	void InitializeMotionComponent(APlayerCharacter* Player, UPlayerAttackComponent* AttackComponent, UPlayerAttackTargetComponent* TargetComponent, UPlayerAttackParameter* AttackParameter);

	//攻撃パラメータを設定
	void SetAttackParameter(UPlayerAttackParameter* AttackParameter);

	//-----------------------------------------------------
	//更新
	//-----------------------------------------------------

	//攻撃移動状態を更新
	void TickAttackMotion(float DeltaTime);

	//ジャンプ開始時の高さを記録
	void OnJumpStarted();

	//-----------------------------------------------------
	//攻撃踏み込み
	//-----------------------------------------------------

	//現在の攻撃種別に応じた踏み込みを開始
	void BeginCurrentAttackStep();

	//地上攻撃踏み込みを開始
	void BeginAttackStep();

	//踏み込み位置を更新
	void TickAttackStep(float DeltaTime);

	//攻撃踏み込みを終了
	void EndAttackStep();

	//-----------------------------------------------------
	//空中攻撃
	//-----------------------------------------------------

	//空中攻撃状態を開始
	void BeginAirAttack();

	//空中攻撃状態を終了
	void EndAirAttack();

	//空中ダッシュ攻撃を開始
	void BeginAirDashAttack();

	//空中ダッシュ攻撃を終了
	void EndAirDashAttack();

	//空中叩き落とし攻撃を開始
	void BeginAirFallAttack();

	//空中叩き落とし攻撃を終了
	void EndAirFallAttack();

	//-----------------------------------------------------
	//ジャスト回避反撃
	//-----------------------------------------------------

	//ジャスト回避反撃ワープを実行
	bool ExecuteJustEvasiveWarp();

	//ジャスト回避反撃ワープを実行できるか
	bool CanExecuteJustEvasiveWarp() const;

	//-----------------------------------------------------
	//リセット
	//-----------------------------------------------------

	//すべての攻撃移動状態をリセット
	void ResetAttackMotion();

	//踏み込み状態をリセット
	void ResetStepState();

	//空中攻撃状態をリセット
	void ResetAirAttackState();

	//-----------------------------------------------------
	//Getter
	//-----------------------------------------------------

	//空中攻撃開始済みか
	bool GetIsAirAttackStarted() const { return m_bIsAirAttackStarted; }

	//空中攻撃可能か
	bool GetCanAirAttack() const { return m_bCanAirAttack; }

	//空中ダッシュ攻撃中か
	bool GetIsAirDashAttack() const { return m_bIsAirDashAttack; }

	//空中叩き落とし中か
	bool GetIsAirFallAttack() const { return m_bIsAirFallAttack; }

	//空中叩き落とし溜め中か
	bool GetIsAirFallCharging() const { return m_bIsAirFallCharging; }

	//踏み込み先を保持しているか
	bool GetHasAttackTargetLocation() const { return m_bHasAttackTargetLocation; }

public:
	//攻撃移動ターゲット変更通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Motion")
	FOnPlayerAttackMotionTargetChangedDelegate OnAttackMotionTargetChanged;

private:
	//空中攻撃解禁状態を更新
	void UpdateAirAttackUnlock();

	//空中叩き落とし状態を更新
	void UpdateAirFallAttack(float DeltaTime);

	//計算結果から踏み込み先を設定
	void ApplyAttackStepResult(const FPlayerAttackStepResult& StepResult);

	//現在の移動入力状態を取得
	void GetCurrentMoveInput(FVector& OutMoveDirection, bool& bOutHasMoveInput) const;

	//接地状態か
	bool IsGrounded() const;

	//空中にいるか
	bool IsJumping() const;

	//通常重力倍率を取得
	float GetDefaultGravityScale() const;

private:
	//-----------------------------------------------------
	//参照
	//-----------------------------------------------------

	//プレイヤー
	UPROPERTY()
	TObjectPtr<APlayerCharacter> m_Player = nullptr;

	//攻撃コンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerAttackComponent> m_AttackComponent = nullptr;

	//攻撃ターゲットコンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerAttackTargetComponent> m_TargetComponent = nullptr;

	//カスタム移動コンポーネント
	UPROPERTY()
	TObjectPtr<UCustomCharacterMovementComponent> m_CustomMovementComponent = nullptr;

	//回避コンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerEvasiveComponent> m_EvasiveComponent = nullptr;

	//攻撃パラメータ
	UPROPERTY()
	TObjectPtr<UPlayerAttackParameter> m_AttackParameter = nullptr;

private:
	//-----------------------------------------------------
	//踏み込み状態
	//-----------------------------------------------------

	//攻撃踏み込み先を保持しているか
	UPROPERTY(VisibleAnywhere, Category = "Attack|Step")
	bool m_bHasAttackTargetLocation = false;

	//攻撃踏み込み先
	UPROPERTY(VisibleAnywhere, Category = "Attack|Step")
	FVector m_AttackTargetLocation = FVector::ZeroVector;

	//-----------------------------------------------------
	//空中攻撃状態
	//-----------------------------------------------------

	//空中攻撃開始済みか
	UPROPERTY(VisibleAnywhere, Category = "Attack|Air")
	bool m_bIsAirAttackStarted = false;

	//空中攻撃可能か
	UPROPERTY(VisibleAnywhere, Category = "Attack|Air")
	bool m_bCanAirAttack = false;

	//ジャンプ開始時の高さ
	UPROPERTY(VisibleAnywhere, Category = "Attack|Air")
	float m_JumpStartZ = 0.f;

	//空中ダッシュ攻撃中か
	UPROPERTY(VisibleAnywhere, Category = "Attack|Air")
	bool m_bIsAirDashAttack = false;

	//空中叩き落とし中か
	UPROPERTY(VisibleAnywhere, Category = "Attack|Air")
	bool m_bIsAirFallAttack = false;

	//空中叩き落とし溜め中か
	UPROPERTY(VisibleAnywhere, Category = "Attack|Air")
	bool m_bIsAirFallCharging = false;

	//空中叩き落とし溜め時間
	UPROPERTY(VisibleAnywhere, Category = "Attack|Air")
	float m_AirFallChargeTime = 0.f;
};
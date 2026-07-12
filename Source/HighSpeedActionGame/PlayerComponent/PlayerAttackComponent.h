//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー攻撃コンポーネント
//
// 攻撃全体の窓口を担当する
// 入力、コンボ、敵探索、攻撃移動の詳細処理は専用Componentへ委譲する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Types/PlayerActionTypes.h"
#include "Types/PlayerAttackTypes.h"
#include "PlayerAttackComponent.generated.h"

class APlayerCharacter;
class AEnemyBase;
class UPlayerAttackParameter;
class UPlayerComboTable;
class UPlayerAttackComboComponent;
class UPlayerAttackTargetComponent;
class UPlayerAttackMotionComponent;
class UPlayerEvasiveComponent;
class UPlayerElectroGaugeComponent;
class UAbilitySystemComponent;
class UCameraRigComponent;

//攻撃ターゲット変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlayerAttackTargetChangedDelegate,
	AActor*, TargetActor
);

//攻撃終了通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlayerAttackFinishedDelegate,
	EPlayerAttackType, FinishedAttackType
);

//コンボ予約受付変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlayerComboReserveWindowChangedDelegate,
	bool, bOpen
);

//コンボ発動受付変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlayerComboActivationWindowChangedDelegate,
	bool, bOpen
);

//強攻撃溜め通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPlayerHeavyChargeChangedDelegate,
	bool, bCharging,
	float, ChargeTime
);

//攻撃状態変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPlayerAttackStateChangedDelegate,
	EPlayerAttackState, PreviousState,
	EPlayerAttackState, NewState
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayerAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//初期化
	UPlayerAttackComponent();

protected:
	//開始時
	virtual void BeginPlay() override;

public:
	//毎フレーム更新
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

public:
	//-----------------------------------------------------
	// GAS入力Abilityから呼ばれる関数
	//-----------------------------------------------------

	//攻撃入力タグを処理
	bool HandleAttackInputByTag(const FGameplayTag& InputTag);

	//弱攻撃要求
	bool RequestLightAttack();

	//強攻撃押下要求
	bool RequestHeavyAttackPressed();

	//強攻撃離し要求
	bool RequestHeavyAttackReleased();

	//攻撃時に敵を見るカメラを要求
	void RequestAttackLookCamera(AActor* TargetActor);
public:
	//-----------------------------------------------------
	// 実攻撃Abilityから呼ばれる関数
	//-----------------------------------------------------

	//攻撃Ability開始
	void BeginAttackByTag(
		const FGameplayTag& AttackTag,
		EPlayerActionType ActionType,
		EPlayerAttackType AttackType
	);

	//攻撃Ability終了
	void EndAttackByTag(
		const FGameplayTag& AttackTag,
		bool bWasCancelled
	);

	//攻撃を完全終了
	void FinishAttack();

	//コンボ遷移用に現在攻撃だけ終了
	void EndCurrentAttackForComboTransition();

public:
	//-----------------------------------------------------
	// Combo NotifyStateから呼ばれる関数
	//-----------------------------------------------------

	//コンボ予約受付開始
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	void OpenComboReserveWindow();

	//コンボ予約受付終了
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	void CloseComboReserveWindow();

	//コンボ発動受付開始
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	void OpenComboActivationWindow();

	//コンボ発動受付終了
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	void CloseComboActivationWindow();

	//予約済み入力からコンボを発動
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	bool TryActivateBufferedCombo();

	//予約入力をクリア
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	void ClearBufferedInput();

	//ジャスト回避カウンター予約受付開始
	UFUNCTION(BlueprintCallable, Category = "Attack|JustEvasive")
	void OpenJustEvasiveCounterReserveWindow();

	//ジャスト回避カウンター予約受付終了
	UFUNCTION(BlueprintCallable, Category = "Attack|JustEvasive")
	void CloseJustEvasiveCounterReserveWindow();

	//ジャスト回避カウンター発動受付開始
	UFUNCTION(BlueprintCallable, Category = "Attack|JustEvasive")
	void OpenJustEvasiveCounterActivationWindow();

	//ジャスト回避カウンター発動受付終了
	UFUNCTION(BlueprintCallable, Category = "Attack|JustEvasive")
	void CloseJustEvasiveCounterActivationWindow();

	//予約済み入力からジャスト回避カウンターを発動
	UFUNCTION(BlueprintCallable, Category = "Attack|JustEvasive")
	bool TryActivateBufferedJustEvasiveCounter();

	//ジャスト回避後の攻撃予約が有効か
	bool GetIsJustEvasiveCounterReady() const { return m_bJustEvasiveCounterReady; }

	//ジャスト回避後の攻撃予約状態を設定
	void SetJustEvasiveCounterReady(bool bReady) { m_bJustEvasiveCounterReady = bReady; }

	//ジャスト回避後の攻撃予約を消費
	void ConsumeJustEvasiveCounterReady() { m_bJustEvasiveCounterReady = false; }
public:
	//-----------------------------------------------------
	// Notify / Abilityから呼ばれる攻撃制御
	//-----------------------------------------------------

	//攻撃リセット
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void ResetAttack();

	//回避用に攻撃をキャンセル
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void CancelAttackForEvasive();

	//攻撃踏み込み開始
	UFUNCTION(BlueprintCallable, Category = "Attack|Step")
	void BeginAttackStep();

	//攻撃踏み込み更新
	UFUNCTION(BlueprintCallable, Category = "Attack|Step")
	void TickAttackStep();

	//攻撃踏み込み終了
	UFUNCTION(BlueprintCallable, Category = "Attack|Step")
	void EndAttackStep();

	//空中攻撃開始
	UFUNCTION(BlueprintCallable, Category = "Attack|Air")
	void BeginAirAttack();

	//空中攻撃終了
	UFUNCTION(BlueprintCallable, Category = "Attack|Air")
	void EndAirAttack();

	//空中弱攻撃のダッシュ斬り開始
	UFUNCTION(BlueprintCallable, Category = "Attack|Air")
	void BeginAirDashAttack();

	//空中弱攻撃のダッシュ斬り終了
	UFUNCTION(BlueprintCallable, Category = "Attack|Air")
	void EndAirDashAttack();

	//空中強攻撃の叩き落とし開始
	UFUNCTION(BlueprintCallable, Category = "Attack|Air")
	void BeginAirFallAttack();

	//空中強攻撃の叩き落とし終了
	UFUNCTION(BlueprintCallable, Category = "Attack|Air")
	void EndAirFallAttack();

	//ジャンプ開始時
	void OnJumpStarted();

	//強攻撃溜め状態タグを解除
	void ClearHeavyChargeStateTagsFromASC();

	//ジャスト回避後の強攻撃をFull状態にする
	void PrepareJustEvasiveFullCharge();

	//ジャスト回避Full状態を解除
	void ClearJustEvasiveFullCharge();

	//現在の攻撃Abilityをキャンセル
	void CancelCurrentAttackAbility();

	//空中強攻撃中の着地を処理
	void HandleAirFallLanded();
public:
	//-----------------------------------------------------
	// Notify名互換
	//-----------------------------------------------------

	//攻撃踏み込み開始
	UFUNCTION(BlueprintCallable, Category = "Attack|Legacy")
	void AttackFirstStepBegin();

	//攻撃踏み込み更新
	UFUNCTION(BlueprintCallable, Category = "Attack|Legacy")
	void AttackFirstStepTick();

	//攻撃踏み込み終了
	UFUNCTION(BlueprintCallable, Category = "Attack|Legacy")
	void AttackFirstStepEnd();

	//空中弱ダッシュ斬り
	UFUNCTION(BlueprintCallable, Category = "Attack|Legacy")
	void AirDashAttack();

	//空中強叩き落とし
	UFUNCTION(BlueprintCallable, Category = "Attack|Legacy")
	void AirFallAttack();

	//空中強叩き落とし終了
	UFUNCTION(BlueprintCallable, Category = "Attack|Legacy")
	void AirFallAttackEnd();

	//空中攻撃開始
	UFUNCTION(BlueprintCallable, Category = "Attack|Legacy")
	void AirAttackStart();

	//空中攻撃終了
	UFUNCTION(BlueprintCallable, Category = "Attack|Legacy")
	void AirAttackEnd();

public:
	//-----------------------------------------------------
	// Setter
	//-----------------------------------------------------

	//攻撃可能状態を設定
	void SetCanAttack(bool bCanAttack) { m_RuntimeState.bCanAttack = bCanAttack; }

	//攻撃中を設定
	void SetIsAttack(bool bIsAttack) { m_RuntimeState.bIsAttack = bIsAttack; }

	//強攻撃開始済みを設定
	void SetHeavyAttackStart(bool bStarted) { m_RuntimeState.bHeavyAttackStarted = bStarted; }

	//次攻撃をジャスト回避ロング扱いにする
	void SetJustEvasiveLongCharge(bool bLongCharge) { m_RuntimeState.bJustEvasiveLongCharge = bLongCharge; }

	//コンボ遷移中を設定
	void SetComboTransitioning(bool bTransitioning) { m_RuntimeState.bComboTransitioning = bTransitioning; }

	//固定攻撃ターゲットを設定
	void SetLockedAttackTarget(AEnemyBase* Enemy);

public:
	//-----------------------------------------------------
	// Getter
	//-----------------------------------------------------

	//攻撃中状態か
	bool GetIsInAttackState() const;

	//攻撃可能か
	bool GetCanAttack() const { return m_RuntimeState.bCanAttack; }

	//攻撃中か
	bool GetIsAttack() const { return m_RuntimeState.bIsAttack; }

	//強攻撃開始済みか
	bool GetHeavyAttackStart() const { return m_RuntimeState.bHeavyAttackStarted; }

	//強攻撃溜め中か
	bool GetIsHeavyCharging() const { return m_RuntimeState.bIsHeavyCharging; }

	//空中攻撃開始済みか
	bool GetIsAirAttackStart() const;

	//空中攻撃開始済みか
	bool GetIsAirAttackStarted() const;

	//空中ダッシュ攻撃中か
	bool GetIsAirDashAttack() const;

	//空中叩き落とし中か
	bool GetIsAirFallAttack() const;

	//ジャスト回避ロング扱いか
	bool GetJustEvasiveLongCharge() const { return m_RuntimeState.bJustEvasiveLongCharge; }

	//現在の強攻撃溜め時間
	float GetCurrentHeavyChargeTime() const { return m_RuntimeState.HeavyChargeTime; }

	//現在の攻撃状態
	EPlayerAttackState GetAttackState() const { return m_RuntimeState.AttackState; }

	//現在のアクション種別
	EPlayerActionType GetCurrentActionType() const { return m_RuntimeState.CurrentActionType; }

	//現在の攻撃種別
	EPlayerAttackType GetCurrentAttackType() const { return m_RuntimeState.CurrentAttackType; }

	//現在攻撃タグ
	const FGameplayTag& GetCurrentAttackTag() const { return m_RuntimeState.CurrentAttackTag; }

	//コンボ遷移中か
	bool IsComboTransitioning() const { return m_RuntimeState.bComboTransitioning; }

	//攻撃入力を受け付けられるか
	bool CanAcceptAttackInput() const;

public:
	//-----------------------------------------------------
	// Event
	//-----------------------------------------------------

	//攻撃ターゲット変更通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Event")
	FOnPlayerAttackTargetChangedDelegate OnAttackTargetChanged;

	//攻撃終了通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Event")
	FOnPlayerAttackFinishedDelegate OnAttackFinished;

	//コンボ予約受付変更通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Event")
	FOnPlayerComboReserveWindowChangedDelegate OnComboReserveWindowChanged;

	//コンボ発動受付変更通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Event")
	FOnPlayerComboActivationWindowChangedDelegate OnComboActivationWindowChanged;

	//強攻撃溜め通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Event")
	FOnPlayerHeavyChargeChangedDelegate OnHeavyChargeChanged;

	//攻撃状態変更通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Event")
	FOnPlayerAttackStateChangedDelegate OnAttackStateChanged;

private:
	//-----------------------------------------------------
	// 初期化
	//-----------------------------------------------------

	//参照を取得
	void CacheComponents();

	//サブコンポーネントを初期化
	void InitializeSubComponents();

	//サブコンポーネントの通知を登録
	void BindSubComponentEvents();

private:
	//-----------------------------------------------------
	// 更新処理
	//-----------------------------------------------------

	//強攻撃溜め更新
	void UpdateHeavyCharge(float DeltaTime);

private:
	//-----------------------------------------------------
	// 状態管理
	//-----------------------------------------------------

	//攻撃状態を設定
	void SetAttackState(EPlayerAttackState NewState);

	//攻撃開始時の状態を設定
	void ApplyAttackStartState(
		const FGameplayTag& AttackTag,
		EPlayerActionType ActionType,
		EPlayerAttackType AttackType
	);

	//攻撃終了時の状態を初期化
	void ResetRuntimeState();

	//プレイヤーアクション確定を通知
	void CommitPlayerAction(
		EPlayerActionType ActionType,
		EPlayerAttackType AttackType
	);

private:
	//-----------------------------------------------------
	// GASタグ
	//-----------------------------------------------------

	//攻撃タグをASCへ付与
	void AddAttackTagsToASC(const FGameplayTag& AttackTag);

	//攻撃タグをASCから削除
	void RemoveAttackTagsFromASC(const FGameplayTag& AttackTag);

	//攻撃共通タグを削除
	void RemoveCommonAttackTagsFromASC();

	//攻撃Abilityをキャンセル
	void CancelAttackAbilities();

	//ASCを取得
	UAbilitySystemComponent* GetASC() const;

private:
	//-----------------------------------------------------
	// サブコンポーネント通知
	//-----------------------------------------------------

	//攻撃ターゲット変更を受け取る
	UFUNCTION()
	void HandleAttackTargetChanged(AActor* TargetActor);

	//コンボ予約受付変更を受け取る
	UFUNCTION()
	void HandleComboReserveWindowChanged(bool bOpen);

	//コンボ発動受付変更を受け取る
	UFUNCTION()
	void HandleComboActivationWindowChanged(bool bOpen);

private:
	//-----------------------------------------------------
	// 参照
	//-----------------------------------------------------

	//プレイヤー
	UPROPERTY()
	TObjectPtr<APlayerCharacter> m_Player = nullptr;

	//攻撃コンボコンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerAttackComboComponent> m_ComboComponent = nullptr;

	//攻撃ターゲットコンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerAttackTargetComponent> m_TargetComponent = nullptr;

	//攻撃移動コンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerAttackMotionComponent> m_MotionComponent = nullptr;

	//回避コンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerEvasiveComponent> m_EvasiveComponent = nullptr;

	//電力ゲージコンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerElectroGaugeComponent> m_ElectroGaugeComponent = nullptr;

	//カメラリグ
	UPROPERTY(VisibleAnywhere, Category = "Player|Component")
	TObjectPtr<UCameraRigComponent> m_CameraRigComponent = nullptr;

protected:
	//-----------------------------------------------------
	// DataAsset
	//-----------------------------------------------------

	//攻撃パラメータ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Parameter")
	TObjectPtr<UPlayerAttackParameter> m_AttackParameter = nullptr;

	//コンボテーブル
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Combo")
	TObjectPtr<UPlayerComboTable> m_ComboTable = nullptr;

protected:
	//-----------------------------------------------------
	// 状態タグ
	//-----------------------------------------------------

	//攻撃Abilityタグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Tag")
	FGameplayTag m_AttackAbilityTag;

	//攻撃中タグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Tag")
	FGameplayTag m_AttackActiveStateTag;

	//強攻撃溜め中タグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Tag")
	FGameplayTag m_HeavyChargingStateTag;

private:
	//-----------------------------------------------------
	// 実行時状態
	//-----------------------------------------------------

	//攻撃中の一時状態
	UPROPERTY(VisibleAnywhere, Category = "Attack")
	FPlayerAttackRuntimeState m_RuntimeState;

	//ジャスト回避後の攻撃予約が有効か
	UPROPERTY(VisibleAnywhere, Category = "Attack|JustEvasive")
	bool m_bJustEvasiveCounterReady = false;

	//強攻撃最大溜め完了タグ
	UPROPERTY()
	FGameplayTag m_HeavyChargedStateTag;
};
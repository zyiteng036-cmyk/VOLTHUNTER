//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤークラス
//
//入力受付、GASの所有、被ダメージ、死亡、リスポーンを管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "../IDamageable/Damageable.h"
#include "../PlayerComponent/Types/PlayerActionTypes.h"
#include "PlayerCharacter.generated.h"

class APlayerCharacter;
class APlayer_SplineMove;
class UAbilitySystemComponent;
class UCameraComponent;
class UCameraRigComponent;
class UCustomCharacterMovementComponent;
class UGA_Move;
class UGameplayAbility;
class UHitJudgmentComponent;
class UInputAction;
class UInputMappingContext;
class UPlayerAttackComboComponent;
class UPlayerAttackComponent;
class UPlayerAttackMotionComponent;
class UPlayerAttackTargetComponent;
class UPlayerAttributeSet;
class UPlayerElectroGaugeComponent;
class UPlayerEvasiveComponent;
class UPlayerNotifySubsystem;
class UPlayerSkillComponent;
class UPlayerThunderFlashComponent;
class USpringArmComponent;
class UPlayerSettingsSubsystem;

struct FGameplayAbilitySpec;

enum class EElectroState : uint8;

UENUM(BlueprintType)
enum class EPlayerActionCommit : uint8
{
	LightAttack,
	OtherAttack,
	Jump,
	Evasive,
	Move,
	Skill,
	Damaged,
};

//入力とGameplayAbilityを対応付ける設定
USTRUCT(BlueprintType)
struct FPlayerAbilityInputMapping
{
	GENERATED_BODY()

public:
	//Enhanced Inputの入力アセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InputAction = nullptr;

	//この入力で起動するGameplayAbility
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;

	//押下時に扱う入力タグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	FGameplayTag InputTag = FGameplayTag();

	//離した時に扱う入力タグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	FGameplayTag ReleasedInputTag = FGameplayTag();

	//入力を離した通知をGASへ送るか
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	bool bNotifyInputReleased = true;

	//GAS内部で使用する入力ID
	int32 InputID = INDEX_NONE;
};

//やられ状態開始時の通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDyingDelegate, APlayerCharacter*, DyingCharacter);

//死亡確定時の通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedDelegate, APlayerCharacter*, DeadCharacter);

//リスポーン時の通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerRespawnDelegate, APlayerCharacter*, RespawnCharacter);

//被ダメージ時の通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDamagedDelegate, APlayerCharacter*, DamagedCharacter, const FDamageInfo&, DamageInfo);

//カメラ演出要求の通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerCameraRequestDelegate, APlayerCharacter*, RequestCharacter);

//アクション状態変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerActionStateChangedDelegate, EPlayerActionState, PreviousState, EPlayerActionState, NewState);

UCLASS()
class HIGHSPEEDACTIONGAME_API APlayerCharacter : public ACharacter, public IDamageable, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	//サブオブジェクト差し替えに対応して初期化
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	//開始時にプレイヤーシステムを初期化
	virtual void BeginPlay() override;

	//Enhanced Inputをバインド
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	//AbilitySystemComponentを取得
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//ダメージを受け取る
	virtual void TakeDamage(const FDamageInfo& DamageInfo) override;

	//ワールド外落下時に死亡処理を開始
	virtual void FellOutOfWorld(const UDamageType& DamageType) override;

	//リスポーン時の状態を初期化
	void OnRespawn();

	//死亡確定処理を実行
	UFUNCTION(BlueprintCallable, Category = "Player|Death")
	void PlayerDied();

public:
	//死亡状態を取得
	bool GetDie() const { return m_bIsDead; }

	//攻撃ヒット状態を取得
	bool GetIsHit() const { return m_bIsHit; }

	//攻撃ヒット状態を設定
	void SetIsHit(bool bIsHit) { m_bIsHit = bIsHit; }

	//攻撃ヒット状態をリセット
	UFUNCTION(BlueprintCallable, Category = "Player|Attack")
	void ClearAttackHitState();

	//ダメージ状態を取得
	bool GetIsDamage() const { return m_bIsDamage; }

	//ダメージ状態を設定
	void SetIsDamage(bool bIsDamage) { m_bIsDamage = bIsDamage; }

	//接地状態を取得
	bool GetOnGround() const;

	//強化攻撃状態を取得
	bool GetIsEnhancedAttack() const { return m_bIsEnhancedAttack; }

	//強化攻撃状態を設定
	void SetIsEnhancedAttack(bool bEnhancedAttack) { m_bIsEnhancedAttack = bEnhancedAttack; }

	//現在HPを取得
	UFUNCTION(BlueprintPure, Category = "Player|Status")
	float GetCurrentHP() const;

	//最大HPを取得
	UFUNCTION(BlueprintPure, Category = "Player|Status")
	float GetMaxHP() const;

	//現在HPの割合を取得
	UFUNCTION(BlueprintPure, Category = "Player|Status")
	float GetHealthPercentage() const;

	//現在のアクション状態を取得
	UFUNCTION(BlueprintPure, Category = "Player|Action")
	EPlayerActionState GetActionState() const { return m_CurrentActionState; }

	//アクション状態を設定
	void SetActionState(EPlayerActionState NewState);

	//ジャスト回避後の反撃対象を取得
	const AActor* GetJustEvasive_Attacker() const { return m_JustEvasive_Attacker.Get(); }

	//プレイヤーAttributeSetを取得
	UPlayerAttributeSet* GetPlayerAttributeSet() const { return m_PlayerAttributeSet; }

	//カメラリグを取得
	UCameraRigComponent* GetCameraRigComponent() const { return m_CameraRigComponent; }

	//最後に受け取った移動入力値を取得
	FVector2D GetLastMoveInputVector() const { return m_LastMoveInputVector; }

public:
	//被ダメージ無敵状態を設定
	void SetInvincible(bool bInvincible) { m_bDamageInvincible = bInvincible; }

	//HPを回復
	void AddHealth(float HealAmount);

	//ジャスト回避対象を登録
	void RegisterJustEvasiveAttacker(const AActor* Attacker);

	//ジャスト回避成功を通知
	void BroadcastJustEvasiveSuccess(const AActor* Attacker);

	//ジャスト回避後の無敵時間を開始
	void StartJustEvasiveInvincible();

	//入力ロック状態を設定
	UFUNCTION(BlueprintCallable, Category = "Player|Input")
	void SetInputLocked(bool bLock);

	//入力ロック状態を取得
	UFUNCTION(BlueprintPure, Category = "Player|Input")
	bool IsInputLocked() const { return m_bInputLocked; }

	//プレイヤーの一時停止状態を切り替える
	void PausePlayer(bool bPause);

	//アクション確定時の共通状態を更新
	void OnActionCommitted(EPlayerActionCommit Action);

	//現在触れているスプライン移動Actorを設定
	void SetCurrentSplineMoveActor(APlayer_SplineMove* Actor);

	//現在触れているスプライン移動Actorを取得
	APlayer_SplineMove* GetCurrentSplineMoveActor() const { return m_CurrentSplineMoveActor.Get(); }

	//スプライン移動開始を通知
	void NotifySplineMoveStarted();

	//スプライン移動終了を通知
	void NotifySplineMoveFinished();

	//オープニングカメラを要求
	UFUNCTION(BlueprintCallable, Category = "Player")
	void OpeningCamera();

	//プレイヤーコリジョンを復帰
	void RevivalCollision();

	//プレイヤーコリジョンを一時無効化
	void DeleteCollision();

	//操作と移動を無効化
	UFUNCTION(BlueprintCallable, Category = "Player|Status")
	void DisableControl();

	//操作と移動を復帰
	UFUNCTION(BlueprintCallable, Category = "Player|Status")
	void EnableControl();

public:
	//HP無限状態を切り替える
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void Debug_ToggleInfiniteHP();

	//オーバーチャージ状態を切り替える
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void Debug_ToggleOverCharge();

	//プレイヤー死亡を実行
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void Debug_TogglePlayerDie();

	//ボスイベント位置へワープ
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void Debug_WarpBossEvent();

public:
	//死亡確定時の通知
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDiedDelegate OnPlayerDied;

	//やられ状態開始時の通知
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDyingDelegate OnPlayerDying;

	//リスポーン時の通知
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerRespawnDelegate OnPlayerRespawn;

	//被ダメージ時の通知
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDamagedDelegate OnPlayerDamaged;

	//オープニングカメラ要求の通知
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerCameraRequestDelegate OnOpeningCameraRequested;

	//アクション状態変更の通知
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerActionStateChangedDelegate OnActionStateChanged;

protected:
	//死亡時のBlueprint処理を呼び出す
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Death")
	void OnPlayerDieBP(bool bFallDie);

	//リスポーン時のBlueprint処理を呼び出す
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Death")
	void OnPlayerRespawnBP();

private:
	//キャラクター基本設定を初期化
	void InitializeCharacterSettings();

	//AbilitySystemComponentを初期化
	void InitializeAbilitySystem();

	//Attributeの初期値を設定
	void InitializeAttributes();

	//移動Abilityを登録して起動
	void GiveAndActivateMoveAbility();

	//入力用Abilityを登録
	void GiveActionAbilities();

	//常時保持するAbilityを登録
	void GivePassiveStartupAbilities();

	//移動Abilityを再起動
	void ReactivateMoveAbility();

	//Enhanced InputのMappingContextを登録
	void RegisterInputMappingContext();

	//ワールドとComponentのイベントを登録
	void BindWorldEvents();

private:
	//移動入力を処理
	void MoveInput(const FInputActionValue& Value);

	//視点入力を処理
	void LookInput(const FInputActionValue& Value);

	//スキル選択入力を処理
	void SkillSelectInput(const FInputActionValue& Value);

	//スキル選択入力解除を処理
	void SkillSelectReleased();

	//スプライン移動入力を処理
	void SplineMoveInput();

	//アクション押下入力を処理
	void HandleActionPressed(const FInputActionValue&, int32 InputID);

	//アクション離し入力を処理
	void HandleActionReleased(const FInputActionValue&, int32 InputID);

	//InputIDからAbilitySpecを取得
	FGameplayAbilitySpec* FindAbilitySpecByInputID(int32 InputID) const;

	//ダメージAbilityを起動
	bool ActivateDamageAbility();

	//死亡Abilityを起動
	bool ActivateDeathAbility();

	//Abilityを必要に応じて付与して起動
	bool EnsureAbilityAndActivate(TSubclassOf<UGameplayAbility> AbilityClass);

private:
	//ボス戦開始時の処理
	UFUNCTION()
	void OnBossBattleStarted();

	//ボス戦終了時の処理
	UFUNCTION()
	void OnBossBattleEnded();

	//やられ状態を開始
	void PlayerDying();

	//ノックバックを適用
	void ApplyKnockBack(const FDamageInfo& DamageInfo);

	//攻撃ヒット時の状態を更新
	UFUNCTION()
	void OnAttackHit(const AActor* HitActor);

	//HPへダメージを適用
	void ApplyPlayerDamage(float DamageAmount);

	//HPを指定値に設定
	void SetHealthValue(float NewHealth);

	//被ダメージ後の無敵時間を開始
	void StartDamageInvincible();

	//被ダメージ後の無敵時間を終了
	void OnDamageInvincibleEnd();

	//ジャスト回避対象をクリア
	void ClearJustEvasiveAttacker();

	//ジャスト回避対象の有効性を確認
	void CheckJustEvasiveTargetValidity();

	//リスポーン時に一時状態を初期化
	void ResetTransientState();

	//ジャスト回避後の無敵時間を終了
	void OnJustEvasiveInvincibleEnd();

	//電力状態変更を受け取る
	UFUNCTION()
	void HandleElectroStateChanged(EElectroState PreviousState, EElectroState NewState);

	//オーバーチャージ状態か確認
	bool IsOverChargeMode() const;

	//プレイヤー通知Subsystemを取得
	UPlayerNotifySubsystem* FindPlayerNotifySubsystem() const;

protected:
	//-----------------------------------------------------
	//Component
	//-----------------------------------------------------

	//AbilitySystemComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> m_AbilitySystemComponent = nullptr;

	//プレイヤーAttributeSet
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPlayerAttributeSet> m_PlayerAttributeSet = nullptr;

	//カスタム移動コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UCustomCharacterMovementComponent> m_CustomMovementComponent = nullptr;

	//カメラ制御コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraRigComponent> m_CameraRigComponent = nullptr;

	//スプリングアーム
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> m_SpringArm = nullptr;

	//カメラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> m_Camera = nullptr;

	//回避コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Evasive")
	TObjectPtr<UPlayerEvasiveComponent> m_EvasiveComp = nullptr;

	//電力ゲージコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gauge")
	TObjectPtr<UPlayerElectroGaugeComponent> m_ElectroGaugeComp = nullptr;

	//攻撃コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UPlayerAttackComponent> m_AttackComp = nullptr;

	//攻撃ターゲットコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UPlayerAttackTargetComponent> m_AttackTargetComponent = nullptr;

	//攻撃コンボコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UPlayerAttackComboComponent> m_AttackComboComponent = nullptr;

	//攻撃移動コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UPlayerAttackMotionComponent> m_AttackMotionComponent = nullptr;

	//攻撃ヒット判定コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UHitJudgmentComponent> m_HitJudgmentComponent = nullptr;

	//スキルコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UPlayerSkillComponent> m_SkillComp = nullptr;

	//ThunderFlashコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UPlayerThunderFlashComponent> m_ThunderFlashComponent = nullptr;

	//プレイヤー設定Subsystem
	UPROPERTY()
	TObjectPtr<UPlayerSettingsSubsystem> m_PlayerSettingsSubsystem = nullptr;
protected:
	//-----------------------------------------------------
	//Ability
	//-----------------------------------------------------

	//常時付与するAbility
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Ability")
	TArray<TSubclassOf<UGameplayAbility>> m_PassiveStartupAbilities;

	//移動専用Ability
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Ability")
	TSubclassOf<UGameplayAbility> m_MoveAbilityClass = nullptr;

	//起動中の移動Ability
	UPROPERTY()
	TObjectPtr<UGA_Move> m_ActiveMoveAbility = nullptr;

	//移動以外の入力とAbilityの対応設定
	UPROPERTY(EditDefaultsOnly, Category = "Input|Config")
	TArray<FPlayerAbilityInputMapping> ActionInputMappings;

	//ダメージAbility
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Damage")
	TSubclassOf<UGameplayAbility> m_DamageAbilityClass = nullptr;

	//死亡Ability
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Death")
	TSubclassOf<UGameplayAbility> m_DeathAbilityClass = nullptr;

protected:
	//-----------------------------------------------------
	//Input
	//-----------------------------------------------------

	//入力マッピング
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> m_InputMapping = nullptr;

	//移動入力
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_MoveAction = nullptr;

	//視点入力
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_LookAction = nullptr;

	//スキル選択入力
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_SkillSelectAction = nullptr;

	//スプライン移動入力
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_SplineMoveAction = nullptr;

	//マウス視点感度
	UPROPERTY(EditDefaultsOnly, Category = "Input|Camera")
	float m_MouseSensitivity = 1.f;

	//ゲームパッド視点感度
	UPROPERTY(EditDefaultsOnly, Category = "Input|Camera")
	float m_GamepadSensitivity = 1.5f;

protected:
	//-----------------------------------------------------
	//Character
	//-----------------------------------------------------

	//キャラクター回転速度
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	FRotator m_CharacterRotationRate = FRotator(0.f, 720.f, 0.f);

	//スプリングアーム初期位置
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector m_DefaultSpringArmRelativeLocation = FVector(0.f, 0.f, 70.f);

	//カメラ初期ソケットオフセット
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector m_DefaultCameraSocketOffset = FVector(0.f, 0.f, 70.f);

	//被ダメージ後の無敵時間
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float m_DamageInvincibleDuration = 1.f;

	//オーバーチャージ中の被ダメージ倍率
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float m_OverchargeDamageRate = 0.5f;

	//ノックバック倍率
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float m_KnockBackPowerMultiplier = 1.f;

	//ジャスト回避対象の監視間隔
	UPROPERTY(EditDefaultsOnly, Category = "JustEvasive")
	float m_JustEvasiveTargetCheckInterval = 0.2f;

	//ジャスト回避後の反撃対象を保持する距離
	UPROPERTY(EditDefaultsOnly, Category = "JustEvasive")
	float m_JustEvasiveTargetKeepDistance = 3000.f;

	//ジャスト回避成功後の無敵時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evasive|Just")
	float m_JustEvasiveInvincibleDuration = 0.5f;

	//デバッグ表示時間
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	float m_DebugMessageDuration = 5.f;

	//デバッグ用ボス前ワープ位置
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	FVector m_DebugBossWarpLocation = FVector(-31013.f, 6854.f, 4600.f);

private:
	//-----------------------------------------------------
	//Runtime
	//-----------------------------------------------------

	//死亡しているか
	bool m_bIsDead = false;

	//落下死亡か
	bool m_bFallDie = false;

	//ダメージ状態か
	bool m_bIsDamage = false;

	//攻撃がヒットしたか
	bool m_bIsHit = false;

	//強化攻撃状態か
	bool m_bIsEnhancedAttack = false;

	//被ダメージ無敵中か
	bool m_bDamageInvincible = false;

	//入力ロック中か
	bool m_bInputLocked = false;

	//ジャスト回避無敵中か
	bool m_bJustEvasiveInvincible = false;

	//デバッグ無敵中か
	bool m_bDebugGodMode = false;

	//オーバーチャージ中か
	bool m_bIsOverChargeMode = false;

	//ボス戦中か
	bool m_bBossBattleActive = false;

	//現在のアクション状態
	UPROPERTY(VisibleAnywhere, Category = "Action")
	EPlayerActionState m_CurrentActionState = EPlayerActionState::None;

	//ジャスト回避後の反撃対象
	UPROPERTY()
	TWeakObjectPtr<const AActor> m_JustEvasive_Attacker = nullptr;

	//現在触れているスプライン移動Actor
	UPROPERTY()
	TWeakObjectPtr<APlayer_SplineMove> m_CurrentSplineMoveActor = nullptr;

	//最後に受け取った移動入力値
	FVector2D m_LastMoveInputVector = FVector2D::ZeroVector;

	//被ダメージ無敵タイマー
	FTimerHandle m_DamageInvincibleTimerHandle;

	//ジャスト回避対象監視タイマー
	FTimerHandle m_JustEvasiveTargetCheckTimerHandle;

	//ジャスト回避無敵タイマー
	FTimerHandle m_JustEvasiveInvincibleTimerHandle;

private:
	//Abilityのデフォルト付与レベル
	static constexpr int32 DefaultAbilityLevel = 1;

	//落下中の横摩擦
	static constexpr float DefaultFallingLateralFriction = 0.f;

	//Attribute未設定時の最大HP
	static constexpr float DefaultMaxHealth = 100.f;

	//Attribute未設定時の最大電力ゲージ
	static constexpr float DefaultMaxElectroGauge = 100.f;
};
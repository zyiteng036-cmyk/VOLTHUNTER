//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤークラス
//
//入力の受付、Componentの振り分け
//AbilitySystemComponentの所有、アビリティの起動管理
//ダメージ処理
//
//アクション系の処理などは各Componetで
//-----------------------------------------------------

//インクルードガード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "PlayerParam.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "IDamageable/Damageable.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class EPlayerActionCommit : uint8
{
	LightAttack,
	OtherAttack,
	Jump,
	Evasive,
	Move,
	Damaged,
};

//前方宣言
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UHitJudgmentComponent;
class UPlayer_MovementComponent;
class UPlayer_CameraComponent;
class UPlayer_EvasiveComponent;
class UPlayer_AttackComponent;
class UPlayer_ElectroGaugeComponent;
class UPlayer_SkillComponent;
class APlayer_SplineMove;
class APlayerSword;

//やられた（HP0になった）時の通知用デリゲート
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDyingDelegate, APlayerCharacter*, DyingCharacter);
//死亡通知用デリゲート
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedDelegate, APlayerCharacter*, DeadCharacter);

UCLASS()
class HIGHSPEEDACTIONGAME_API APlayerCharacter : public ACharacter, public IDamageable, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	//コンストラクタ
	//初期化
	APlayerCharacter();

protected:
	//スタート時、生成時
	virtual void BeginPlay() override;

	virtual void Landed(const FHitResult& Hit) override;

public:
	//毎フレーム
	virtual void Tick(float DeltaTime) override;

	//===入力バインド===
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//===リスポーン===
	void OnRespawn();

	virtual void FellOutOfWorld(const UDamageType& dmgType) override;

	//ステータス取得・設定
	//===死んだかどうか===
	bool GetDie()const { return m_IsDie; }
	//ヒットしたかを取得
	bool GetIsHit()const { return m_IsHit; }
	void SetIsHit(bool IsHit) { m_IsHit = IsHit; }
	//ダメージを受けたかどうか
	bool GetIsDamage()const { return m_IsDamage; }
	void SetIsDamage(bool _IsDamage) { m_IsDamage = _IsDamage; }
	//地面に設置しているかを取得
	bool GetOnGround()const { return m_IsOnGround; }
	//===強化攻撃かどうか===
	bool GetIsEnhancedAttack()const;
	//強化攻撃時にセット
	void SetIsEnhancedAttack(const bool _EnhancedAttack);

	UFUNCTION(BlueprintPure, Category = "Player|Status")
	float GetCurrentHP() const { return m_PlayerHP; }

	UFUNCTION(BlueprintPure, Category = "Player|Status")
	float GetMaxHP() const { return m_PlayerParam.MaxHP; }

	const FPlayerParam& GetPlayerParam() const { return m_PlayerParam; }

	//ダメージ・戦闘
	//===ダメージインターフェース実装===
	virtual void TakeDamage(const FDamageInfo& _damageInfo)override;
	//無敵状態をセット
	void SetInvincible(const bool _invincible);
	//リスポーン箇所に触れている間回復
	void AddHealth(float _HealAmount);
	//回避成功時のターゲット取得
	const AActor* GetJustEvasive_Attacker() { return m_JustEvasive_Attacker.Get(); }
	//回避コンポーネントから呼ぶ用
	void BroadcastJustEvasiveSuccess(const AActor* Attacker);

	//制御・システム
	void PausePlayer(const bool _isPause);
	void OnActionCommitted(EPlayerActionCommit Action);
	//スプライン移動
	void SetCurrentSplineMoveActor(APlayer_SplineMove* Actor);
	void Input_SplineMove();
	//オープニングカメラ
	UFUNCTION(BlueprintCallable, Category = "Player")
	void OpeningCamera();

	//プレイヤーコリジョン
	//復帰
	void RevivalCollision();
	//削除
	void DeleteCollision();

	//操作と移動を無効化
	UFUNCTION(BlueprintCallable, Category = "Player|Status")
	void DisableControl();
	//操作と移動を復帰
	UFUNCTION(BlueprintCallable, Category = "Player|Status")
	void EnableControl();

	//デバッグ機能
	//HP無限
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void Debug_ToggleInfiniteHP();

	//オーバーチャージ
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void Debug_ToggleOverCharge();

	//プレイヤーを死亡させる
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void Debug_TogglePlayerDie();

	//プレイヤーをボスの手前までワープさせる
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void Debug_WarpBossEvent();

	//Gameplay Ability System (GAS) 関連
	//アビリティの付与、起動、クールタイム管理を行う
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* m_AbilitySystemComponent = nullptr;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//次のアビリティの予約
	TSubclassOf<UGameplayAbility> m_BufferedNextAbility = nullptr;
	//次の起動予定アビリティ取得
	//コンボ入力受付中、現在のアビリティ終了後、自動で発動させるための予約用バッファ。
	TSubclassOf<UGameplayAbility> GetBufferedNextAbility()const { return m_BufferedNextAbility; }

	//次に発動するアビリティを予約
	void SetBufferedNextAbility(TSubclassOf<UGameplayAbility> _BufferedNextAbility) { m_BufferedNextAbility = _BufferedNextAbility; }

	//デリゲート・イベント
	//死亡時用のデリゲート
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDiedDelegate OnPlayerDied;

	//やられた時用のデリゲート
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDyingDelegate OnPlayerDying;

	//死亡時
	UFUNCTION(BlueprintCallable, Category = "Player|Death")
	void PlayerDied();

protected:
	//死亡時に呼ぶ
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Death")
	void OnPlayerDieBP(bool bFallDie);
	//リスポーン時
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Death")
	void OnPlayerRespawnBP();

	UFUNCTION()
	void OnAttackHit(const AActor* _hitActor);

private:
	//デリゲート用
	//ボス戦が始まった
	UFUNCTION()
	void OnBossBattleStarted();

	//ボスが死亡したとき
	UFUNCTION()
	void OnBossBattleEnded();
	//ボス戦中化のフラグ
	bool m_IsBossBattleActive = false;

	//やられた時
	void PlayerDying();
	//ノックバック
	void BlowEnemy(const FVector& _blowVector, const float _blowScale);
	//ダメージを減らす
	void PlayerTakeDamege(const float& _Damage);
	//無敵時間を終了する関数
	void OnDamageInvincibleEnd();
	//無敵時間を計測する
	void _updateInvincibleDuraction(float DeltaTime);
	//ジャスト回避対象の距離や生存を確認し、更新する
	void CheckJustEvasiveTargetValidity();

public:
	//===コンポーネント===
	//プレイヤーの移動コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_MovementComponent* m_MovementComponent = nullptr;

	//カメラコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_CameraComponent* m_CameraComponent = nullptr;

	//回避コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_EvasiveComponent* m_EvasiveComponent = nullptr;

	//攻撃コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_AttackComponent* m_AttackComponent = nullptr;

	//電力ゲージコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_ElectroGaugeComponent* m_ElectroGaugeComponent = nullptr;

	//スキルコンポ―ネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_SkillComponent* m_SkillComponent = nullptr;

public:
	//===アビリティ===

	//弱攻撃１のアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_AttackLight01 = nullptr;

	//弱攻撃2のアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_AttackLight02 = nullptr;

	//弱攻撃3のアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_AttackLight03 = nullptr;

	//強攻撃の長いときアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_AttackHeavy = nullptr;

	//強攻撃のチャージアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_AttackHeavyCharge = nullptr;

	//空中攻撃弱１のアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_AirAttackLight = nullptr;

	//空中攻撃強のアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_AirAttackHeavy = nullptr;

	//回避のアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_Evasive = nullptr;

	//バック回避のアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_BackEvasive = nullptr;

	//ダメージアビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_Damage = nullptr;

	//死亡アビリティ
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_Die = nullptr;

	//スキル
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> m_AbilityPlayer_Skill01 = nullptr;

protected:

	//プレイヤーのパラメーター
	//BPで操作できる値のみ
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	FPlayerParam m_PlayerParam;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TObjectPtr<UHitJudgmentComponent> m_HitJudgmentComponent = nullptr;

public:
	//カメラ設定のみをPlayerクラスで行う
	//スプリングアーム
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> m_SpringArm = nullptr;

	//カメラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> m_Camera = nullptr;

private:
	TWeakObjectPtr<APlayer_SplineMove> m_CurrentSplineMoveActor = nullptr;

	//Enhanced Input関連
	//マッピング
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> m_InputMapping = nullptr;

	//移動
	//前後
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_MoveForwardAction = nullptr;
	//左右
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_MoveRightAction = nullptr;

	//カメラ操作
	//ピッチ
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_CameraRotatePitch = nullptr;

	//ヨウ
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_CameraRotateYaw = nullptr;

	//ジャンプ
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_JumpAction = nullptr;

	//回避スウェイ
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_EvasiveAction = nullptr;

	//ダッシュ
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_DashAction = nullptr;

	//カメラリセット
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_CameraResetAction = nullptr;

	//弱攻撃
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_AttackLightAction = nullptr;

	//強攻撃
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_AttackHeavyAction = nullptr;

	//スキル選択
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_SkillSelectAction = nullptr;

	//スキル発動
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_SkillActiveAction = nullptr;

	//スプライン移動
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> m_SplineMoveAction = nullptr;

	//コントローラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerController, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<APlayerController> m_PlayerController = nullptr;

	//ジャスト回避した際に敵の位置を取得
	UPROPERTY()
	TWeakObjectPtr<const AActor> m_JustEvasive_Attacker = nullptr;

	//プレイヤーのHP
	float m_PlayerHP = 0.0f;

	//無敵状態
	bool m_Invincible = false;
	//無敵経過計測
	float m_InvincibleDuration = 0.0f;

	//死亡しているか
	bool m_IsDie = false;
	//落下死亡か
	bool m_IsFallDie = false;

	//ダメージを受ける
	bool m_IsDamage = false;

	//地面にいるか
	bool m_IsOnGround = false;

	bool m_IsHit = false;

	bool m_IsEnhancedAttack = false;

	//デバッグ中
	bool m_IsDebugGodMode = false;

	//被ダメージ時の無敵時間管理用タイマーハンドル
	FTimerHandle m_DamageInvincibleTimerHandle;

	//減少ゲージ
	float m_SubElectoroGauge = 5.0f;
};
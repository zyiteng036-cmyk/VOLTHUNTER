//担当
//伊藤直樹

//プレイヤー攻撃クラス

#pragma once

#include "CoreMinimal.h"
#include "../PlayerParam.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "../Enemy/EnemyBase.h"
#include "Player_AttackComponent.generated.h"

class APlayerCharacter;
class UPlayer_MovementComponent;
class UPlayer_EvasiveComponent;
class UPlayer_CameraComponent;
class UPlayer_ElectroGaugeComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayer_AttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//コンストラクタ
	UPlayer_AttackComponent();

protected:
	//ゲーム開始時に呼ばれる
	virtual void BeginPlay() override;

public:
	//毎フレーム呼ばれる
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//強攻撃の溜め時間を更新
	void _updateAttackHeavy(float DeltaTime);

	//空中攻撃の解禁状態を更新
	void _updateAirAttackUnlock();

	//弱攻撃の入力処理
	void Input_AttackLight(const FInputActionValue& Value);

	//強攻撃の入力処理
	void Input_AttackHeavy(const FInputActionValue& Value);

	//強攻撃の溜めを離したときの処理
	void Completed_AttackHeavy(const FInputActionValue& Value);

	//ジャスト回避からの派生攻撃
	void JustEvasiveAttack();

	//空中攻撃開始時
	void AirAttackStart();
	//空中攻撃終了時
	void AirAttackEnd();

	//空中攻撃の強たたき落とし
	void AirFallAttack();

	//空中攻撃の強たたき落とし終了
	void AirFallAttackEnd();

	//空中攻撃の弱ダッシュ斬り
	void AirDashAttack();

	//攻撃間合い詰め開始
	//踏み込む位置と向きを設定
	//入力方向で進む方向が変化し敵がいれば敵に向かっていく
	void AttackFirstStepBegin();
	//攻撃間合い詰めの毎フレーム更新
	void AttackFirstStepTick();
	//攻撃間合い詰め終了
	void AttackFirstStepEnd();

	//固定する敵をセットする
	void SetLockedAttackTarget(const AEnemyBase* Enemy);

	//攻撃をリセットする
	void ResetAttack();

	//各種状態のセッター
	void SetCanAttack(bool _CanAttack) { m_CanAttack = _CanAttack; }
	void SetNextAttackRequested(bool NextAttackRequested) { m_NextAttackRequested = NextAttackRequested; }
	void SetCanBufferAttack(bool CanBufferAttack) { m_CanBufferAttack = CanBufferAttack; }
	void SetIsAirAttackStart(bool IsAirAttackStart) { m_IsAirAttackStart = IsAirAttackStart; }
	void SetIsAttack(const bool _isAttask) { m_IsAttack = _isAttask; }
	void SetHeavyAttackStart(const bool _HeavyAttackStart) { m_HeavyAttackStart = _HeavyAttackStart; }
	void SetComboIndex(const int32 _ComboIndex) { m_ComboIndex = _ComboIndex; }
	void SetJustEvasiveLongCharge(const bool _LongCharge) { m_JustEvasiveLongCharge = _LongCharge; }

	//これを呼ぶことにより一つで攻撃状態の条件を確認できる
	bool GetIsInAttackState()const;

	//各種状態のゲッター
	bool GetCanAttack()const { return m_CanAttack; }
	bool GetHeavyAttackStart()const { return m_HeavyAttackStart; }
	bool GetIsAttack()const { return m_IsAttack; }
	bool GetNextAttackRequested()const { return m_NextAttackRequested; }
	bool GetCanBufferAttack()const { return m_CanBufferAttack; }
	bool GetIsAirAttackStart()const { return m_IsAirAttackStart; }
	bool GetIsHeavyCharging()const { return m_IsHeavyCharging; }
	bool GetJustEvasiveLongCharge()const { return m_JustEvasiveLongCharge; }
	float GetCurrentHeavyChargeTime()const { return m_HeavyChargeTime; }

	//ジャンプ開始時
	void OnJumpStarted();

private:
	//ロックする敵をクリア
	void ClearLockedAttackTarget();
	//ロックする敵が存在しアクティブか確認
	bool HasLockedAttackTarget()const;

	//リセット用ヘルパー関数群
	//地上攻撃リセット
	void ResetAttackFlags();
	//空中攻撃リセット
	void ResetAirAttackFlags();
	//踏み込み移動リセット
	void ResetMovementState();
	//回避リセット
	void ResetEvasiveState();
	//キャンセルアビリティ
	void CancelAttackAbilities();

	//攻撃可能状態か
	bool CanAcceptAttackInput() const;

	//ジャンプ中か
	bool IsJumping()const;

	//ターゲットの自動検索
	bool TryTargetAutoSearch(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput);
	//ターゲットがいない場合の前方設定
	void TargetForward(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput);
	//ターゲット位置の適用
	void ApplyTargetLocation(const FVector& TargetLocation, const FVector& DirectionToTarget);

protected:
	//プレイヤーポインタ
	APlayerCharacter* m_Player = nullptr;

	//各種コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_MovementComponent* m_MovementComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_EvasiveComponent* m_EvasiveComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_CameraComponent* m_CameraComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_ElectroGaugeComponent* m_ElectroComponent = nullptr;

	//プレイヤーのパラメーター(BPで操作できる値のみ)
	FPlayerParam m_PlayerParam;

	//固定するターゲット
	TWeakObjectPtr<const AEnemyBase> m_LockedAttackTarget = nullptr;

private:

	//攻撃が可能か
	bool m_CanAttack = true;

	//コンボ回数
	int32 m_ComboIndex = 0;

	//次の攻撃をリクエストしているか
	bool m_NextAttackRequested = false;

	//攻撃の先行入力（バッファ）ができるか
	bool m_CanBufferAttack = false;

	//弱攻撃が開始しているか
	bool m_IsAttack = false;

	//空中攻撃は一ジャンプ一度
	bool m_IsAirAttackStart = false;

	//攻撃中、敵に吸い付くための目標位置を持っているか
	bool m_HasAttackTargetLocation = false;

	//攻撃中の移動量
	float m_MoveStep = 0.f;

	//弱の空中攻撃のダッシュ斬りが発動中か
	bool m_IsAirDashAttack = false;

	//強の空中攻撃のたたき落としが発動中か
	bool m_IsAirFallAttack = false;

	//強の空中攻撃が溜め状態か
	bool m_IsAirFallCharging = false;

	//溜め攻撃中か
	bool m_IsHeavyCharging = false;

	//地上ため時間
	float m_HeavyChargeTime = 0.f;

	//空中ため時間
	float m_AirFallChargeTime = 0.f;

	//強攻撃が開始しているか
	bool m_HeavyAttackStart = false;

	//ジャンプ開始時の高さ
	float m_JumpStartZ = 0.f;

	//空中攻撃が解禁されたか
	bool m_CanAirAttack = false;

	//次の攻撃をロング扱いに
	bool m_JustEvasiveLongCharge = false;

	//Notify開始時に決定した「踏み込み先の位置」
	FVector m_AttackTargetLocation = FVector::ZeroVector;

	//ダメージ情報
	FDamageInfo m_DamageInfo;
};
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
	// Sets default values for this component's properties
	UPlayer_AttackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void _updateAttackHeavy(float DeltaTime);

	void _updateAirAttackUnlock();

	//弱攻撃
	void Input_AttackLight(const FInputActionValue& Value);

	//強攻撃
	void Input_AttackHeavy(const FInputActionValue& Value);

	//溜め離したとき
	void Completed_AttackHeavy(const FInputActionValue& Value);

	//ジャスト回避攻撃
	void JustEvasiveAttack();

	//空中
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
	void AttackFirstStepTick();
	//攻撃間合い詰め終了
	void AttackFirstStepEnd();

	//固定する敵をセットする
	void SetLockedAttackTarget(const AEnemyBase* Enemy);

	//攻撃をリセットする
	void ResetAttack();

	void SetCanAttack(bool _CanAttack) { m_CanAttack = _CanAttack; }
	void SetNextAttackRequested(bool NextAttackRequested) { m_NextAttackRequested = NextAttackRequested; }
	void SetCanBufferAttack(bool CanBufferAttack) { m_CanBufferAttack = CanBufferAttack; }
	void SetIsAirAttackStart(bool IsAirAttackStart) { m_IsAirAttackStart = IsAirAttackStart; }
	void SetIsAttack(const bool _isAttask) { m_IsAttack = _isAttask; }
	void SetHeavyAttackStart(const bool _HeavyAttackStart) { m_HeavyAttackStart = _HeavyAttackStart; }
	void SetComboIndex(const int32 _ComboIndex) { m_ComboIndex = _ComboIndex; }
	void SetJustEvasiveLongCharge(const bool _LongCharge) { m_JustEvasiveLongCharge = _LongCharge; }

	//これを呼ぶことにより一つで条件を確認できる
	bool GetIsInAttackState()const;
	bool GetCanAttack()const { return m_CanAttack; }
	bool GetHeavyAttackStart()const { return m_HeavyAttackStart; }
	bool GetIsAttack()const { return m_IsAttack; }
	bool GetNextAttackRequested()const { return m_NextAttackRequested; }
	bool GetCanBufferAttack()const { return m_CanBufferAttack; }
	bool GetIsAirAttackStart()const { return m_IsAirAttackStart; }
	bool GetIsHeavyCharging()const { return m_IsHeavyCharging; }
	bool GetJustEvasiveLongCharge()const { return m_JustEvasiveLongCharge; }
	float GetCurrentHeavyChargeTime()const { return m_HeavyChargeTime; }


	// ジャンプ開始時
	void OnJumpStarted();
private:
	//ロックする敵をクリア
	void ClearLockedAttackTarget();
	bool HasLockedAttackTarget()const;


	//リセット
	//地上攻撃リセット
	void ResetAttackFlags();
	//空中攻撃
	void ResetAirAttackFlags();
	//踏み込み移動
	void ResetMovementState();
	//回避
	void ResetEvasiveState();
	//キャンセルアビリティ
	void CancelAttackAbilities();

	//攻撃可能状態か
	bool CanAcceptAttackInput() const;

	//ジャンプ中か
	bool IsJumping()const;

protected:
	APlayerCharacter* m_Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_MovementComponent* m_MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_EvasiveComponent* m_EvasiveComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_CameraComponent* m_CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_ElectroGaugeComponent* m_ElectroComponent;

	//プレイヤーのパラメーター
//BPで操作できる値のみ
	FPlayerParam PlayerParam;

	//固定するターゲット
	const AEnemyBase* m_LockedAttackTarget;
private:

	//攻撃が可能か
	bool m_CanAttack;

	//コンボ回数
	int32 m_ComboIndex;

	//次の攻撃をリクエスト
	bool m_NextAttackRequested;

	//攻撃出来る
	bool m_CanBufferAttack;

	//弱攻撃が開始しているか
	bool m_IsAttack;

	//空中攻撃は一ジャンプ一度
	bool m_IsAirAttackStart;

	// 攻撃中、敵に吸い付くための目標位置を持っているか
	bool m_HasAttackTargetLocation;

	//攻撃中の移動量
	float m_MoveStep;

	//弱の空中攻撃のダッシュ斬りが発動中か
	bool m_IsAirDashAttack;

	//強の空中攻撃のたたき落としが発動中か
	bool m_IsAirFallAttack;

	//強の空中攻撃が溜め状態か
	bool  m_IsAirFallCharging;

	//溜め攻撃中か
	bool m_IsHeavyCharging;

	//地上ため時間
	float m_HeavyChargeTime;

	//空中ため時間
	float m_AirFallChargeTime;

	//強攻撃が開始しているか
	bool m_HeavyAttackStart;

	// ジャンプ開始時の高さ
	float m_JumpStartZ;

	// 空中攻撃が解禁されたか
	bool m_CanAirAttack;

	//次の攻撃をロング扱いに
	bool m_JustEvasiveLongCharge;

	// Notify開始時に決定した「踏み込み先の位置」
	FVector m_AttackTargetLocation;

	FDamageInfo DamageInfo;
};

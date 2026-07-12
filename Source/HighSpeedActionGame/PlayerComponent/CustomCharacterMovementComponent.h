//担当
//伊藤直樹

//-----------------------------------------------------
//カスタムキャラクター移動コンポーネント
//
// UCharacterMovementComponentを継承し、
// プレイヤー専用の移動状態管理とVelocity反映を担当する
//
// 速度計算そのものはPlayerMovementSolverへ委譲する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/PlayerMovementTypes.h"
#include "CustomCharacterMovementComponent.generated.h"

class UPlayerMovementParameter;

//移動状態変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPlayerMovementStateChanged,
	EPlayerMovementState, PreviousState,
	EPlayerMovementState, NewState
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	//初期化
	UCustomCharacterMovementComponent();

protected:
	//開始時
	virtual void BeginPlay() override;

	//毎フレーム更新
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	//MovementMode変更時
	virtual void OnMovementModeChanged(
		EMovementMode PreviousMovementMode,
		uint8 PreviousCustomMode
	) override;

	//移動計算前の状態更新
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	//キャラクター回転制御
	virtual void PhysicsRotation(float DeltaTime) override;

	//最大速度取得
	virtual float GetMaxSpeed() const override;

public:
	//移動入力を更新
	void UpdateMovementInput(const FVector2D& InputVector);

	//移動入力をクリア
	void ClearMovementInput();

	//ダッシュ開始
	bool StartDash();

	//Dash中か取得
	bool IsDashing() const;

	//Dashを終了
	void EndDash();

	//ジャンプ開始
	bool StartJump();

	//ジャンプ入力を離した時
	void ReleaseJump();

	//回避開始
	bool StartEvasive();


	//回避終了
	void EndEvasive();

	//ジャスト回避カウンター移動開始
	void StartJustEvasiveCounterMove(const AActor* TargetActor);


	//移動状態を変更
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetMovementState(EPlayerMovementState NewState);

	//移動状態をリセット
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ResetMovementState();

	//移動可能か設定
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetCanMovement(bool bCanMove);

	//外部速度倍率を設定
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetExternalSpeedMultiplier(float NewMultiplier);

	//現在の移動状態を取得
	EPlayerMovementState GetMovementState() const { return m_CurrentMovementState; }

	//移動入力状態を取得
	const FPlayerMovementInputState& GetMovementInputState() const { return m_InputState; }

	//現在の入力値を取得
	FVector2D GetCurrentMoveInput() const { return m_InputState.RawInput; }

	//現在の移動希望方向を取得
	FVector GetWishDirection() const { return m_InputState.WorldDirection; }

	//移動可能か
	bool GetCanMovement() const { return m_bCanMovement; }

	//ダッシュ中か
	bool GetIsDash() const;

	//回避中か
	bool GetIsEvasive() const;

	//空中状態か
	bool GetIsAirborne() const;

	//ジャンプクールタイム中か
	bool GetIsJumpCooldown() const { return m_bIsJumpCooldown; }

	//操作方向の平面回転を取得
	FRotator GetControlRotationFlat() const;

	//入力方向を取得
	bool GetMoveInputDirection(FVector& OutDirection) const;

	//入力がなければ正面方向を返す
	bool GetDesiredMoveDirection(FVector& OutDirection) const;

	//移動パラメータを取得
	const UPlayerMovementParameter* GetMovementParameter() const { return m_MovementParameter; }

	//最後に開始した回避方向タイプを取得
	EPlayerEvasiveDirectionType GetLastEvasiveDirectionType() const { return m_LastEvasiveDirectionType; }

	//Dash停止要求
	void RequestStopDash();

	//Dashを強制キャンセル
	void CancelDash();

	//攻撃開始時の移動入力を保存
	void CacheAttackStartInput();

	//攻撃開始時の移動入力をクリア
	void ClearAttackStartInput();

	//攻撃開始時の移動入力状態を取得
	const FPlayerMovementInputState& GetAttackStartInputState() const
	{
		return m_AttackStartInputState;
	}

	//攻撃開始時の移動入力を生入力から保存
	void CacheAttackStartInputFromRaw(const FVector2D& InputVector);
public:
	//移動状態変更通知
	UPROPERTY(BlueprintAssignable, Category = "Movement|Event")
	FOnPlayerMovementStateChanged OnMovementStateChanged;

private:
	//パラメータ初期値を反映
	void ApplyParameterDefaults();

	//移動入力を減衰
	void DecayMovementInput();

	//アイドル状態を更新
	void UpdateIdleRotation();

	//ダッシュ状態を更新
	void UpdateDashState(float DeltaTime);

	//Dash減速へ移行
	void BeginDashDecel();

	//Dash開始方向を計算
	FVector CalculateDashStartDirection() const;

	//ジャンプ長押し時間を更新
	void UpdateJumpHold(float DeltaTime);

	//回避後硬直を更新
	void UpdateEvasiveRecovery(float DeltaTime);

	//ジャスト回避後の敵方向固定を更新
	void UpdateJustEvasiveFacingLock(float DeltaTime);

	//ジャスト回避後の敵方向固定を終了
	void EndJustEvasiveFacingLock();


	//ジャンプクールタイム開始
	void StartJumpCooldown();

	//ジャンプクールタイム終了
	void OnJumpCooldownFinished();

	//ダッシュ開始可能か
	bool CanStartDash() const;

	//ジャンプ開始可能か
	bool CanStartJump() const;

	//回避開始可能か
	bool CanStartEvasive() const;

	//入力があるか
	bool HasMoveInput() const;

	//状態変更通知
	void BroadcastMovementStateChanged(EPlayerMovementState PreviousState, EPlayerMovementState NewState);

	//移動状態をGameplayTagへ反映
	void UpdateMovementStateGameplayTags(EPlayerMovementState NewState);
private:
	//移動パラメータ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameter", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerMovementParameter> m_MovementParameter = nullptr;

	//現在の移動状態
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	EPlayerMovementState m_CurrentMovementState = EPlayerMovementState::Normal;

	//移動入力状態
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FPlayerMovementInputState m_InputState;

	//ダッシュ方向
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	FVector m_LockedDashDirection = FVector::ForwardVector;

	//回避方向
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Evasive", meta = (AllowPrivateAccess = "true"))
	FVector m_LockedEvasiveDirection = FVector::ForwardVector;

	//移動可能か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool m_bCanMovement = true;

	//アイドル中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool m_bIsIdle = false;

	//ジャンプ長押し中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Jump", meta = (AllowPrivateAccess = "true"))
	bool m_bIsJumpCharging = false;

	//ジャンプクールタイム中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Jump", meta = (AllowPrivateAccess = "true"))
	bool m_bIsJumpCooldown = false;

	//ジャンプ長押し時間
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Jump", meta = (AllowPrivateAccess = "true"))
	float m_JumpHoldTime = 0.f;

	//回避後硬直残り時間
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Evasive", meta = (AllowPrivateAccess = "true"))
	float m_EvasiveRecoveryTimer = 0.f;

	//外部速度倍率
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float m_ExternalSpeedMultiplier = 1.f;

	//現在のダッシュ経過時間
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	float m_DashElapsedTime = 0.f;
private:
	//初期ブレーキ力
	float m_DefaultBrakingDecelerationWalking = 2048.f;

	//初期摩擦
	float m_DefaultGroundFriction = 8.f;

	//初期ブレーキ摩擦係数
	float m_DefaultBrakingFrictionFactor = 2.f;

	//初期空中操作
	float m_DefaultAirControl = 0.5f;

	//ジャンプクールタイム用タイマー
	FTimerHandle m_JumpCooldownTimerHandle;

	//最後に開始した回避方向タイプ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Evasive", meta = (AllowPrivateAccess = "true"))
	EPlayerEvasiveDirectionType m_LastEvasiveDirectionType = EPlayerEvasiveDirectionType::BackStep;

	//ジャスト回避後に敵方向を固定中か
	bool m_bJustEvasiveFacingLock = false;

	//ジャスト回避後の敵方向固定残り時間
	float m_JustEvasiveFacingLockTime = 0.f;

	//敵方向固定前の自動回転設定
	bool m_bSavedOrientRotationToMovement = true;

	//攻撃開始時の移動入力状態
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Attack", meta = (AllowPrivateAccess = "true"))
	FPlayerMovementInputState m_AttackStartInputState;

	//ダッシュ中に移動入力がない経過時間
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Movement|Dash",
		meta = (AllowPrivateAccess = "true")
	)
	float m_DashNoInputElapsedTime = 0.f;
};
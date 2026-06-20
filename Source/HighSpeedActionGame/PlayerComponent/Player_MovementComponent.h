//担当
//伊藤直樹
//-----------------------------------------------------
//プレイヤーの移動状態、移動入力を管理するコンポーネント
//
//前後左右の移動・アイドル・ダッシュ・ジャンプの移動状態を管理
//
//-----------------------------------------------------
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "../PlayerParam.h"
#include "Player_MovementComponent.generated.h"

class APlayerCharacter;
class UInputMappingContext;
class UInputAction;
class UPlayer_CameraComponent;
class UPlayer_EvasiveComponent;
class UPlayer_AttackComponent;
class UPlayer_ElectroGaugeComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayer_MovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//コンストラクタ
	UPlayer_MovementComponent();

protected:
	//ゲーム開始時に呼ばれる処理
	virtual void BeginPlay() override;

	//毎フレーム呼ばれる処理
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	//===更新処理===

	//アイドル状態
	//地上にいるか移動してるかを判定
	void _updateIdle();

	//ダッシュ
	//入力が無くなったらダッシュ終了、ロックオン中のカメラ処理
	void _updateDash(float DeltaTime);

	//ジャンプ
	//ジャンプの上昇処理
	void _updateJump(float DeltaTime);

	//入力コールバック関連
	//移動
	//前後
	void Input_MoveForward(const FInputActionValue& Value);
	//左右
	void Input_MoveRight(const FInputActionValue& Value);

	//ジャンプ入力の受付
	void Input_Jump(const FInputActionValue& Value);
	//ジャンプ入力離した
	void Input_JumpRelease();

	//ジャンプ開始処理
	void StartJump(float JumpZVelocity);

	//ダッシュ終了処理
	void EndDash();

	//ダッシュ入力
	void Input_Dash(const FInputActionValue& Value);

	//プレイヤーが動こうとしている方向
	//入力が無い場合はfalseを返す
	bool GetDesiredMoveDirection(FVector& OutDirection) const;

	//ジャスト回避成功時敵の方向を向く
	//攻撃やカメラ制御は行わない
	void OnJustEvasive(const AActor* Target);

	//===状態をセット===

	//移動関連が操作できるかどうか
	void SetCanMovement(bool CanMovement) { m_CanMovement = CanMovement; }

	//ダッシュの状態
	void SetIsDash(bool IsDash) { m_IsDash = IsDash; }
	//ダッシュ開始状態
	void SetStartDash(bool StartDash) { m_DashStart = StartDash; }
	//アイドル中か
	void SetIsIdle(bool IsIdle) { m_IsIdle = IsIdle; }
	//ジャンプ状態をplayerクラスの地面についたときにfalseにする
	void SetIsJump(bool IsJump) { m_IsJump = IsJump; }

	//着地時にキャラクタークラスから呼ぶ関数
	void OnPlayerLanded();

	//===状態の取得===

	//移動操作可能か
	bool GetCanMovement() const { return m_CanMovement; }
	//ダッシュ中か
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool GetIsDash() const { return m_IsDash; }
	//ダッシュ開始したか
	bool GetStartDash() const { return m_DashStart; }
	//アイドル中か
	bool GetIsIdle() const { return m_IsIdle; }
	//ジャンプ中か
	bool GetIsJump() const { return m_IsJump; }

	//現在の入力
	FVector2D GetCurrentMoveInput() const { return m_CurrentMoveInput; }

	//入力があるか(実装はCPPにてマジックナンバーを回避)
	bool GetIsMoveInput() const;

	//Z軸の回転成分のみを抽出したコントロール回転を取得
	FRotator GetControlRotationFlat() const;

	//入力方向取得
	bool GetMoveInputDirection(FVector& Direction) const;

private:
	//クールタイム終了時に呼ばれる関数
	void OnJumpCooldownFinished();

protected:
	//プレイヤーの参照
	APlayerCharacter* m_Player = nullptr;

	//プレイヤーのパラメータ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MoveParam")
	FPlayerParam m_PlayerParam;

	//各コンポーネントの参照
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_CameraComponent* m_CameraComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_EvasiveComponent* m_EvasiveComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_AttackComponent* m_AttackComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_ElectroGaugeComponent* m_ElectroComponent = nullptr;

	//ダッシュエフェクトのアセット
	UPROPERTY(EditAnywhere, Category = "Dash|Effect")
	UNiagaraSystem* m_DashEffect = nullptr;

	//生成されたNiagaraComponent
	UPROPERTY()
	UNiagaraComponent* m_DashEffectComp = nullptr;

private:
	//===パラメータ===

	//現在の入力方向
	FVector2D m_CurrentMoveInput = FVector2D::ZeroVector;

	//着地ディレイ中の慣性用
	FVector m_LandingInertiaVelocity = FVector::ZeroVector;

	//移動関連が操作できるかどうか
	bool m_CanMovement = true;

	//アイドル状態かどうか
	bool m_IsIdle = false;

	//ジャンプ中かどうか
	bool m_IsJump = false;

	//押下時間
	float m_JumpHoldTime = 0.0f;
	//押下中・判定待ち
	bool m_IsJumpCharging = false;

	//ダッシュ状態かどうか
	bool m_IsDash = false;

	//ダッシュ開始したか
	bool m_DashStart = false;

	//入力があるか
	bool m_HasMoveInput = false;

	//ダッシュの方向
	FVector m_DashDirection = FVector::ZeroVector;

	//デフォルトのブレーキ力と摩擦力の一時保存用
	float m_DefaultBrakingDecelerationWalking = 2048.0f;
	float m_DefaultBrakingFrictionFactor = 0.0f;
	float m_DefaultGroundFriction = 8.0f;

	//ダッシュの減速中（慣性移動中）かどうか
	bool m_IsDashDecelerating = false;

	//ジャンプのクールタイム中かどうか
	bool m_IsJumpCooldown = false;

	//タイマーハンドル
	FTimerHandle m_JumpCooldownTimerHandle;
};
//担当
//伊藤直樹



//-----------------------------------------------------
//プレイヤーの移動状態、移動入力を管理するコンポーネント
//
// 前後左右の移動・アイドル・ダッシュ・ジャンプの移動状態を管理
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
	// Sets default values for this component's properties
	UPlayer_MovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;

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

	void StartJump(float _JumpZVelocity);

	void EndDash();

	//ダッシュ
	void Input_Dash(const FInputActionValue& Value);



	//プレイヤーが動こうとしている方向
	//入力が無い場合は false を返す
	bool GetDesiredMoveDirection(FVector& OutDirection) const;

	//ジャスト回避成功時敵の方向を向く
	//攻撃やカメラ制御は行わない
	void OnJustEvasive(const AActor* Target);

	//===状態をセット===
	//移動関連が操作できるかどうか
	void SetCanMovement(bool _CanMovement) { m_CanMovement = _CanMovement; }

	//ダッシュの状態
	void SetIsDash(bool Enable) { m_IsDash = Enable; }
	//ダッシュ開始状態
	void SetStartDash(bool startDash) { m_DashStart = startDash; }
	//アイドル中か
	void SetIsIdle(bool Idle) { m_IsIdle = Idle; }
	//ジャンプ状態をplayerクラスの地面についたときにfalseにする
	void SetIsJump(bool _IsJump) { m_IsJump = _IsJump; }

	// 着地時にキャラクタークラスから呼ぶ関数
	void OnPlayerLanded();

	//状態を与える
	//ダッシュ中か
	bool GetCanMovement()const { return m_CanMovement; }
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool GetIsDash()const { return m_IsDash; }
	//ダッシュ開始したか
	bool GetStartDash()const { return m_DashStart; }
	//アイドル中か
	bool GetIsIdle()const { return m_IsIdle; }

	//ジャンプ中か
	bool GetIsJump()const { return m_IsJump; }
	//現在の入力
	FVector2D GetCurrentMoveInput()const { return m_CurrentMoveInput; }

	//入力があるか
	bool GetIsMoveInput()const { return m_CurrentMoveInput.SizeSquared() >= 0.1f; }

	FRotator GetControlRotationFlat()const;


	//入力方向取得
	bool GetMoveInputDirection(FVector& Direction)const;


private:
	// クールタイム終了時に呼ばれる関数
	void OnJumpCooldownFinished();
protected:

	APlayerCharacter* m_Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MoveParam")
	FPlayerParam PlayerParam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_CameraComponent* m_CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_EvasiveComponent* m_EvasiveComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_AttackComponent* m_AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_ElectroGaugeComponent* m_ElectroComponent;


	UPROPERTY(EditAnywhere, Category = "Dash|Effect")
	UNiagaraSystem* m_DashEffect;

	// 生成されたNiagaraComponent
	UPROPERTY()
	UNiagaraComponent* m_DashEffectComp;
private:
	//現在の入力方向
	FVector2D m_CurrentMoveInput;

	// 着地ディレイ中の慣性用
	FVector m_LandingInertiaVelocity;

	//移動関連が操作できるかどうか
	bool m_CanMovement;

	//アイドル状態かどうか
	bool m_IsIdle;

	//ジャンプ中かどうか
	bool m_IsJump;

	//押下時間
	float m_JumpHoldTime;
	//押下中・判定待ち
	bool  m_IsJumpCharging;


	//ダッシュ
	//ダッシュ状態かどうか
	bool m_IsDash;

	//ダッシュ開始したか
	bool m_DashStart;

	//入力があるか
	bool m_HasMoveInput;

	//ダッシュの方向
	FVector m_DashDirection;

	float m_DefaultBrakingDecelerationWalking;
	float m_DefaultBrakingFrictionFactor;
	float m_DefaultGroundFriction;

	// ダッシュの減速中（慣性移動中）かどうか
	bool m_IsDashDecelerating;

	// ジャンプのクールタイム中かどうか
	bool m_IsJumpCooldown;

	// タイマーハンドル
	FTimerHandle JumpCooldownTimerHandle;
};

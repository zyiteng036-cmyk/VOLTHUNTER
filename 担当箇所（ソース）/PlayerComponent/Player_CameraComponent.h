//担当
//伊藤直樹

//カメラの管理クラス

#pragma once

#include "CoreMinimal.h"
#include "CameraComponent/ZoomInOut/CameraOverrideHandler.h"
#include "CameraComponent/OpeningCamera/CameraOpeningHandler.h"
#include "CameraComponent/CameraAction/CameraActionHandler.h"
#include "CameraComponent/CameraBoss/CameraBossHandler.h"
#include "CameraComponent/CameraDie/CameraDieHandler.h"
#include "Components/ActorComponent.h"
#include "../PlayerParam.h"
#include "CameraComponent/ClearCamera/ClearCameraParam/ClearCameraPhaseParam.h"
#include "InputActionValue.h"
#include "Player_CameraComponent.generated.h"


class APlayerCharacter;
class UInputMappingContext;
class UInputAction;
class UPlayer_MovementComponent;
class UPlayer_AttackComponent;
class UPlayer_EvasiveComponent;
class USpringArmComponent;
class UCameraComponent;
class AEnemyBase;

UENUM()
enum class ESkillCameraPhase : uint8
{
	None,
	ZoomIn,   // 一瞬寄る
	ZoomOut  // 引く
};

UENUM()
enum class EClearCameraPhase : uint8
{
	None,
	Side,
	FrontAngle
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayer_CameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayer_CameraComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:

	//デッドゾーン判定
	void _updateCameraFocus(float DeltaTime);

	//カメラが一定時間たつと自動で戻る
	void _updateAutoCameraReturn(float DeltaTime);

	//敵の方向へカメラを向ける
	void _updateEnemyDirectionCamera(float DeltaTime);

	//強攻撃チャージカメラ
	bool _updateHeavyChargeCamera(float DeltaTime);

	//スキル使用時のカメラ
	bool _updateSkillCamera(float DeltaTime);

	//クリア時のカメラ
	void _updateClearCameraZoom(float DeltaTime);


	//スキル発動かの通知
	void StartSkillCamera();

	//スキル終了
	void EndSkillCamera();

	void CancellationLockOn();

	//カメラシェイクを終了
	void CameraShakEnd();

	// カメラの視界内にあるか判定（汎用的なのでComponent本体に持つ）
	bool IsLocationInCameraView(const FVector& WorldLocation) const;

	//入力コールバック関連
	//カメラ
	//ピッチ
	void Input_CameraRotatePitch(const FInputActionValue& Value);
	//ヨウ
	void Input_CameraRotateYaw(const FInputActionValue& Value);

	//カメラリセット
	void Input_CameraReset(const FInputActionValue& Value);

	//カメラロックオン
	void Input_TargetLockOn(const FInputActionValue& Value);

	//ターゲットを変える
	void Input_TargetChange(const FInputActionValue& Value);

	void OnJEnemyDirection(const AActor* Target, bool _LowAngle = false);

	// ロックオン中か返す関数
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	bool GetIsTargetLockedOn() const { return m_IsTargetLockOn; }

	void SetIsTargetLockedOn(const bool _IsTargetLockOn) { m_IsTargetLockOn = _IsTargetLockOn; }

	//ロックオンカメラを使用しているかを与える
	TObjectPtr<UCameraComponent> GetLockOnCamera() const { return m_LockOnCamera; }


	//ゲームクリア時のカメラ
	void ClearCamera();

	//カメラリセット
	UFUNCTION(BlueprintCallable, Category = "Reset")
	void ResetCamera();

private:
	//後ろ方向から
	void SetBackClearCamera();
	//真横から
	void SetSideClearCamera();
	//斜め前から
	void SetFrontClearCamera();

public:
	//ZoomInOutHandller
	// NotifyStateから呼ばれる関数
	void StartAnimCameraOverride(float TargetLength, float TargetFOV, float Speed);
	void EndAnimCameraOverride();

	//オープニングHandller
	// ゲームモードから呼ぶ関数
	void PlayInGameOpening();

	// カメラの横位置
	void SetCameraSideOffset(bool bIsSideView, float OffsetAmount = 150.f);

	//死亡時
	void OnPlayerDie();
private:
	//曲線補間
	FVector SmoothDamp(FVector Current, FVector Target, FVector& Velocity, float SmoothTime, float DeltaTime);
	float GetCameraFollowTime()const;

	//ボス戦の開始イベントを受け取る
	UFUNCTION()
	void OnBossBattleStart();

	//ボス戦の終了イベントを受け取る
	UFUNCTION()
	void OnBossBattleEnd();
protected:
	APlayerCharacter* m_Player;
	FPlayerParam PlayerParam;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_MovementComponent* m_MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_AttackComponent* m_AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_EvasiveComponent* m_EvasiveComponent;

	UPROPERTY()
	EClearCameraPhase m_ClearCameraPhase = EClearCameraPhase::None;

	// 背面クリアカメラ用設定
	UPROPERTY(EditAnywhere, Category = "ClearCamera|Phase")
	FClearCameraPhaseParam BackClearParam;

	// 横クリアカメラ用設定
	UPROPERTY(EditAnywhere, Category = "ClearCamera|Phase")
	FClearCameraPhaseParam SideClearParam;

	// 正面クリアカメラ用設定
	UPROPERTY(EditAnywhere, Category = "ClearCamera|Phase")
	FClearCameraPhaseParam FrontClearParam;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Camera Opening")
	UCameraOpeningHandler* OpeningHandler;
	// オープニング中かどうか
	bool m_IsOpeningCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Boss Camera")
	UCameraBossHandler* BossHandler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Die Camera")
	UCameraDieHandler* DieHandler;

private:
	// 演出管理クラスのインスタンス
	UPROPERTY()
	UCameraOverrideHandler* OverrideHandler;



	// ハンドラ定義
	UPROPERTY()
	UCameraActionHandler* ActionHandler;


private:
	//カメラ設定
//スプリングアーム
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> m_SpringArm;

	//カメラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> m_Camera;


	//ロックオン用スプリングアーム
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> m_LockOnSpringArm;
	//ロックオン用カメラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> m_LockOnCamera;


	//コントローラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerController, meta = (AllowPrivateAccess = "true"))

	TWeakObjectPtr<APlayerController> m_PlayerController;


	const AActor* m_EnemyTarget;

	//ジャスト回避時の補間速度
	float m_EnemyDirectionInterpSpeed;

	bool m_IsEnemyDirectionLooking;


	//ロックオン
	bool m_IsTargetLockOn;

	//Pitchの最小可動域
	float m_MinPitch;

	//Pitchの最大可動域
	float m_MaxPitch;


	//最後にカメラ入力があった時間
	float m_LastCamerainputTime;

	//カメラが自動で戻る待ち時間
	float m_CameraAutoDelay;

	//カメラの自動補間速度
	float m_CameraReturnSpeed;

	//===強攻撃チャージ===
	// 近づけた距離
	float m_HeavyChargeArmLength;
	//近づける際の補間速度
	float m_HeavyChargeInterpSpeed;


	//スキル０１発動時のカメラ
	ESkillCameraPhase m_SkillCameraPhase = ESkillCameraPhase::None;

	// 距離
	float m_SkillZoomInArmLength; // 一瞬寄る距離
	float m_SkillZoomOutArmLength; // 引いた後の距離

	// 速度
	float m_SkillZoomInSpeed; // 速く
	float m_SkillZoomOutSpeed;  // 少しゆっくり

	//ゲームクリア時のカメラ
	bool m_IsClearCamera;

	//クリア時どの時間でカメラを切り替えるか
	FTimerHandle m_SideClearTimer;
	FTimerHandle m_FrontClearTimer;
	FTimerHandle m_BackClearTimer;

	//クリア時のズーム
	float m_ClearCurrentArmLength;
	float m_ClearTargetArmLength;

	float m_ClearZoomLength;

	float m_ClearZoomInterpSpeed;

	// ---- Reset 用保存値 ----
	float   m_DefaultArmLength;
	FVector m_DefaultSocketOffset;
	FRotator m_DefaultControlRotation;
	bool    m_DefaultUsePawnControlRotation;

	//焦点
	FVector m_CameraFocusLocation;
	FVector m_CameraFocusVelocity;
	// DeadZone 用
	FVector m_CameraTargetLocation;


	// 補間用のターゲットオフセット
	FVector m_TargetSocketOffset;

	// 既存のTick内に補間処理を追加するための変数
	UPROPERTY(EditAnywhere, Category = "Camera|Tutorial")
	float m_SideOffsetInterpSpeed = 5.0f;

	bool m_bInvertCameraX = false;
	bool m_bInvertCameraY = false;
};

//担当
//伊藤直樹

//-----------------------------------------------------
//カメラリグ共通型
//
// CameraRigComponentとCameraLayerが共有する状態、
// 演出要求、ターゲット情報、最終出力値を定義する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "CameraRigTypes.generated.h"

class UCameraParameter;
class USpringArmComponent;
class UCameraComponent;

//カメラ基本モード
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	//通常
	Normal,

	//Boss戦
	BossBattle,

	//演出中
	Action
};

//カメラ要求の種類
UENUM(BlueprintType)
enum class ECameraRequestType : uint8
{
	//なし
	None,

	//オープニング
	Opening,

	//クリア
	Clear,

	//死亡
	Die,

	//スキル
	Skill,

	//ジャスト回避後の敵注視
	JustEvasiveLook,

	//Spline移動
	SplineMove,

	//AnimNotifyからの一時上書き
	AnimOverride
};

//クリアカメラのフェーズ
UENUM(BlueprintType)
enum class EClearCameraPhase : uint8
{
	//なし
	None,

	//背面
	Back,

	//側面
	Side,

	//正面
	Front
};

//カメラ注視ターゲット情報
USTRUCT(BlueprintType)
struct FCameraTargetInfo
{
	GENERATED_BODY()

public:
	//ターゲットを持つか
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Target")
	bool bHasTarget = false;

	//ターゲットActor
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Target")
	TObjectPtr<AActor> TargetActor = nullptr;

	//ターゲット座標
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Target")
	FVector TargetLocation = FVector::ZeroVector;

	//低めの角度で見るか
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Target")
	bool bLowAngle = false;

public:
	//ターゲットを設定
	void SetTarget(AActor* Actor, bool bInLowAngle)
	{
		TargetActor = Actor;
		bHasTarget = IsValid(Actor);
		bLowAngle = bInLowAngle;
		TargetLocation = bHasTarget ? Actor->GetActorLocation() : FVector::ZeroVector;
	}

	//ターゲット座標を更新
	void UpdateTargetLocation()
	{
		if (!IsValid(TargetActor))
		{
			Reset();
			return;
		}

		TargetLocation = TargetActor->GetActorLocation();
	}

	//ターゲットを初期化
	void Reset()
	{
		bHasTarget = false;
		TargetActor = nullptr;
		TargetLocation = FVector::ZeroVector;
		bLowAngle = false;
	}
};

//カメラ要求
USTRUCT(BlueprintType)
struct FCameraRequest
{
	GENERATED_BODY()

public:
	//要求中か
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Request")
	bool bIsActive = false;

	//要求種類
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Request")
	ECameraRequestType RequestType = ECameraRequestType::None;

	//クリアカメラフェーズ
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Request")
	EClearCameraPhase ClearPhase = EClearCameraPhase::None;

	//経過時間
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Request")
	float ElapsedTime = 0.f;

	//自動終了時間
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Request")
	float Duration = 0.f;

	//ターゲット情報
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Request")
	FCameraTargetInfo TargetInfo;

public:
	//要求を開始
	void Start(ECameraRequestType InRequestType)
	{
		bIsActive = true;
		RequestType = InRequestType;
		ClearPhase = EClearCameraPhase::None;
		ElapsedTime = 0.f;
		Duration = 0.f;
		TargetInfo.Reset();
	}

	//時間を進める
	void Tick(float DeltaTime)
	{
		if (!bIsActive) return;

		ElapsedTime += DeltaTime;
	}

	//進行率を取得
	float GetAlpha() const
	{
		if (Duration <= 0.f) return 1.f;

		return FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f);
	}

	//自動終了対象か
	bool CanAutoFinish() const
	{
		return Duration > 0.f;
	}

	//終了したか
	bool IsFinished() const
	{
		return CanAutoFinish() && GetAlpha() >= 1.f;
	}

	//初期化
	void Reset()
	{
		bIsActive = false;
		RequestType = ECameraRequestType::None;
		ClearPhase = EClearCameraPhase::None;
		ElapsedTime = 0.f;
		Duration = 0.f;
		TargetInfo.Reset();
	}
};
//カメラレイヤーが共有する毎フレーム情報
USTRUCT(BlueprintType)
struct FCameraContext
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------
	// 入力情報
	//-----------------------------------------------------

	//World
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	TObjectPtr<UWorld> World = nullptr;

	//カメラ所有Actor
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	TObjectPtr<AActor> OwnerActor = nullptr;

	//DeltaTime
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	float DeltaTime = 0.f;

	//カメラパラメータ
	const UCameraParameter* CameraParam = nullptr;

	//現在のカメラモード
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	ECameraMode CurrentMode = ECameraMode::Normal;

	//現在のカメラ回転
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	FRotator CameraRotation = FRotator::ZeroRotator;

	//現在のカメラ位置
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	FVector CameraLocation = FVector::ZeroVector;

	//SpringArm参照
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	TObjectPtr<USpringArmComponent> SpringArm = nullptr;

	//Camera参照
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	TObjectPtr<UCameraComponent> Camera = nullptr;

	//プレイヤーが手動でカメラ操作中か
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	bool bIsPlayerMovingCamera = false;

	//初回Tickか
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	bool bIsFirstTick = false;

public:
	//-----------------------------------------------------
	// Player / Boss
	//-----------------------------------------------------

	//プレイヤー速度
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	FVector PlayerVelocity = FVector::ZeroVector;

	//Boss戦中か
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	bool bIsBossBattle = false;

	//Bossターゲットを持つか
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	bool bHasBossTarget = false;

	//Boss座標
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	FVector BossLocation = FVector::ZeroVector;

public:
	//-----------------------------------------------------
	// Action
	//-----------------------------------------------------

	//カメラ要求
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Context")
	FCameraRequest CameraRequest;

public:
	//-----------------------------------------------------
	// 出力値
	//-----------------------------------------------------

	//SpringArmの目標距離
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Output")
	float TargetArmLength = 0.f;

	//SpringArmのソケットオフセット
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Output")
	FVector CumulativeSocketOffset = FVector::ZeroVector;

	//SpringArmのターゲットオフセット
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Output")
	FVector CumulativeTargetOffset = FVector::ZeroVector;

	//最終FOV
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Output")
	float FinalFOV = 0.f;

	//SpringArmの追従速度
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Output")
	float CameraLagSpeed = 0.f;

	//カメラの相対回転オフセット
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Output")
	FRotator CumulativeRotationOffset = FRotator::ZeroRotator;

	//ハードクランプ用の最終ワールドオフセット
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Output")
	FVector HardClampOffset = FVector::ZeroVector;

public:
	//出力を初期化
	void ResetOutput()
	{
		TargetArmLength = 0.f;
		CumulativeSocketOffset = FVector::ZeroVector;
		CumulativeTargetOffset = FVector::ZeroVector;
		FinalFOV = 0.f;
		CameraLagSpeed = 0.f;
		CumulativeRotationOffset = FRotator::ZeroRotator;
		HardClampOffset = FVector::ZeroVector;
	}
};
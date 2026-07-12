//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤーカメラ共通定義
//
// CameraRigComponent、CameraLayer、Ability、各Componentが共有する
// カメラ状態とカメラ計算用コンテキストを定義する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "PlayerMovementTypes.h"
#include "PlayerCameraTypes.generated.h"

class UWorld;
class AActor;
class UPlayerCameraParameter;

//プレイヤーカメラの状態
UENUM(BlueprintType)
enum class EPlayerCameraState : uint8
{
	//通常
	Normal,

	//ダッシュ
	Dash,

	//空中
	Airborne,

	//回避
	Evasive,

	//攻撃
	Attack,

	//強攻撃チャージ
	HeavyCharge,

	//スキル
	Skill,

	//敵方向注視
	EnemyDirection,

	//ボス戦
	BossBattle,

	//死亡
	Dead,

	//オープニング
	Opening,

	//クリア
	Clear,

	//操作不能
	Disabled
};

//カメラの計算結果
USTRUCT(BlueprintType)
struct FPlayerCameraResult
{
	GENERATED_BODY()

public:
	//SpringArmの距離
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	float TargetArmLength = 350.f;

	//SpringArmのSocketOffset
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	FVector SocketOffset = FVector::ZeroVector;

	//SpringArmのTargetOffset
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	FVector TargetOffset = FVector::ZeroVector;

	//カメラFOV
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	float FOV = 90.f;

	//カメララグ速度
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	float CameraLagSpeed = 10.f;

	//最終的な回転補正
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	FRotator RotationOffset = FRotator::ZeroRotator;
};

//カメラ計算に必要な入力情報
USTRUCT(BlueprintType)
struct FPlayerCameraContext
{
	GENERATED_BODY()

public:
	//1フレームの経過時間
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	float DeltaTime = 0.f;

	//現在のカメラ状態
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	EPlayerCameraState CameraState = EPlayerCameraState::Normal;

	//現在の移動状態
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	EPlayerMovementState MovementState = EPlayerMovementState::Normal;

	//プレイヤー速度
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	FVector PlayerVelocity = FVector::ZeroVector;

	//現在戦闘中か
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	bool bIsInCombat = false;

	//プレイヤーがカメラを手動操作しているか
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	bool bIsManualCameraInput = false;

	//敵方向を見る処理中か
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	bool bLookAtTarget = false;

	//注視対象位置
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	FVector LookTargetLocation = FVector::ZeroVector;

	//リセット直後の最初のフレームか
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	bool bIsFirstTick = false;
};
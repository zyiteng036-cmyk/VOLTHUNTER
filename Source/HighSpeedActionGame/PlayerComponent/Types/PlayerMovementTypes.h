//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー移動共通定義
//
// GameplayAbility、CustomCharacterMovementComponent、CameraRigComponentが
// 共通で参照する移動状態や入力情報を定義する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "PlayerMovementTypes.generated.h"

//プレイヤーの移動状態
UENUM(BlueprintType)
enum class EPlayerMovementState : uint8
{
	//通常移動
	Normal,

	//方向反転中
	Reverse,

	//ダッシュ中
	Dash,

	//ダッシュ減速中
	DashDecel,

	//空中状態
	Airborne,

	//回避中
	Evasive,

	//回避後硬直
	EvasiveRecovery,

	//操作不能
	Disabled
};

//回避方向の種類
UENUM(BlueprintType)
enum class EPlayerEvasiveDirectionType : uint8
{
	//入力方向へ回避
	InputDirection,

	//入力なしの後ろ回避
	BackStep
};

//移動入力の状態
USTRUCT(BlueprintType)
struct FPlayerMovementInputState
{
	GENERATED_BODY()

public:
	//入力値
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector2D RawInput = FVector2D::ZeroVector;

	//ワールド空間での移動希望方向
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector WorldDirection = FVector::ZeroVector;

	//入力があるか
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bHasInput = false;
};

//移動状態変更通知用データ
USTRUCT(BlueprintType)
struct FPlayerMovementStateChangeInfo
{
	GENERATED_BODY()

public:
	//変更前の状態
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	EPlayerMovementState PreviousState = EPlayerMovementState::Normal;

	//変更後の状態
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	EPlayerMovementState NewState = EPlayerMovementState::Normal;
};
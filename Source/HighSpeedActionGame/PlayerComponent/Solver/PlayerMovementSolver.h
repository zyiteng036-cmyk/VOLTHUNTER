//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー移動ソルバー
//
// 移動、ダッシュ、ジャンプ、回避に必要な速度計算だけを担当する
// Componentの状態管理やGameplayAbilityの発動判断は行わない
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"

class UPlayerMovementParameter;

class HIGHSPEEDACTIONGAME_API FPlayerMovementSolver
{
public:
	//入力値からワールド方向を計算
	static FVector CalculateInputWorldDirection(
		const FVector2D& InputVector,
		const FRotator& ControlRotation
	);

	//ダッシュ開始時のLaunch速度を計算
	static FVector CalculateDashLaunchVelocity(
		const FVector& DashDirection,
		const UPlayerMovementParameter* Param
	);

	//ダッシュ減速中の速度を計算
	static FVector CalculateDashDecelVelocity(
		const FVector& CurrentVelocity,
		float DeltaTime,
		const UPlayerMovementParameter* Param
	);

	//ダッシュ終了判定
	static bool ShouldFinishDashDecel(
		const FVector& CurrentVelocity,
		const UPlayerMovementParameter* Param
	);

	//ジャンプ開始時の速度を計算
	static FVector CalculateJumpVelocity(
		const FVector& CurrentVelocity,
		const UPlayerMovementParameter* Param
	);

	//短押しジャンプ時の速度を計算
	static FVector CalculateJumpReleaseVelocity(
		const FVector& CurrentVelocity,
		float HoldTime,
		const UPlayerMovementParameter* Param
	);

	//回避方向を計算
	static FVector CalculateEvasiveDirection(
		const FVector2D& InputVector,
		const FRotator& ControlRotation,
		const FVector& ActorForward,
		const UPlayerMovementParameter* Param
	);

	//回避開始時のLaunch速度を計算
	static FVector CalculateEvasiveLaunchVelocity(
		const FVector& EvasiveDirection,
		const UPlayerMovementParameter* Param
	);

	//状況に応じた重力倍率を計算
	static float CalculateGravityScale(
		const FVector& CurrentVelocity,
		bool bIsGrounded,
		const UPlayerMovementParameter* Param
	);
};

#include "PlayerMovementSolver.h"
#include "../../DataAssets/Player/PlayerMovementParameter.h"

//入力値からワールド方向を計算
FVector FPlayerMovementSolver::CalculateInputWorldDirection(
	const FVector2D& InputVector,
	const FRotator& ControlRotation
)
{
	FRotator FlatRotation = ControlRotation;
	FlatRotation.Pitch = 0.f;
	FlatRotation.Roll = 0.f;

	const FVector Forward = FRotationMatrix(FlatRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(FlatRotation).GetUnitAxis(EAxis::Y);

	//InputVector.X = 左右
	//InputVector.Y = 前後
	const FVector Direction =
		Forward * InputVector.Y +
		Right * InputVector.X;

	return Direction.GetSafeNormal2D();
}
//ダッシュ開始時のLaunch速度を計算
FVector FPlayerMovementSolver::CalculateDashLaunchVelocity(
	const FVector& DashDirection,
	const UPlayerMovementParameter* Param
)
{
	if (!Param) return FVector::ZeroVector;

	const FVector Direction = DashDirection.IsNearlyZero()
		? FVector::ForwardVector
		: DashDirection.GetSafeNormal2D();

	return Direction * Param->StartDashSpeed;
}

//ダッシュ減速中の速度を計算
FVector FPlayerMovementSolver::CalculateDashDecelVelocity(
	const FVector& CurrentVelocity,
	float DeltaTime,
	const UPlayerMovementParameter* Param
)
{
	if (!Param) return CurrentVelocity;

	return FMath::VInterpTo(
		CurrentVelocity,
		FVector::ZeroVector,
		DeltaTime,
		Param->DashInertiaInterpSpeed
	);
}

//ダッシュ終了判定
bool FPlayerMovementSolver::ShouldFinishDashDecel(
	const FVector& CurrentVelocity,
	const UPlayerMovementParameter* Param
)
{
	if (!Param) return true;

	const FVector HorizontalVelocity =
		FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.f);

	return HorizontalVelocity.SizeSquared() < Param->DashEndVelocitySqThreshold;
}


//ジャンプ開始時の速度を計算
FVector FPlayerMovementSolver::CalculateJumpVelocity(
	const FVector& CurrentVelocity,
	const UPlayerMovementParameter* Param
)
{
	if (!Param) return CurrentVelocity;

	FVector NewVelocity = CurrentVelocity;
	NewVelocity.Z = Param->LongJumpZVelocity;

	return NewVelocity;
}

//短押しジャンプ時の速度を計算
FVector FPlayerMovementSolver::CalculateJumpReleaseVelocity(
	const FVector& CurrentVelocity,
	float HoldTime,
	const UPlayerMovementParameter* Param
)
{
	//短押し・長押しによるジャンプ高度差は使わない
	return CurrentVelocity;
}

//回避方向を計算
FVector FPlayerMovementSolver::CalculateEvasiveDirection(
	const FVector2D& InputVector,
	const FRotator& ControlRotation,
	const FVector& ActorForward,
	const UPlayerMovementParameter* Param
)
{
	if (!Param)
	{
		return -ActorForward.GetSafeNormal2D();
	}

	if (InputVector.SizeSquared() >= Param->InputDeadZoneSq)
	{
		const FVector InputDirection = CalculateInputWorldDirection(InputVector, ControlRotation);
		if (!InputDirection.IsNearlyZero())
		{
			return InputDirection.GetSafeNormal2D();
		}
	}

	return -ActorForward.GetSafeNormal2D();
}

//回避開始時のLaunch速度を計算
FVector FPlayerMovementSolver::CalculateEvasiveLaunchVelocity(
	const FVector& EvasiveDirection,
	const UPlayerMovementParameter* Param
)
{
	if (!Param) return FVector::ZeroVector;

	FVector Direction = EvasiveDirection;
	Direction.Z = 0.f;

	if (!Direction.Normalize())
	{
		Direction = -FVector::ForwardVector;
	}

	FVector LaunchVelocity = Direction * Param->EvasiveSpeed;
	LaunchVelocity.Z += Param->EvasiveVerticalSpeed;

	return LaunchVelocity;
}

//状況に応じた重力倍率を計算
float FPlayerMovementSolver::CalculateGravityScale(
	const FVector& CurrentVelocity,
	bool bIsGrounded,
	const UPlayerMovementParameter* Param
)
{
	if (!Param) return 1.f;

	if (bIsGrounded)
	{
		return Param->GravityScale;
	}

	if (CurrentVelocity.Z < 0.f)
	{
		return Param->GravityScaleFall;
	}

	return Param->GravityScale;
}
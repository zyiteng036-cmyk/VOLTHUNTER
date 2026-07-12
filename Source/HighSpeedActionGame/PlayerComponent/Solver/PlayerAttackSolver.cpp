#include "PlayerAttackSolver.h"
#include "../../DataAssets/Player/PlayerAttackParameter.h"
#include "../../Enemy/EnemyBase.h"

//敵が攻撃対象として有効か
bool FPlayerAttackSolver::IsValidAttackEnemy(const AEnemyBase* Enemy)
{
	if (!Enemy) return false;
	if (Enemy->GetIsDying()) return false;
	if (!Enemy->GetIsActive()) return false;

	return true;
}

//入力方向と敵方向が許容角度内か
bool FPlayerAttackSolver::IsEnemyInInputDirection(
	const FVector& PlayerLocation,
	const FVector& MoveDirection,
	bool bHasMoveInput,
	const AEnemyBase* Enemy,
	const UPlayerAttackParameter* Param
)
{
	if (!Param) return false;
	if (!IsValidAttackEnemy(Enemy)) return false;

	const FVector EnemyLocation = Enemy->GetActorLocation();

	FVector ToEnemy = EnemyLocation - PlayerLocation;
	ToEnemy.Z = 0.f;

	const float Distance = ToEnemy.Size();
	if (Distance > Param->AttackRange) return false;

	if (!bHasMoveInput)
	{
		return true;
	}

	if (ToEnemy.IsNearlyZero()) return true;

	FVector FlatMoveDirection = MoveDirection;
	FlatMoveDirection.Z = 0.f;

	if (!FlatMoveDirection.Normalize())
	{
		return true;
	}

	const FVector ToEnemyDirection = ToEnemy.GetSafeNormal();
	const float Dot = FVector::DotProduct(FlatMoveDirection, ToEnemyDirection);

	const float DirectionThreshold =
		FMath::Cos(FMath::DegreesToRadians(Param->AttackInputAngle));

	return Dot >= DirectionThreshold;
}

//敵への踏み込み位置を計算
FPlayerAttackStepResult FPlayerAttackSolver::CalculateStepTargetToEnemy(
	const FVector& PlayerLocation,
	AEnemyBase* Enemy
)
{
	FPlayerAttackStepResult Result;

	if (!IsValidAttackEnemy(Enemy))
	{
		return Result;
	}

	const FVector EnemyLocation = Enemy->GetActorLocation();

	FVector DirectionToEnemy = EnemyLocation - PlayerLocation;
	DirectionToEnemy.Z = 0.f;

	if (!DirectionToEnemy.Normalize())
	{
		return Result;
	}

	const float WarpDistance = Enemy->GetWarpOffsetDistance();

	FVector TargetLocation = EnemyLocation - DirectionToEnemy * WarpDistance;
	TargetLocation.Z = PlayerLocation.Z;

	Result.bHasTargetLocation = true;
	Result.TargetLocation = TargetLocation;
	Result.LookDirection = DirectionToEnemy;
	Result.bHasEnemyTarget = true;
	Result.TargetActor = Enemy;

	return Result;
}

//敵がいない時の前方踏み込み位置を計算
FPlayerAttackStepResult FPlayerAttackSolver::CalculateForwardStepTarget(
	const FVector& PlayerLocation,
	const FVector& MoveDirection,
	const FVector& ActorForward,
	bool bHasMoveInput,
	const UPlayerAttackParameter* Param
)
{
	FPlayerAttackStepResult Result;

	if (!Param) return Result;

	FVector Forward = FVector::ZeroVector;

	if (bHasMoveInput && !MoveDirection.IsNearlyZero())
	{
		Forward = MoveDirection;
	}
	else
	{
		Forward = ActorForward;
	}

	Forward.Z = 0.f;

	if (!Forward.Normalize())
	{
		Forward = FVector::ForwardVector;
	}

	FVector TargetLocation = PlayerLocation + Forward * Param->AttackEnemyNothing;
	TargetLocation.Z = PlayerLocation.Z;

	Result.bHasTargetLocation = true;
	Result.TargetLocation = TargetLocation;
	Result.LookDirection = Forward;
	Result.bHasEnemyTarget = false;
	Result.TargetActor = nullptr;

	return Result;
}

//ジャスト回避攻撃のワープ位置を計算
FPlayerAttackStepResult FPlayerAttackSolver::CalculateJustEvasiveWarpTarget(
	const FVector& PlayerLocation,
	AEnemyBase* Enemy
)
{
	FPlayerAttackStepResult Result;

	if (!IsValidAttackEnemy(Enemy))
	{
		return Result;
	}

	const FVector EnemyLocation = Enemy->GetActorLocation();

	FVector DirectionToEnemy = EnemyLocation - PlayerLocation;
	DirectionToEnemy.Z = 0.f;

	if (!DirectionToEnemy.Normalize())
	{
		return Result;
	}

	const float WarpDistance = Enemy->GetWarpOffsetDistance();

	FVector TargetLocation = EnemyLocation - DirectionToEnemy * WarpDistance;
	TargetLocation.Z = PlayerLocation.Z;

	Result.bHasTargetLocation = true;
	Result.TargetLocation = TargetLocation;
	Result.LookDirection = DirectionToEnemy;
	Result.bHasEnemyTarget = true;
	Result.TargetActor = Enemy;

	return Result;
}

//空中弱攻撃の目標位置を計算
FPlayerAttackStepResult FPlayerAttackSolver::CalculateAirDashTarget(
	const FVector& PlayerLocation,
	const FVector& MoveDirection,
	const FVector& ActorForward,
	bool bHasMoveInput,
	AEnemyBase* Enemy,
	const UPlayerAttackParameter* Param
)
{
	FPlayerAttackStepResult Result;

	if (!Param) return Result;

	FVector BaseForward = FVector::ZeroVector;

	if (bHasMoveInput && !MoveDirection.IsNearlyZero())
	{
		BaseForward = MoveDirection;
	}
	else
	{
		BaseForward = ActorForward;
	}

	BaseForward.Z = 0.f;

	if (!BaseForward.Normalize())
	{
		BaseForward = FVector::ForwardVector;
	}

	if (IsValidAttackEnemy(Enemy))
	{
		const FVector EnemyLocation = Enemy->GetActorLocation();

		FVector EnemyForward =
			FRotationMatrix(
				FRotator(0.f, Enemy->GetActorRotation().Yaw, 0.f)
			).GetUnitAxis(EAxis::X);

		EnemyForward.Z = 0.f;

		if (!EnemyForward.Normalize())
		{
			EnemyForward = FVector::ForwardVector;
		}

		FVector TargetLocation = EnemyLocation - EnemyForward * Param->AttackEnemyBack;
		TargetLocation.Z = EnemyLocation.Z;

		FVector LookDirection = EnemyLocation - PlayerLocation;
		LookDirection.Z = 0.f;

		if (!LookDirection.Normalize())
		{
			LookDirection = BaseForward;
		}

		Result.bHasTargetLocation = true;
		Result.TargetLocation = TargetLocation;
		Result.LookDirection = LookDirection;
		Result.bHasEnemyTarget = true;
		Result.TargetActor = Enemy;

		return Result;
	}

	const float DownRad = FMath::DegreesToRadians(Param->AirDashDownAngle);

	const FVector DashDirection =
		BaseForward * FMath::Cos(DownRad) +
		FVector::DownVector * FMath::Sin(DownRad);

	const FVector TargetLocation =
		PlayerLocation + DashDirection.GetSafeNormal() * Param->AirDashDistance;

	Result.bHasTargetLocation = true;
	Result.TargetLocation = TargetLocation;
	Result.LookDirection = BaseForward;
	Result.bHasEnemyTarget = false;
	Result.TargetActor = nullptr;

	return Result;
}

//踏み込み中の次の位置を計算
FVector FPlayerAttackSolver::CalculateStepMoveLocation(
	const FVector& CurrentLocation,
	const FVector& TargetLocation,
	float StepSpeed,
	float DeltaTime,
	float StopDistance,
	bool bIgnoreZ
)
{
	FVector ToTarget = TargetLocation - CurrentLocation;

	if (bIgnoreZ)
	{
		ToTarget.Z = 0.f;
	}

	const float Distance = ToTarget.Size();

	if (Distance <= StopDistance)
	{
		return TargetLocation;
	}

	if (ToTarget.IsNearlyZero())
	{
		return CurrentLocation;
	}

	const FVector MoveDirection = ToTarget.GetSafeNormal();
	const float MoveStep = StepSpeed * DeltaTime;

	if (MoveStep >= Distance)
	{
		return TargetLocation;
	}

	return CurrentLocation + MoveDirection * MoveStep;
}

//空中攻撃が解禁される高さに達しているか
bool FPlayerAttackSolver::CanUnlockAirAttack(
	float JumpStartZ,
	float CurrentZ,
	const UPlayerAttackParameter* Param
)
{
	if (!Param) return false;

	const float DeltaZ = CurrentZ - JumpStartZ;

	return DeltaZ >= Param->AirAttackMinHeight;
}

//強攻撃溜め時間を更新
float FPlayerAttackSolver::UpdateHeavyChargeTime(
	float CurrentChargeTime,
	float DeltaTime,
	const UPlayerAttackParameter* Param
)
{
	if (!Param) return CurrentChargeTime;

	return FMath::Min(
		CurrentChargeTime + DeltaTime,
		Param->HeavyChargeMaxTime
	);
}

//空中強攻撃の溜めが終わったか
bool FPlayerAttackSolver::ShouldFinishAirFallCharge(
	float CurrentChargeTime,
	const UPlayerAttackParameter* Param
)
{
	if (!Param) return true;

	return CurrentChargeTime >= Param->AirFallChargeDuration;
}
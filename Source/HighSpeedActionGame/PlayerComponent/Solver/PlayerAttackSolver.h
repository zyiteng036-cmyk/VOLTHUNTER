//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー攻撃ソルバー
//
// 攻撃の踏み込み先、敵吸着位置、空中攻撃位置、
// ジャスト回避攻撃のワープ位置などの計算だけを担当する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../Types/PlayerAttackTypes.h"

class AEnemyBase;
class UPlayerAttackParameter;

class HIGHSPEEDACTIONGAME_API FPlayerAttackSolver
{
public:
	//敵が攻撃対象として有効か
	static bool IsValidAttackEnemy(const AEnemyBase* Enemy);

	//入力方向と敵方向が許容角度内か
	static bool IsEnemyInInputDirection(
		const FVector& PlayerLocation,
		const FVector& MoveDirection,
		bool bHasMoveInput,
		const AEnemyBase* Enemy,
		const UPlayerAttackParameter* Param
	);

	//敵への踏み込み位置を計算
	static FPlayerAttackStepResult CalculateStepTargetToEnemy(
		const FVector& PlayerLocation,
		AEnemyBase* Enemy
	);

	//敵がいない時の前方踏み込み位置を計算
	static FPlayerAttackStepResult CalculateForwardStepTarget(
		const FVector& PlayerLocation,
		const FVector& MoveDirection,
		const FVector& ActorForward,
		bool bHasMoveInput,
		const UPlayerAttackParameter* Param
	);

	//ジャスト回避攻撃のワープ位置を計算
	static FPlayerAttackStepResult CalculateJustEvasiveWarpTarget(
		const FVector& PlayerLocation,
		AEnemyBase* Enemy
	);

	//空中弱攻撃の目標位置を計算
	static FPlayerAttackStepResult CalculateAirDashTarget(
		const FVector& PlayerLocation,
		const FVector& MoveDirection,
		const FVector& ActorForward,
		bool bHasMoveInput,
		AEnemyBase* Enemy,
		const UPlayerAttackParameter* Param
	);

	//踏み込み中の次の位置を計算
	static FVector CalculateStepMoveLocation(
		const FVector& CurrentLocation,
		const FVector& TargetLocation,
		float StepSpeed,
		float DeltaTime,
		float StopDistance,
		bool bIgnoreZ
	);

	//空中攻撃が解禁される高さに達しているか
	static bool CanUnlockAirAttack(
		float JumpStartZ,
		float CurrentZ,
		const UPlayerAttackParameter* Param
	);

	//強攻撃溜め時間を更新
	static float UpdateHeavyChargeTime(
		float CurrentChargeTime,
		float DeltaTime,
		const UPlayerAttackParameter* Param
	);

	//空中強攻撃の溜めが終わったか
	static bool ShouldFinishAirFallCharge(
		float CurrentChargeTime,
		const UPlayerAttackParameter* Param
	);
};
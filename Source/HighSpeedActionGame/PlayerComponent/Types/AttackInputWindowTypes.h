//担当
//伊藤直樹

//-----------------------------------------------------
//攻撃入力受付窓の種類
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "AttackInputWindowTypes.generated.h"

UENUM(BlueprintType)
enum class EAttackInputWindowMode : uint8
{
	//通常コンボ用
	Combo,

	//ジャスト回避カウンター用
	JustEvasiveCounter
};
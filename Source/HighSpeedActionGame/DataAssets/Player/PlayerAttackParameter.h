//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー攻撃パラメータ
//
//攻撃対象、踏み込み、強攻撃、空中攻撃の調整値を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerAttackParameter.generated.h"

UCLASS(BlueprintType)
class HIGHSPEEDACTIONGAME_API UPlayerAttackParameter : public UDataAsset
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------
	//攻撃対象
	//-----------------------------------------------------

	//攻撃対象を検索する最大距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Target")
	float AttackRange = 900.f;

	//入力方向と敵方向の許容角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Target")
	float AttackInputAngle = 60.f;

	//-----------------------------------------------------
	//地上攻撃踏み込み
	//-----------------------------------------------------

	//敵がいない時の踏み込み距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Step")
	float AttackEnemyNothing = 350.f;

	//敵の背後へ回り込む距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Step")
	float AttackEnemyBack = 250.f;

	//地上攻撃の踏み込み速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Step")
	float AttackSpeed = 1800.f;

	//踏み込み完了と判定する距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Step")
	float AttackStepStopDistance = 5.f;

	//-----------------------------------------------------
	//地上強攻撃
	//-----------------------------------------------------

	//最大溜め状態になるまでの時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Heavy")
	float HeavyChargeMaxTime = 1.2f;

	//-----------------------------------------------------
	//空中攻撃
	//-----------------------------------------------------

	//空中攻撃解禁に必要な上昇量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Air")
	float AirAttackMinHeight = 120.f;

	//空中攻撃の踏み込み速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Air")
	float AirAttackSpeed = 2600.f;

	//空中弱攻撃の移動距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Air")
	float AirDashDistance = 700.f;

	//空中弱攻撃の下方向角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Air")
	float AirDashDownAngle = 15.f;

	//-----------------------------------------------------
	//空中強攻撃
	//-----------------------------------------------------

	//叩き落とし開始までの溜め時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Air|Fall")
	float AirFallChargeDuration = 0.25f;

	//叩き落とし中の落下速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Air|Fall")
	float AirFallSpeed = 2500.f;

	//叩き落とし開始時の上方向速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Air|Fall")
	float AirFallRiseSpeed = 400.f;
};
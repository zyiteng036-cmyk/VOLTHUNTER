//担当
//伊藤直樹

#pragma once

#include "CoreMinimal.h"
#include "PlayerParam.generated.h"

/**
 *
 */
 //UE側で操作できるパラメータ
USTRUCT(BlueprintType)
struct FPlayerParam
{
	GENERATED_BODY()

public:

	//プレイヤーマックスHP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxHP = 100.f;

	//走る速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (Units = "cm/s", ToolTip = "走る速度"))
	float RunSpeed = 950.f;


	//重力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ToolTip = "重力"))
	float GravityScale = 3.f;

	//落下時の重力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ToolTip = "重力"))
	float GravityScaleFall = 6.f;


	//空中操作
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ToolTip = "空中操作"))
	float AirControl = 0.5f;

	//ダッシュ中の空中操作
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ToolTip = "ダッシュ中の空中操作"))
	float AirControlDash = 1.f;


	//短い入力のジャンプの高さ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (Units = "cm", ToolTip = "短い入力のジャンプの高さ"))
	float ShortJumpZVelocity = 700.f;

	//長い入力のジャンプの高さ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (Units = "cm", ToolTip = "長い入力のジャンプの高さ"))
	float LongJumpZVelocity = 1300.f;

	// 長押し判定時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (Units = "s", ToolTip = "長押し判定時間"))
	float LongJumpThreshold = 0.15f;


	//攻撃--------------------------------------------------------
	//攻撃の間合い
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "cm", ToolTip = "攻撃の間合い"))
	float AttackRange = 900.f;

	//敵がいない時にどれぐらい進むか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "cm", ToolTip = "敵がいない時にどれぐらい進むか"))
	float AttackEnemyNothing = 200.f;

	//敵のどれぐらい手前に行くか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "cm", ToolTip = "敵のどれぐらい手前に行くか"))
	float AttackEnemyFront = 200.f;

	//初撃で進む速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "cm/s", ToolTip = "初撃で進む速度"))
	float AttackSpeed = 2500.f;

	//入力があるときに敵を見つける角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "deg", ToolTip = "入力があるときに敵を見つける角度"))
	float AttackInputAngle = 90.f;

	//弱空中攻撃ご敵の背後に行く
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "cm", ToolTip = "弱空中攻撃ご敵の背後に行く"))
	float AttackEnemyBack = 250.f;

	//弱空中の敵がいなかった際のダッシュ斬りの進む距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "cm", ToolTip = "弱空中の敵がいなかった際のダッシュ斬りの進む距離"))
	float AirDashDistance = 1200.f;

	//弱空中の敵がいなかった際の斜めの角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "deg", ToolTip = "弱空中攻撃ご敵の背後に行く"))
	float AirDashDownAngle = 15.f;

	//強攻撃のため時間/ここで短い、長いの攻撃が変わる
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "s", ToolTip = "強攻撃のため時間/ここで短い、長いの攻撃が変わる"))
	float HeavyChargeLong = 1.f;

	//強攻撃の最大のため時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "s", ToolTip = "強攻撃の最大のため時間"))
	float HeavyChargeMaxTime = 5.f;

	//空中攻撃で進む速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "cm/s", ToolTip = "空中攻撃で進む速度"))
	float AirAttackSpeed = 7000.f;

	//空中攻撃強の溜め時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (Units = "s", ToolTip = "空中攻撃強の溜め時間"))
	float AirFallChargeDuration = 0.5f;

	//空中攻撃強の叩きつけ速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (Units = "cm/s", ToolTip = "空中攻撃強の叩きつけ速度"))
	float AirFallSpeed = 4000.f;

	// 空中攻撃が解禁される最低上昇量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (Units = "cm", ToolTip = "空中攻撃が解禁される最低上昇量"))
	float AirAttackMinHeight = 90.f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (Units = "cm", ToolTip = "敵情報の解除を行う距離"))
	float JustEvasiveTargetKeepDistance = 3000.f;

	//カメラ--------------------------------------------------------
	//カメラ距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (Units = "cm", ToolTip = "カメラ距離"))
	float CameraLength = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector CameraSocketOffset = FVector(0.f, 0.f, 70.f);

	//カメラの感度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ToolTip = "カメラの感度"))
	float CameraSensitivity = 0.9f;

	//カメララグ速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ToolTip = "カメララグ速度 [1/s]"))
	float CameraLagSpeed = 6.f;

	//カメラの離れる距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (Units = "cm", ToolTip = "カメラの離れる距離"))
	float CameraLagMaxDistance = 100.f;

	//ダッシュ時のカメララグ速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ToolTip = "ダッシュ時のカメララグ速度 [1/s]"))
	float Dash_CameraLagSpeed = 3.f;

	//ダッシュ時のカメラの離れる距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (Units = "cm", ToolTip = "ダッシュ時のカメラの離れる距離"))
	float Dash_CameraLagMaxDistance = 150.f;

	//回転ラグ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ToolTip = "回転ラグ [1/s]"))
	float CameraRotateLagSpeed = 12.f;

	//ダッシュ直後のカメラが寄る距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (Units = "cm", ToolTip = "ダッシュ直後のカメラ距離"))
	float DashStartArmLength = 50.f;

	//中盤で寄せる
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (Units = "cm", ToolTip = "中盤で寄せる距離"))
	float DashMidArmLength = 250.f;

	//初速時のFOV
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (Units = "deg", ToolTip = "初速時のFOV"))
	float DashStartFOV = 90.f;

	//中盤のFOV
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (Units = "deg", ToolTip = "中盤のFOV"))
	float DashMidFOV = 100.f;

	//デフォルトのFOV
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (Units = "deg", ToolTip = "デフォルトのFOV"))
	float DefaultFOV = 95.f;



	//ダッシュ--------------------------------------------------------
	//ダッシュ速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (Units = "cm/s", ToolTip = "ダッシュ速度"))
	float DashSpeed = 2000.f;

	//ダッシュ開始時の速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (Units = "cm/s", ToolTip = "ダッシュ開始時の速度"))
	float StartDashSpeed = 2500.f;


	//回避--------------------------------------------------------
	// 回避ストック
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive", meta = (ToolTip = "回避ストック"))
	int32 EvasiveStock = 10;

	//１ストック回復に必要な時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive", meta = (Units = "s", ToolTip = "1ストック回復に必要な時間"))
	float EvasiveRecoveryTime = 3.0f;
	//回避速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive", meta = (Units = "cm/s", ToolTip = "回避速度"))
	float EvasiveSpeed = 2000.f;

	//回避速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive", meta = (Units = "cm/s", ToolTip = "空中回避速度"))
	float AirStepSpeed = 100.f;

	//回避時の上方向への加速
	//回避中少し浮く
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive", meta = (Units = "cm/s", ToolTip = "回避時の上方向への加速"))
	float VerticalSpeed = 200.f;

	//回避終了時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive", meta = (Units = "s", ToolTip = "回避終了時間"))
	float FinishEvasiveTime = 0.4;

};

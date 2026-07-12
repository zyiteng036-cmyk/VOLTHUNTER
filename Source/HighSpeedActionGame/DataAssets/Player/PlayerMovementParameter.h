//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー移動パラメータ
//
//通常移動、ダッシュ、ジャンプ、回避の調整値を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerMovementParameter.generated.h"

UCLASS(BlueprintType)
class HIGHSPEEDACTIONGAME_API UPlayerMovementParameter : public UDataAsset
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------
	//通常移動
	//-----------------------------------------------------

	//通常移動速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float RunSpeed = 950.f;

	//移動時の加速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float Acceleration = 2500.f;

	//移動中の摩擦
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float Friction = 8.f;

	//入力なし時の摩擦
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float StopFriction = 20.f;

	//アイドル判定に使用する速度の二乗値
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float IdleVelocitySqThreshold = 30.f;

	//-----------------------------------------------------
	//移動入力
	//-----------------------------------------------------

	//入力あり判定に使用する入力値の二乗長
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float InputDeadZoneSq = 0.1f;

	//移動入力を毎フレーム減衰させる倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float MoveInputDecayRate = 0.5f;

	//-----------------------------------------------------
	//移動回転
	//-----------------------------------------------------

	//移動時の旋回速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Rotation")
	float MoveRotationRateYaw = 1300.f;

	//真後ろ反転補正を行う角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Rotation")
	float ReverseRotationThreshold = 178.f;

	//真後ろ反転補正時にずらす角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Rotation")
	float ReverseRotationOffset = 179.f;

	//-----------------------------------------------------
	//方向反転
	//-----------------------------------------------------

	//方向反転を判定する最低速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Reverse")
	float ReverseCheckMinSpeed = 50.f;

	//急旋回と判定する方向内積
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Reverse")
	float SharpTurnDotThreshold = -0.3f;

	//-----------------------------------------------------
	//ダッシュ
	//-----------------------------------------------------

	//ダッシュ開始速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float StartDashSpeed = 1600.f;

	//ダッシュ最高速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashSpeed = 1800.f;

	//ダッシュ中の加速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashAcceleration = 8000.f;

	//ダッシュ中の摩擦
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashFriction = 1.5f;

	//ダッシュ速度へ近づける補間速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashSpeedInterpSpeed = 2.f;

	//ダッシュ中の空中操作量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float AirControlDash = 0.1f;

	//移動入力がなくなってから減速へ移るまでの時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0.0", Units = "s"))
	float DashNoInputEndDelay = 0.5f;

	//-----------------------------------------------------
	//ダッシュ減速
	//-----------------------------------------------------

	//ダッシュ減速時のブレーキ力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Decel")
	float DashDecelBraking = 500.f;

	//ダッシュ減速時の摩擦
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Decel")
	float DashDecelFriction = 2.f;

	//ダッシュ終了時の慣性補間速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Decel")
	float DashInertiaInterpSpeed = 3.f;

	//ダッシュ終了判定に使用する速度の二乗値
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Decel")
	float DashEndVelocitySqThreshold = 900.f;

	//-----------------------------------------------------
	//ジャンプ
	//-----------------------------------------------------

	//通常重力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float GravityScale = 3.f;

	//落下中の重力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float GravityScaleFall = 6.f;

	//空中操作量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float AirControl = 0.4f;

	//長押しジャンプ初速
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float LongJumpZVelocity = 1200.f;

	//短押しジャンプの最低保証速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float ShortJumpZVelocity = 450.f;

	//長押しジャンプとして扱う時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float LongJumpThreshold = 0.2f;

	//短押し時の上昇速度倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float JumpShortReleaseMultiplier = 0.35f;

	//着地後のジャンプクールタイム
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float JumpCooldownTime = 0.2f;

	//-----------------------------------------------------
	//回避
	//-----------------------------------------------------

	//回避速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive")
	float EvasiveSpeed = 1600.f;

	//回避開始時の上方向速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive")
	float EvasiveVerticalSpeed = 150.f;

	//回避状態の継続時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive")
	float EvasiveDuration = 0.45f;

	//回避後の硬直時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive")
	float EvasiveRecoveryTime = 0.12f;

	//-----------------------------------------------------
	//ジャスト回避
	//-----------------------------------------------------

	//ジャスト回避の有効時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive|Just")
	float JustEvasiveLimitTime = 1.5f;

	//ジャスト回避成功時の時間倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive|Just")
	float JustEvasiveSlowTimeScale = 0.2f;

	//ジャスト回避成功時のスロー時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive|Just")
	float JustEvasiveSlowDuration = 0.2f;

	//スロー状態を強制解除する安全時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive|Just")
	float SlowMotionSafetyLimit = 3.f;

	//ジャスト回避成功時の電力ゲージ加算量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evasive|Just")
	float JustEvasiveGaugeAddValue = 15.f;

	//-----------------------------------------------------
	//物理更新
	//-----------------------------------------------------

	//固定サブステップ時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float FixedSubstepDeltaTime = 1.f / 120.f;

	//サブステップ最大回数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	int32 MaxPhysicsSubsteps = 4;

	//外部速度倍率の初期値
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float DefaultExternalSpeedMultiplier = 1.f;
};
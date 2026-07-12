//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤーカメラパラメータ
//
//通常カメラと各種カメラ演出の調整値を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerCameraParameter.generated.h"

class UUserWidget;

UCLASS(BlueprintType)
class HIGHSPEEDACTIONGAME_API UCameraParameter : public UDataAsset
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------
	//基本カメラ
	//-----------------------------------------------------

	//基本カメラ距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	float BaseArmLength = 300.f;

	//基本ソケットオフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	FVector BaseSocketOffset = FVector(0.f, 40.f, 0.f);

	//基本ターゲットオフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	FVector BaseTargetOffset = FVector::ZeroVector;

	//基本FOV
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	float BaseFOV = 90.f;

	//基本カメラ追従速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	float BaseLagSpeed = 10.f;

	//基本回転追従速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	float BaseRotationLagSpeed = 15.f;

	//-----------------------------------------------------
	//Pitch制限
	//-----------------------------------------------------

	//プレイヤー操作時の下方向制限
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pitch")
	float PitchMin = -60.f;

	//プレイヤー操作時の上方向制限
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pitch")
	float PitchMax = 70.f;

	//レイヤー加算後の安全下限
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pitch|Safe")
	float SafePitchMin = -85.f;

	//レイヤー加算後の安全上限
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pitch|Safe")
	float SafePitchMax = 85.f;

	//-----------------------------------------------------
	//移動先読み
	//-----------------------------------------------------

	//前方向速度を先読みに反映する割合
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ForwardLeadFraction = 0.35f;

	//前進時の先読み距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead|Distance")
	float LeadDistForward = 150.f;

	//後退時の先読み距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead|Distance")
	float LeadDistBackward = 100.f;

	//右移動時の先読み距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead|Distance")
	float LeadDistRight = 60.f;

	//左移動時の先読み距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead|Distance")
	float LeadDistLeft = 150.f;

	//先読み量が最大になる移動速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead", meta = (ClampMin = "1.0"))
	float VelocityLeadSpeedRef = 600.f;

	//回転先読み倍率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead")
	float RotationLeadScale = 3.f;

	//1フレームに加算する回転先読み制限
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead")
	float RotationLeadYawClamp = 5.f;

	//先読みのブレンド速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead")
	float LeadBlendSpeed = 12.f;

	//先読み位置の補間速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead")
	float LeadInterpSpeed = 12.f;

	//移動入力がない時に速度先読みを停止するか
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead")
	bool bStopLeadWhenNoMoveInput = true;

	//移動入力ありと判定する加速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead", meta = (ClampMin = "0.0"))
	float LeadInputAccelerationThreshold = 1.f;

	//先読み解除時の基本復帰速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead", meta = (ClampMin = "0.0"))
	float LeadReturnConstantSpeed = 300.f;

	//先読み解除時の最低復帰速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead", meta = (ClampMin = "0.0"))
	float LeadReturnMinSpeed = 20.f;

	//復帰速度を減速し始める距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead", meta = (ClampMin = "1.0"))
	float LeadReturnSlowDistance = 80.f;

	//先読み位置をゼロに丸める許容値
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lead", meta = (ClampMin = "0.0"))
	float LeadReturnSnapTolerance = 0.1f;

	//-----------------------------------------------------
	//見下ろし
	//-----------------------------------------------------

	//入力停止後に見下ろしを始めるまでの時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookDown", meta = (ClampMin = "0.0"))
	float LookDownDelayTime = 0.4f;

	//最大見下ろし角度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookDown")
	float MaxLookDownPitchOffset = -15.f;

	//見下ろし量が最大になる落下速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookDown", meta = (ClampMin = "0.1"))
	float LookDownTargetZVelocity = -300.f;

	//見下ろし開始時の補間速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookDown", meta = (ClampMin = "0.1"))
	float LookDownBlendInSpeed = 2.f;

	//見下ろし解除時の補間速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookDown", meta = (ClampMin = "0.1"))
	float LookDownBlendOutSpeed = 5.f;

	//-----------------------------------------------------
	//SoftClamp
	//-----------------------------------------------------

	//通常時の画面中心オフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SoftClamp")
	FVector2D NormalScreenOffset = FVector2D::ZeroVector;

	//通常時のデッドゾーン
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SoftClamp")
	FVector2D NormalDeadzone = FVector2D(0.3f, 0.3f);

	//Boss戦時の画面中心オフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SoftClamp|Boss")
	FVector2D BossScreenOffset = FVector2D::ZeroVector;

	//Boss戦時のデッドゾーン
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SoftClamp|Boss")
	FVector2D BossDeadzone = FVector2D(0.4f, 0.4f);

	//画面外補正の補間速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SoftClamp")
	float SoftClampInterpSpeed = 12.f;

	//画面外補正の復帰速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SoftClamp")
	float SoftClampReturnSpeed = 10.f;

	//-----------------------------------------------------
	//Boss戦カメラ
	//-----------------------------------------------------

	//Boss近距離時のカメラ距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Distance")
	float BossNearArmLength = 320.f;

	//Boss遠距離時のカメラ距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Distance")
	float BossFarArmLength = 750.f;

	//近距離カメラとして扱う距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Distance")
	float BossNearDistance = 800.f;

	//遠距離カメラとして扱う距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Distance")
	float BossFarDistance = 3500.f;

	//Boss戦カメラ距離の補間速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Distance")
	float BossArmLengthInterpSpeed = 4.f;

	//Boss戦中のFOV
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle")
	float BossBattleFOV = 90.f;

	//Boss戦カメラへのブレンド速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle")
	float BossBattleBlendSpeed = 2.5f;

	//Bossを画面内に収める縦オフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Framing")
	float BossFramingHeightOffset = 120.f;

	//Bossとの距離を平滑化する速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Distance")
	float BossDistanceInterpSpeed = 5.f;

	//Boss距離の小さな変化を無視する範囲
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Distance")
	float BossDistanceDeadZone = 50.f;

	//カメラ距離の小さな変化を無視する範囲
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Distance")
	float BossArmLengthDeadZone = 10.f;

	//Boss戦中のカメラ追従速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle", meta = (ClampMin = "0.1"))
	float BossCameraLagSpeed = 8.f;

	//Bossカメラのデバッグログを出すか
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle|Debug")
	bool bEnableBossCameraDebugLog = false;

	//Boss戦FOVの補間速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossBattle", meta = (ClampMin = "0.1"))
	float BossFOVInterpSpeed = 4.f;

	//-----------------------------------------------------
	//ジャスト回避カメラ
	//-----------------------------------------------------

	//敵方向を見る補間速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JustEvasive")
	float JustEvasiveLookInterpSpeed = 10.f;

	//敵方向を見続ける時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JustEvasive")
	float JustEvasiveLookDuration = 0.3f;

	//敵方向を見る時のPitch
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JustEvasive")
	float JustEvasiveLookPitch = -10.f;

	//-----------------------------------------------------
	//スキルカメラ
	//-----------------------------------------------------

	//スキル開始時のカメラ距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SkillCamera")
	float SkillZoomInArmLength = 200.f;

	//スキル発動後のカメラ距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SkillCamera")
	float SkillZoomOutArmLength = 800.f;

	//スキル開始時の寄り速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SkillCamera")
	float SkillZoomInSpeed = 10.f;

	//スキル発動後の引き速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SkillCamera")
	float SkillZoomOutSpeed = 6.f;

	//スキルカメラ解除時の復帰速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SkillCamera")
	float SkillCameraReturnSpeed = 8.f;

	//-----------------------------------------------------
	//オープニングカメラ
	//-----------------------------------------------------

	//オープニング開始前の待機時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera")
	float OpeningWaitTime = 0.f;

	//オープニングカメラの移動時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera")
	float OpeningMoveDuration = 4.f;

	//オープニング開始時の顔との距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera")
	float OpeningFaceDistance = 100.f;

	//オープニング開始時のYawオフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera")
	float OpeningStartYawOffset = 180.f;

	//顔アップ時の縦オフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera")
	float OpeningFaceSocketOffsetZ = 70.f;

	//顔アップ時の横オフセット
	//正数で右、負数で左へ移動する
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera")
	float OpeningFaceSocketOffsetY = 80.f;

	//オープニング開始時のPitch
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera")
	float OpeningStartPitch = -10.f;

	//オープニング補間の指数
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera")
	float OpeningEaseExp = 4.f;

	//オープニング中に表示するUI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera|UI")
	TSubclassOf<UUserWidget> OpeningWidgetClass = nullptr;

	//UIを非表示にし始める時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera|UI")
	float OpeningUiHideDelayTime = 2.f;

	//UIのフェード時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpeningCamera|UI")
	float OpeningUiFadeDuration = 0.5f;

	//-----------------------------------------------------
	//クリアカメラ
	//-----------------------------------------------------

	//背面カメラのYaw加算値
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Back")
	float BackClearAddYaw = 330.f;

	//背面カメラのPitch
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Back")
	float BackClearPitch = -10.f;

	//背面カメラのソケットオフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Back")
	FVector BackClearSocketOffset = FVector(0.f, 0.f, -30.f);

	//背面カメラのアーム長
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Back")
	float BackClearTargetArmLength = 700.f;

	//側面カメラのYaw加算値
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Side")
	float SideClearAddYaw = 90.f;

	//側面カメラのPitch
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Side")
	float SideClearPitch = -10.f;

	//側面カメラのソケットオフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Side")
	FVector SideClearSocketOffset = FVector(-50.f, 0.f, 20.f);

	//側面カメラのアーム長
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Side")
	float SideClearTargetArmLength = 0.f;

	//正面カメラのYaw加算値
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Front")
	float FrontClearAddYaw = -1.f;

	//正面カメラのPitch
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Front")
	float FrontClearPitch = 10.f;

	//正面カメラのソケットオフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Front")
	FVector FrontClearSocketOffset = FVector(0.f, 0.f, -15.f);

	//正面カメラのアーム長
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Front")
	float FrontClearTargetArmLength = 0.f;

	//背面カメラへ切り替えるまでの時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Timing")
	float ClearCameraBackDelay = 1.f;

	//側面カメラへ切り替えるまでの時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Timing")
	float ClearCameraSideDelay = 3.f;

	//正面カメラへ切り替えるまでの時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Timing")
	float ClearCameraFrontDelay = 4.33f;

	//クリア演出中のズーム距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Zoom")
	float ClearZoomLength = 300.f;

	//クリア演出中のズーム補間速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClearCamera|Zoom")
	float ClearZoomInterpSpeed = 0.3f;

	//-----------------------------------------------------
	//死亡カメラ
	//-----------------------------------------------------

	//死亡時に追加で引く距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DieCamera")
	float DieZoomOutDistance = 250.f;

	//死亡時の見下ろし角度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DieCamera")
	float DieTargetPitch = -50.f;

	//-----------------------------------------------------
	//安全補正
	//-----------------------------------------------------

	//補間計算に使用する最大DeltaTime
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Safety")
	float MaxDeltaTimeThreshold = 0.02f;

	//フレーム時間急増時に使用する標準DeltaTime
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Safety")
	float StandardFrameTimeDelta = 0.0166f;

	//タイムスロー計算に使用する最小時間倍率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Safety")
	float MinTimeDilation = 0.0001f;

	//カメラ相対位置の復帰速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Safety")
	float CameraRecoveryInterpSpeed = 15.f;

	//相対位置をゼロと判定する許容値
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Safety")
	float RelativeLocationZeroTolerance = 0.1f;

	//-----------------------------------------------------
	//Spline移動カメラ
	//-----------------------------------------------------

	//Spline移動中のカメラ距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SplineMoveCamera")
	float SplineMoveArmLength = 700.f;

	//Spline移動中のFOV
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SplineMoveCamera")
	float SplineMoveFOV = 100.f;

	//Spline移動中のソケットオフセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SplineMoveCamera")
	FVector SplineMoveSocketOffset = FVector(0.f, 0.f, 80.f);

	//Spline移動カメラへ切り替える速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SplineMoveCamera")
	float SplineMoveBlendInSpeed = 5.f;

	//通常カメラへ戻る速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SplineMoveCamera")
	float SplineMoveBlendOutSpeed = 6.f;
};
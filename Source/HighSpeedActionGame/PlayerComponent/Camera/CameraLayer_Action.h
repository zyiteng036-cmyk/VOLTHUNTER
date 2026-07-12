//担当
//伊藤直樹

//-----------------------------------------------------
//アクションカメラレイヤー
//
// スキル、ジャスト回避後の敵注視、Spline移動など、
// プレイヤーの行動に反応する一時カメラを担当する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "CameraLayer.h"
#include "CameraLayer_Action.generated.h"

//スキルカメラの内部フェーズ
UENUM(BlueprintType)
enum class ECameraSkillPhase : uint8
{
	//なし
	None,

	//一瞬寄る
	ZoomIn,

	//大きく引く
	ZoomOut
};

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraLayer_Action : public UCameraLayer
{
	GENERATED_BODY()

public:
	//初期化
	UCameraLayer_Action();

public:
	//レイヤー処理を適用
	virtual void ApplyLayer(FCameraContext& Context) override;

	//レイヤー名を取得
	virtual FName GetLayerName() const override;

private:
	//-----------------------------------------------------
	// Action
	//-----------------------------------------------------

	//アクションカメラを更新
	void UpdateActionCamera(FCameraContext& Context);

	//アクションカメラ解除中の復帰処理
	void UpdateActionReturn(FCameraContext& Context);

	//現在の要求が指定タイプか
	bool IsRequestType(const FCameraContext& Context, ECameraRequestType RequestType) const;

private:
	//-----------------------------------------------------
	// Skill
	//-----------------------------------------------------

	//スキルカメラを更新
	void UpdateSkillCamera(FCameraContext& Context);

	//スキルカメラのフェーズを更新
	void UpdateSkillPhase(const FCameraContext& Context);

	//スキルカメラ距離を適用
	void ApplySkillCamera(FCameraContext& Context);

	//スキルカメラを解除方向へ戻す
	void ReturnSkillCamera(FCameraContext& Context);

private:
	//-----------------------------------------------------
	// JustEvasive
	//-----------------------------------------------------

	//ジャスト回避後の敵注視カメラを更新
	void UpdateJustEvasiveLookCamera(FCameraContext& Context);

	//敵方向への回転を計算
	FRotator CalculateLookAtTargetRotation(const FCameraContext& Context) const;

	//Controller回転を補間
	void InterpControllerRotation(const FCameraContext& Context, const FRotator& TargetRotation) const;

	//PlayerControllerを取得
	APlayerController* GetPlayerController(const FCameraContext& Context) const;

private:
	//-----------------------------------------------------
	// SplineMove
	//-----------------------------------------------------

	//Spline移動カメラを更新
	void UpdateSplineMoveCamera(FCameraContext& Context);

	//Spline移動カメラを解除方向へ戻す
	void ReturnSplineMoveCamera(FCameraContext& Context);

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//現在スキルカメラが影響している割合
	UPROPERTY(VisibleAnywhere, Category = "Camera|Action")
	float m_SkillCameraBlendAlpha = 0.f;

	//現在のスキルカメラ距離
	UPROPERTY(VisibleAnywhere, Category = "Camera|Action")
	float m_CurrentSkillArmLength = 0.f;

	//スキルカメラフェーズ
	UPROPERTY(VisibleAnywhere, Category = "Camera|Action")
	ECameraSkillPhase m_SkillPhase = ECameraSkillPhase::None;

	//前フレームにスキルカメラが有効だったか
	UPROPERTY(VisibleAnywhere, Category = "Camera|Action")
	bool m_bWasSkillCameraActive = false;

	//Spline移動カメラのブレンド率
	UPROPERTY(VisibleAnywhere, Category = "Camera|Action")
	float m_SplineMoveCameraBlendAlpha = 0.f;

	//前フレームにSpline移動カメラが有効だったか
	UPROPERTY(VisibleAnywhere, Category = "Camera|Action")
	bool m_bWasSplineMoveCameraActive = false;
};
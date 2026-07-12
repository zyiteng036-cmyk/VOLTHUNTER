//担当
//伊藤直樹

//-----------------------------------------------------
//落下時見下ろしカメラレイヤー
//
// 落下中かつプレイヤーがカメラ操作していない時だけ、
// カメラを少し下へ向けて着地点を見やすくする
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "CameraLayer.h"
#include "CameraLayer_LookDown.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraLayer_LookDown : public UCameraLayer
{
	GENERATED_BODY()

public:
	//初期化
	UCameraLayer_LookDown();

public:
	//レイヤー処理を適用
	virtual void ApplyLayer(FCameraContext& Context) override;

	//レイヤー名を取得
	virtual FName GetLayerName() const override;

private:
	//-----------------------------------------------------
	// 内部処理
	//-----------------------------------------------------

	//見下ろしを適用できるか
	bool CanApplyLookDown(const FCameraContext& Context) const;

	//目標Pitchを計算
	float CalculateTargetPitchOffset(const FCameraContext& Context) const;

	//見下ろしを通常状態へ戻す
	void ResetLookDown(const FCameraContext& Context);

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//現在のPitch補正値
	UPROPERTY(VisibleAnywhere, Category = "Camera|LookDown")
	float m_CurrentPitchOffset = 0.f;

	//見下ろし条件を満たしている経過時間
	UPROPERTY(VisibleAnywhere, Category = "Camera|LookDown")
	float m_LookDownConditionTime = 0.f;
};
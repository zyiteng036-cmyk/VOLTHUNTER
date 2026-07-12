//担当
//伊藤直樹

//-----------------------------------------------------
//カメラSoftClampレイヤー
//
//プレイヤーがデッドゾーン外へ出た時に画面内へ収める
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "CameraLayer.h"
#include "CameraLayer_SoftClamp.generated.h"

class UCameraSoftClampSolver;

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraLayer_SoftClamp : public UCameraLayer
{
	GENERATED_BODY()

public:
	//レイヤー処理を適用
	virtual void ApplyLayer(FCameraContext& Context) override;

	//レイヤー名を取得
	virtual FName GetLayerName() const override;

private:
	//SoftClampソルバーを初期化
	void InitializeSolver();

	//使用する画面中心オフセットを取得
	FVector2D GetScreenOffset(const FCameraContext& Context) const;

	//使用するデッドゾーンを取得
	FVector2D GetDeadzone(const FCameraContext& Context) const;

	//SoftClampを停止するカメラ要求か
	bool ShouldSkipForCameraRequest(const FCameraContext& Context) const;

private:
	//SoftClamp計算用ソルバー
	UPROPERTY()
	TObjectPtr<UCameraSoftClampSolver> m_SoftClampSolver = nullptr;
};
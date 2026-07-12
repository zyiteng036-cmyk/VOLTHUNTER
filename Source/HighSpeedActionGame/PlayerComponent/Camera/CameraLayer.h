//担当
//伊藤直樹

//-----------------------------------------------------
//カメラレイヤー基底クラス
//
// CameraRigComponentのパイプライン内で実行される
// 各レイヤーはFCameraContextを受け取り、ArmLength、Offset、FOV、Rotationを補正する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "../Types/CameraRigTypes.h"
#include "CameraLayer.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew)
class HIGHSPEEDACTIONGAME_API UCameraLayer : public UObject
{
	GENERATED_BODY()

public:
	//初期化
	UCameraLayer();

public:
	//-----------------------------------------------------
	// Layer
	//-----------------------------------------------------

	//レイヤー処理を適用
	virtual void ApplyLayer(FCameraContext& Context);

	//レイヤー有効状態を設定
	UFUNCTION(BlueprintCallable, Category = "Camera|Layer")
	void SetLayerActive(bool bActive);

	//レイヤー有効状態を取得
	UFUNCTION(BlueprintPure, Category = "Camera|Layer")
	bool IsLayerActive() const { return m_bLayerActive; }

	//レイヤー名を取得
	virtual FName GetLayerName() const;

protected:
	//レイヤー処理可能か
	bool CanApplyLayer(const FCameraContext& Context) const;

protected:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//レイヤーが有効か
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Layer")
	bool m_bLayerActive = true;
};
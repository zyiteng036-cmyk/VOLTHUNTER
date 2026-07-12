//担当
//伊藤直樹

//-----------------------------------------------------
//オープニングカメラレイヤー
//
// ゲーム開始時の顔アップ演出から通常カメラへ戻す
// ゲーム進行フェーズ専用のカメラを担当する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "CameraLayer.h"
#include "CameraLayer_OpeningCamera.generated.h"

class UUserWidget;
class APlayerController;

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraLayer_OpeningCamera : public UCameraLayer
{
	GENERATED_BODY()

public:
	//初期化
	UCameraLayer_OpeningCamera();

public:
	//レイヤー処理を適用
	virtual void ApplyLayer(FCameraContext& Context) override;

	//レイヤー名を取得
	virtual FName GetLayerName() const override;

private:
	//-----------------------------------------------------
	// OpeningCamera
	//-----------------------------------------------------

	//オープニングカメラを更新
	void UpdateOpeningCamera(FCameraContext& Context);

	//オープニングカメラを開始
	void StartOpeningCamera(FCameraContext& Context);

	//オープニングカメラを終了
	void StopOpeningCamera(FCameraContext& Context);

	//オープニングカメラ値を適用
	void ApplyOpeningCamera(FCameraContext& Context);

	//オープニングUIを更新
	void UpdateOpeningWidget(const FCameraContext& Context);

	//オープニングUIを生成
	void CreateOpeningWidget(const FCameraContext& Context);

	//オープニングUIを破棄
	void RemoveOpeningWidget();

	//PlayerControllerを取得
	APlayerController* GetPlayerController(const FCameraContext& Context) const;

	//オープニング要求か
	bool IsOpeningRequest(const FCameraContext& Context) const;

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//オープニングカメラが有効だったか
	UPROPERTY(VisibleAnywhere, Category = "Camera|Opening")
	bool m_bWasOpeningCameraActive = false;

	//オープニング開始時のカメラ距離
	UPROPERTY(VisibleAnywhere, Category = "Camera|Opening")
	float m_OpeningInitialArmLength = 0.f;

	//オープニング開始時のソケットオフセット
	UPROPERTY(VisibleAnywhere, Category = "Camera|Opening")
	FVector m_OpeningInitialSocketOffset = FVector::ZeroVector;

	//オープニング開始時のコントローラー回転
	UPROPERTY(VisibleAnywhere, Category = "Camera|Opening")
	FRotator m_OpeningInitialControlRotation = FRotator::ZeroRotator;

	//オープニング中のUI
	UPROPERTY()
	TObjectPtr<UUserWidget> m_OpeningWidget = nullptr;

	//オープニングUIを非表示にしたか
	UPROPERTY(VisibleAnywhere, Category = "Camera|Opening")
	bool m_bOpeningUiHidden = false;
};
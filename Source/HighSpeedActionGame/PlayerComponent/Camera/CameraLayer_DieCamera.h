//担当
//伊藤直樹

//-----------------------------------------------------
//死亡カメラレイヤー
//
// プレイヤー死亡時にカメラを死亡位置へ固定し、
// 少し引いた見下ろし構図へ切り替える
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "CameraLayer.h"
#include "CameraLayer_DieCamera.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraLayer_DieCamera : public UCameraLayer
{
	GENERATED_BODY()

public:
	//初期化
	UCameraLayer_DieCamera();

public:
	//レイヤー処理を適用
	virtual void ApplyLayer(FCameraContext& Context) override;

	//レイヤー名を取得
	virtual FName GetLayerName() const override;

private:
	//-----------------------------------------------------
	// DieCamera
	//-----------------------------------------------------

	//死亡カメラを更新
	void UpdateDieCamera(FCameraContext& Context);

	//死亡カメラを開始
	void StartDieCamera(FCameraContext& Context);

	//死亡カメラを終了
	void StopDieCamera(FCameraContext& Context);

	//死亡カメラ値を適用
	void ApplyDieCamera(FCameraContext& Context);

	//PlayerControllerを取得
	APlayerController* GetPlayerController(const FCameraContext& Context) const;

	//死亡要求か
	bool IsDieRequest(const FCameraContext& Context) const;

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//死亡カメラが有効だったか
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	bool m_bWasDieCameraActive = false;

	//死亡時に固定するSpringArm位置
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	FVector m_DieLockedSpringArmLocation = FVector::ZeroVector;

	//死亡時のSpringArm回転
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	FRotator m_DieTargetRotation = FRotator::ZeroRotator;

	//死亡時カメラの目標距離
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	float m_DieTargetArmLength = 0.f;

	//死亡前のPawnControlRotation使用状態
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	bool m_bOriginalUsePawnControlRotation = true;

	//死亡前のカメラ衝突状態
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	bool m_bOriginalCollisionTest = true;

	//死亡前のカメララグ状態
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	bool m_bOriginalCameraLag = true;

	//死亡前の回転ラグ状態
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	bool m_bOriginalRotationLag = false;

	//死亡前の絶対座標使用状態
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	bool m_bOriginalAbsoluteLocation = false;

	//死亡前の絶対回転使用状態
	UPROPERTY(VisibleAnywhere, Category = "Camera|Die")
	bool m_bOriginalAbsoluteRotation = false;
};
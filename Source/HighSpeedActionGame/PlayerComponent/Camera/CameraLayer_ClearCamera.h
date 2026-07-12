//担当
//伊藤直樹

//-----------------------------------------------------
//クリアカメラレイヤー
//
// ゲームクリア時の専用カメラを担当する
// 背面、側面、正面の順にカメラ角度を切り替えながらズーム演出を行う
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "CameraLayer.h"
#include "CameraLayer_ClearCamera.generated.h"

class APlayerController;
class UCharacterMovementComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraLayer_ClearCamera : public UCameraLayer
{
	GENERATED_BODY()

public:
	//初期化
	UCameraLayer_ClearCamera();

public:
	//レイヤー処理を適用
	virtual void ApplyLayer(FCameraContext& Context) override;

	//レイヤー名を取得
	virtual FName GetLayerName() const override;

private:
	//-----------------------------------------------------
	// ClearCamera
	//-----------------------------------------------------

	//クリアカメラを更新
	void UpdateClearCamera(FCameraContext& Context);

	//クリアカメラを開始
	void StartClearCamera(FCameraContext& Context);

	//クリアカメラを終了
	void StopClearCamera(FCameraContext& Context);

	//クリアカメラフェーズを更新
	void UpdateClearPhase(FCameraContext& Context);

	//指定フェーズを適用
	void ApplyClearPhase(FCameraContext& Context, EClearCameraPhase NewPhase);

	//クリアカメラのズームを更新
	void UpdateClearZoom(FCameraContext& Context);

	//クリア用の回転を作成
	FRotator MakeClearRotation(const FCameraContext& Context, float AddYaw, float Pitch) const;

	//PlayerControllerを取得
	APlayerController* GetPlayerController(const FCameraContext& Context) const;

	//CharacterMovementを取得
	UCharacterMovementComponent* GetCharacterMovement(const FCameraContext& Context) const;

	//クリア要求か
	bool IsClearRequest(const FCameraContext& Context) const;

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//クリアカメラが有効だったか
	UPROPERTY(VisibleAnywhere, Category = "Camera|Clear")
	bool m_bWasClearCameraActive = false;

	//現在のクリアフェーズ
	UPROPERTY(VisibleAnywhere, Category = "Camera|Clear")
	EClearCameraPhase m_CurrentClearPhase = EClearCameraPhase::None;

	//クリア中の現在カメラ距離
	UPROPERTY(VisibleAnywhere, Category = "Camera|Clear")
	float m_CurrentClearArmLength = 0.f;

	//クリア中の目標カメラ距離
	UPROPERTY(VisibleAnywhere, Category = "Camera|Clear")
	float m_TargetClearArmLength = 0.f;

	//元のPawnControlRotation使用状態
	UPROPERTY(VisibleAnywhere, Category = "Camera|Clear")
	bool m_bOriginalUsePawnControlRotation = true;

	//元のキャラ自動旋回状態
	UPROPERTY(VisibleAnywhere, Category = "Camera|Clear")
	bool m_bOriginalOrientRotationToMovement = true;

	//クリア中の現在SocketOffset
	UPROPERTY(VisibleAnywhere, Category = "Camera|Clear")
	FVector m_CurrentClearSocketOffset = FVector::ZeroVector;
};
//担当 
//伊藤直樹

//-----------------------------------------------------
//カメラ先読みレイヤー
//
// プレイヤーの速度方向とカメラ回転量に応じて、
// カメラの構図を少し先へずらす
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "CameraLayer.h"
#include "CameraLayer_Lead.generated.h"

class ACharacter;
class APlayerController;
class UCameraParameter;
class UCharacterMovementComponent;


UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraLayer_Lead : public UCameraLayer
{
	GENERATED_BODY()

public:
	//初期化
	UCameraLayer_Lead();

public:
	//レイヤー処理を適用
	virtual void ApplyLayer(FCameraContext& Context) override;

	//レイヤー名を取得
	virtual FName GetLayerName() const override;

private:
	//-----------------------------------------------------
	// 内部処理
	//-----------------------------------------------------

	//移動方向の先読み目標値を計算
	FVector CalculateTargetVelocityLead(
		const ACharacter* Character,
		const APlayerController* PlayerController,
		const UCameraParameter* CameraParam
	) const;

	//回転先読みYawを計算
	float CalculateTargetRotationLead(
		const APlayerController* PlayerController,
		const UCameraParameter* CameraParam
	);

	//先読み値を補間
	void FilterInertiaWithSpring(
		const FVector& TargetVelocityLead,
		float TargetYawDelta,
		const UCameraParameter* CameraParam,
		const FCameraContext& Context
	);

	//先読み値をContextへ反映
	void MapOffsetsToContext(
		FCameraContext& Context,
		const UCameraParameter* CameraParam
	) const;

	//LeadのDebugログを出す
	void DebugLeadLog(
		const ACharacter* Character,
		const FVector& TargetVelocityLead
	) const;

	//移動入力があるか
	bool HasMoveInput(
		const UCharacterMovementComponent* MovementComponent,
		const UCameraParameter* CameraParam
	) const;
private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//補間済み移動先読み
	UPROPERTY(VisibleAnywhere, Category = "Camera|Lead")
	FVector m_SmoothedVelocityLead = FVector::ZeroVector;

	//補間済み回転先読みYaw
	UPROPERTY(VisibleAnywhere, Category = "Camera|Lead")
	float m_SmoothedYawDelta = 0.f;

	//前フレームのカメラYaw
	UPROPERTY(VisibleAnywhere, Category = "Camera|Lead")
	float m_PrevYaw = 0.f;

	//現在の先読み補間速度
	UPROPERTY(VisibleAnywhere, Category = "Camera|Lead")
	float m_CurrentInterpSpeed = 0.f;
};
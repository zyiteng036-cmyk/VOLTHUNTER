//担当
//伊藤直樹

//-----------------------------------------------------
//カメラSoftClampソルバー
//
//画面内デッドゾーンからはみ出した量をカメラスライド量へ変換する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CameraSoftClampSolver.generated.h"

class AActor;

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraSoftClampSolver : public UObject
{
	GENERATED_BODY()

public:
	//デッドゾーンを超えた分のカメラスライド量を計算
	FVector EvaluateClampOffset(const AActor* OwnerActor, const FVector& CameraLocation, const FRotator& CameraRotation, float FOV, const FVector2D& ScreenOffset, const FVector2D& Deadzone) const;

private:
	//デッドゾーンからはみ出した角度を計算
	float CalculateSoftClampExcess(float Angle, float Limit) const;

	//画面アスペクト比を取得
	float GetViewportAspectRatio(const AActor* OwnerActor) const;

	//デッドゾーンをデバッグ描画
	void DrawDebugVisualizer(const AActor* OwnerActor, const FVector& CameraLocation, const FRotator& CameraRotation, const FVector& LocalTargetPosition, float CenterAngleY, float LimitY, float CenterAngleZ, float LimitZ, float ExcessY, float ExcessZ) const;

private:
	//Viewportを取得できない場合の画面比率
	static constexpr float DefaultAspectRatio = 16.f / 9.f;

	//近距離時に補正を無効化する距離
	static constexpr float SlideMinDistance = 120.f;

	//補正を最大適用する距離
	static constexpr float SlideMaxDistance = 200.f;

	//FOVを半角へ変換する倍率
	static constexpr float HalfAngleRatio = 0.5f;

	//デバッグ枠をカメラ前方へ描画する距離
	static constexpr float DebugViewDistance = 30.f;
};
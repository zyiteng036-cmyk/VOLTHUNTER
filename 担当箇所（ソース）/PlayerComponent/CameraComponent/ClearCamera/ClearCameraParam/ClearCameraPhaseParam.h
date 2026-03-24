//担当
//伊藤直樹



#pragma once

#include "CoreMinimal.h"
#include "ClearCameraPhaseParam.generated.h"


USTRUCT(BlueprintType)
struct FClearCameraPhaseParam
{
	GENERATED_BODY()

	// キャラクターの向きに対して加算するYaw角度
	// どの方向からキャラを見るかを決める
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClearCamera")
	float AddYaw = 0.f;

	// カメラの上下角度
	// 見下ろし、見上げの演出調整用
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClearCamera")
	float Pitch = 0.f;

	// SpringArm のソケットオフセット
	// カメラ位置の微調整用
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClearCamera")
	FVector SocketOffset = FVector::ZeroVector;

	// そのフェーズで使用するアーム長
	// 壁回避や寄り演出の基準距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClearCamera")
	float TargetArmLength = 0.f;
};
#include "CameraSoftClampSolver.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

//デッドゾーンを超えた分のカメラスライド量を計算
FVector UCameraSoftClampSolver::EvaluateClampOffset(const AActor* OwnerActor, const FVector& CameraLocation, const FRotator& CameraRotation, float FOV, const FVector2D& ScreenOffset, const FVector2D& Deadzone) const
{
	if (!OwnerActor) return FVector::ZeroVector;

	//カメラからプレイヤーへの相対位置をカメラ空間へ変換
	const FVector CameraToTarget = OwnerActor->GetActorLocation() - CameraLocation;
	const FVector LocalTargetPosition = CameraRotation.UnrotateVector(CameraToTarget);

	//カメラ後方にいる対象は補正しない
	if (LocalTargetPosition.X <= KINDA_SMALL_NUMBER)
	{
		return FVector::ZeroVector;
	}

	//カメラ正面から見た水平角度と垂直角度を計算
	const float AngleY = FMath::RadiansToDegrees(FMath::Atan2(LocalTargetPosition.Y, LocalTargetPosition.X));
	const float AngleZ = FMath::RadiansToDegrees(FMath::Atan2(LocalTargetPosition.Z, LocalTargetPosition.X));

	const float AspectRatio = GetViewportAspectRatio(OwnerActor);
	const float HalfHorizontalFOV = FOV * HalfAngleRatio;

	//水平FOVと画面比率から垂直FOVを計算
	const float HalfVerticalFOV = FMath::RadiansToDegrees(
		FMath::Atan(
			FMath::Tan(FMath::DegreesToRadians(HalfHorizontalFOV)) / AspectRatio
		)
	);

	//画面中心オフセットとデッドゾーンを角度へ変換
	const float CenterAngleY = ScreenOffset.X * HalfHorizontalFOV;
	const float CenterAngleZ = ScreenOffset.Y * HalfVerticalFOV;
	const float LimitY = Deadzone.X * HalfHorizontalFOV;
	const float LimitZ = Deadzone.Y * HalfVerticalFOV;

	//デッドゾーンからはみ出した角度だけを取得
	const float ExcessY = CalculateSoftClampExcess(AngleY - CenterAngleY, LimitY);
	const float ExcessZ = CalculateSoftClampExcess(AngleZ - CenterAngleZ, LimitZ);

	//現在は内部処理をコメントアウトしているデバッグ描画
	DrawDebugVisualizer(
		OwnerActor,
		CameraLocation,
		CameraRotation,
		LocalTargetPosition,
		CenterAngleY,
		LimitY,
		CenterAngleZ,
		LimitZ,
		ExcessY,
		ExcessZ
	);

	if (FMath::IsNearlyZero(ExcessY) && FMath::IsNearlyZero(ExcessZ))
	{
		return FVector::ZeroVector;
	}

	const FVector CameraRight = CameraRotation.RotateVector(FVector::RightVector);
	const FVector CameraUp = CameraRotation.RotateVector(FVector::UpVector);

	//はみ出した角度をカメラ空間上の移動距離へ変換
	float SlideY = LocalTargetPosition.X * FMath::Tan(FMath::DegreesToRadians(ExcessY));
	float SlideZ = LocalTargetPosition.X * FMath::Tan(FMath::DegreesToRadians(ExcessZ));

	//近距離時にカメラが急激に動かないよう補正量を減らす
	const float DistanceWeight = FMath::GetMappedRangeValueClamped(
		FVector2D(SlideMinDistance, SlideMaxDistance),
		FVector2D(0.f, 1.f),
		LocalTargetPosition.X
	);

	SlideY *= DistanceWeight;
	SlideZ *= DistanceWeight;

	//カメラの右方向と上方向をワールド空間の補正量へ変換
	return CameraRight * SlideY + CameraUp * SlideZ;
}

//デッドゾーンからはみ出した角度を計算
float UCameraSoftClampSolver::CalculateSoftClampExcess(float Angle, float Limit) const
{
	if (Angle > Limit)
	{
		return Angle - Limit;
	}

	if (Angle < -Limit)
	{
		return Angle + Limit;
	}

	return 0.f;
}

//画面アスペクト比を取得
float UCameraSoftClampSolver::GetViewportAspectRatio(const AActor* OwnerActor) const
{
	if (!OwnerActor) return DefaultAspectRatio;

	const APlayerController* PlayerController = OwnerActor->GetInstigatorController<APlayerController>();
	if (!PlayerController) return DefaultAspectRatio;

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

	//Viewportを取得できない環境では標準画面比率を使用
	if (!LocalPlayer || !LocalPlayer->ViewportClient)
	{
		return DefaultAspectRatio;
	}

	FVector2D ViewportSize = FVector2D::ZeroVector;
	LocalPlayer->ViewportClient->GetViewportSize(ViewportSize);

	if (ViewportSize.Y <= KINDA_SMALL_NUMBER)
	{
		return DefaultAspectRatio;
	}

	return ViewportSize.X / ViewportSize.Y;
}

//デッドゾーンをデバッグ描画
void UCameraSoftClampSolver::DrawDebugVisualizer(const AActor* OwnerActor, const FVector& CameraLocation, const FRotator& CameraRotation, const FVector& LocalTargetPosition, float CenterAngleY, float LimitY, float CenterAngleZ, float LimitZ, float ExcessY, float ExcessZ) const
{

	////デバッグ表示を再開する場合は以下を有効化
	//if (!OwnerActor) return;

	//UWorld* World = OwnerActor->GetWorld();
	//if (!World) return;

	//APlayerController* PlayerController =
	//	OwnerActor->GetInstigatorController<APlayerController>();

	//if (!PlayerController) return;

	//FVector RealCameraLocation = FVector::ZeroVector;
	//FRotator RealCameraRotation = FRotator::ZeroRotator;
	//PlayerController->GetPlayerViewPoint(RealCameraLocation, RealCameraRotation);

	//const FVector CameraForward =
	//	RealCameraRotation.Vector();

	//const FVector CameraRight =
	//	RealCameraRotation.RotateVector(FVector::RightVector);

	//const FVector CameraUp =
	//	RealCameraRotation.RotateVector(FVector::UpVector);

	//const float RightY =
	//	DebugViewDistance *
	//	FMath::Tan(FMath::DegreesToRadians(CenterAngleY + LimitY));

	//const float LeftY =
	//	DebugViewDistance *
	//	FMath::Tan(FMath::DegreesToRadians(CenterAngleY - LimitY));

	//const float TopZ =
	//	DebugViewDistance *
	//	FMath::Tan(FMath::DegreesToRadians(CenterAngleZ + LimitZ));

	//const float BottomZ =
	//	DebugViewDistance *
	//	FMath::Tan(FMath::DegreesToRadians(CenterAngleZ - LimitZ));

	//const FVector CenterPosition =
	//	RealCameraLocation + CameraForward * DebugViewDistance;

	//const FVector TopRight =
	//	CenterPosition + CameraRight * RightY + CameraUp * TopZ;

	//const FVector TopLeft =
	//	CenterPosition + CameraRight * LeftY + CameraUp * TopZ;

	//const FVector BottomRight =
	//	CenterPosition + CameraRight * RightY + CameraUp * BottomZ;

	//const FVector BottomLeft =
	//	CenterPosition + CameraRight * LeftY + CameraUp * BottomZ;

	//const bool bIsClamping =
	//	!FMath::IsNearlyZero(ExcessY) ||
	//	!FMath::IsNearlyZero(ExcessZ);

	//const FColor DebugColor =
	//	bIsClamping ? FColor::Red : FColor::Green;

	//DrawDebugLine(World, TopRight, TopLeft, DebugColor, false, -1.f, SDPG_Foreground, 2.f);
	//DrawDebugLine(World, TopLeft, BottomLeft, DebugColor, false, -1.f, SDPG_Foreground, 2.f);
	//DrawDebugLine(World, BottomLeft, BottomRight, DebugColor, false, -1.f, SDPG_Foreground, 2.f);
	//DrawDebugLine(World, BottomRight, TopRight, DebugColor, false, -1.f, SDPG_Foreground, 2.f);

	//DrawDebugSphere(
	//	World,
	//	OwnerActor->GetActorLocation(),
	//	20.f,
	//	8,
	//	FColor::Yellow,
	//	false,
	//	-1.f,
	//	SDPG_Foreground,
	//	1.5f
	//);

	//if (GEngine)
	//{
	//	const FString StateMessage =
	//		bIsClamping
	//		? TEXT("Deadzone State: OUTSIDE")
	//		: TEXT("Deadzone State: INSIDE");

	//	GEngine->AddOnScreenDebugMessage(
	//		20,
	//		0.f,
	//		DebugColor,
	//		StateMessage,
	//		true,
	//		FVector2D(1.5f, 1.5f)
	//	);

	//	const FString ValueMessage =
	//		FString::Printf(
	//			TEXT("LocalX: %.1f / Y: %.1f / Z: %.1f / ExcessY: %.2f / ExcessZ: %.2f"),
	//			LocalTargetPosition.X,
	//			LocalTargetPosition.Y,
	//			LocalTargetPosition.Z,
	//			ExcessY,
	//			ExcessZ
	//		);

	//	GEngine->AddOnScreenDebugMessage(
	//		21,
	//		0.f,
	//		FColor::Cyan,
	//		ValueMessage,
	//		true,
	//		FVector2D(1.2f, 1.2f)
	//	);
	//}
}
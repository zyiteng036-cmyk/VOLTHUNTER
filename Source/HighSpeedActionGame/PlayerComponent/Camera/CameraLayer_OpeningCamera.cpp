#include "CameraLayer_OpeningCamera.h"
#include "../../DataAssets/Player/PlayerCameraParameter.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

//初期化
UCameraLayer_OpeningCamera::UCameraLayer_OpeningCamera()
{
}

//レイヤー処理を適用
void UCameraLayer_OpeningCamera::ApplyLayer(FCameraContext& Context)
{
	if (!CanApplyLayer(Context)) return;

	if (IsOpeningRequest(Context))
	{
		UpdateOpeningCamera(Context);
		return;
	}

	if (m_bWasOpeningCameraActive)
	{
		StopOpeningCamera(Context);
	}
}

//レイヤー名を取得
FName UCameraLayer_OpeningCamera::GetLayerName() const
{
	return TEXT("CameraLayer_OpeningCamera");
}

//オープニングカメラを更新
void UCameraLayer_OpeningCamera::UpdateOpeningCamera(FCameraContext& Context)
{
	if (!Context.CameraParam) return;

	if (!m_bWasOpeningCameraActive)
	{
		StartOpeningCamera(Context);
	}

	ApplyOpeningCamera(Context);
	UpdateOpeningWidget(Context);

	if (Context.CameraRequest.IsFinished())
	{
		StopOpeningCamera(Context);
	}
}

//オープニングカメラを開始
void UCameraLayer_OpeningCamera::StartOpeningCamera(FCameraContext& Context)
{
	APlayerController* PC = GetPlayerController(Context);
	if (!PC) return;
	if (!Context.CameraParam) return;

	m_bWasOpeningCameraActive = true;
	m_bOpeningUiHidden = false;

	m_OpeningInitialArmLength = Context.TargetArmLength;
	m_OpeningInitialSocketOffset = Context.CumulativeSocketOffset;
	m_OpeningInitialControlRotation = PC->GetControlRotation();

	CreateOpeningWidget(Context);

	FRotator StartRotation = m_OpeningInitialControlRotation;
	StartRotation.Yaw += Context.CameraParam->OpeningStartYawOffset;
	StartRotation.Pitch = Context.CameraParam->OpeningStartPitch;
	StartRotation.Roll = 0.f;

	PC->SetControlRotation(StartRotation);

	Context.TargetArmLength = Context.CameraParam->OpeningFaceDistance;
	Context.CumulativeSocketOffset.Y = Context.CameraParam->OpeningFaceSocketOffsetY;
	Context.CumulativeSocketOffset.Z = Context.CameraParam->OpeningFaceSocketOffsetZ;

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Context.OwnerActor))
	{
		PlayerCharacter->DisableControl();
	}
}

//オープニングカメラを終了
void UCameraLayer_OpeningCamera::StopOpeningCamera(FCameraContext& Context)
{
	APlayerController* PC = GetPlayerController(Context);

	if (PC)
	{
		PC->SetControlRotation(m_OpeningInitialControlRotation);
	}

	Context.TargetArmLength = m_OpeningInitialArmLength;
	Context.CumulativeSocketOffset = m_OpeningInitialSocketOffset;

	RemoveOpeningWidget();

	m_bWasOpeningCameraActive = false;
	m_bOpeningUiHidden = false;
	m_OpeningInitialArmLength = 0.f;
	m_OpeningInitialSocketOffset = FVector::ZeroVector;
	m_OpeningInitialControlRotation = FRotator::ZeroRotator;

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Context.OwnerActor))
	{
		PlayerCharacter->EnableControl();
	}
}

//オープニングカメラ値を適用
void UCameraLayer_OpeningCamera::ApplyOpeningCamera(FCameraContext& Context)
{
	APlayerController* PC = GetPlayerController(Context);
	if (!PC) return;
	if (!Context.CameraParam) return;

	if (Context.CameraRequest.ElapsedTime < Context.CameraParam->OpeningWaitTime)
	{
		Context.TargetArmLength = Context.CameraParam->OpeningFaceDistance;
		Context.CumulativeSocketOffset.Y = Context.CameraParam->OpeningFaceSocketOffsetY;
		Context.CumulativeSocketOffset.Z = Context.CameraParam->OpeningFaceSocketOffsetZ;
		Context.HardClampOffset = FVector::ZeroVector;
		Context.CumulativeRotationOffset = FRotator::ZeroRotator;
		return;
	}

	const float MoveTime =
		Context.CameraRequest.ElapsedTime -
		Context.CameraParam->OpeningWaitTime;

	const float MoveAlpha =
		Context.CameraParam->OpeningMoveDuration <= 0.f
		? 1.f
		: FMath::Clamp(
			MoveTime / Context.CameraParam->OpeningMoveDuration,
			0.f,
			1.f
		);

	const float SmoothAlpha =
		FMath::InterpEaseInOut(
			0.f,
			1.f,
			MoveAlpha,
			Context.CameraParam->OpeningEaseExp
		);

	const float TargetArmLength =
		FMath::Lerp(
			Context.CameraParam->OpeningFaceDistance,
			m_OpeningInitialArmLength,
			SmoothAlpha
		);

	FVector FaceSocketOffset = m_OpeningInitialSocketOffset;
	FaceSocketOffset.Y = Context.CameraParam->OpeningFaceSocketOffsetY;
	FaceSocketOffset.Z = Context.CameraParam->OpeningFaceSocketOffsetZ;

	const FVector TargetSocketOffset =
		FMath::Lerp(
			FaceSocketOffset,
			m_OpeningInitialSocketOffset,
			SmoothAlpha
		);

	FRotator StartRotation = m_OpeningInitialControlRotation;
	StartRotation.Yaw += Context.CameraParam->OpeningStartYawOffset;
	StartRotation.Pitch = Context.CameraParam->OpeningStartPitch;
	StartRotation.Roll = 0.f;

	const FRotator TargetRotation =
		FMath::Lerp(
			StartRotation,
			m_OpeningInitialControlRotation,
			SmoothAlpha
		);

	Context.TargetArmLength = TargetArmLength;
	Context.CumulativeSocketOffset = TargetSocketOffset;
	Context.HardClampOffset = FVector::ZeroVector;
	Context.CumulativeRotationOffset = FRotator::ZeroRotator;

	PC->SetControlRotation(TargetRotation);
}

//オープニングUIを更新
void UCameraLayer_OpeningCamera::UpdateOpeningWidget(const FCameraContext& Context)
{
	if (!Context.CameraParam) return;
	if (!m_OpeningWidget) return;
	if (m_bOpeningUiHidden) return;

	const float MoveTime =
		Context.CameraRequest.ElapsedTime -
		Context.CameraParam->OpeningWaitTime;

	if (MoveTime < 0.f)
	{
		m_OpeningWidget->SetRenderOpacity(1.f);
		return;
	}

	const float FadeStartTime =
		Context.CameraParam->OpeningUiHideDelayTime;

	const float FadeEndTime =
		Context.CameraParam->OpeningUiHideDelayTime +
		Context.CameraParam->OpeningUiFadeDuration;

	if (MoveTime >= FadeEndTime)
	{
		RemoveOpeningWidget();
		m_bOpeningUiHidden = true;
		return;
	}

	if (MoveTime < FadeStartTime)
	{
		m_OpeningWidget->SetRenderOpacity(1.f);
		return;
	}

	const float Opacity =
		FMath::GetMappedRangeValueClamped(
			FVector2D(FadeStartTime, FadeEndTime),
			FVector2D(1.f, 0.f),
			MoveTime
		);

	m_OpeningWidget->SetRenderOpacity(Opacity);
}

//オープニングUIを生成
void UCameraLayer_OpeningCamera::CreateOpeningWidget(const FCameraContext& Context)
{
	if (!Context.CameraParam) return;
	if (!Context.CameraParam->OpeningWidgetClass) return;
	if (m_OpeningWidget) return;

	APlayerController* PC = GetPlayerController(Context);
	if (!PC) return;

	m_OpeningWidget =
		CreateWidget<UUserWidget>(
			PC,
			Context.CameraParam->OpeningWidgetClass
		);

	if (m_OpeningWidget)
	{
		m_OpeningWidget->AddToViewport();
		m_OpeningWidget->SetRenderOpacity(1.f);
	}
}

//オープニングUIを破棄
void UCameraLayer_OpeningCamera::RemoveOpeningWidget()
{
	if (!m_OpeningWidget) return;

	m_OpeningWidget->RemoveFromParent();
	m_OpeningWidget = nullptr;
}

//PlayerControllerを取得
APlayerController* UCameraLayer_OpeningCamera::GetPlayerController(
	const FCameraContext& Context
) const
{
	if (!Context.OwnerActor) return nullptr;

	return Context.OwnerActor->GetInstigatorController<APlayerController>();
}

//オープニング要求か
bool UCameraLayer_OpeningCamera::IsOpeningRequest(
	const FCameraContext& Context
) const
{
	return Context.CameraRequest.bIsActive &&
		Context.CameraRequest.RequestType == ECameraRequestType::Opening;
}
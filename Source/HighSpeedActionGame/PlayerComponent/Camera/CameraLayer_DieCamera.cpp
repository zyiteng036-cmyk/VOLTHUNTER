#include "CameraLayer_DieCamera.h"
#include "../../DataAssets/Player/PlayerCameraParameter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

//初期化
UCameraLayer_DieCamera::UCameraLayer_DieCamera()
{
}

//レイヤー処理を適用
void UCameraLayer_DieCamera::ApplyLayer(FCameraContext& Context)
{
	if (!CanApplyLayer(Context)) return;

	if (IsDieRequest(Context))
	{
		UpdateDieCamera(Context);
		return;
	}

	if (m_bWasDieCameraActive)
	{
		StopDieCamera(Context);
	}
}

//レイヤー名を取得
FName UCameraLayer_DieCamera::GetLayerName() const
{
	return TEXT("CameraLayer_DieCamera");
}

//死亡カメラを更新
void UCameraLayer_DieCamera::UpdateDieCamera(FCameraContext& Context)
{
	if (!Context.CameraParam) return;

	if (!m_bWasDieCameraActive)
	{
		StartDieCamera(Context);
	}

	ApplyDieCamera(Context);
}

//死亡カメラを開始
void UCameraLayer_DieCamera::StartDieCamera(FCameraContext& Context)
{
	if (!Context.SpringArm) return;
	if (!Context.CameraParam) return;

	APlayerController* PC = GetPlayerController(Context);
	if (!PC) return;

	m_bWasDieCameraActive = true;

	m_bOriginalUsePawnControlRotation = Context.SpringArm->bUsePawnControlRotation;
	m_bOriginalCollisionTest = Context.SpringArm->bDoCollisionTest;
	m_bOriginalCameraLag = Context.SpringArm->bEnableCameraLag;
	m_bOriginalRotationLag = Context.SpringArm->bEnableCameraRotationLag;
	m_bOriginalAbsoluteLocation = Context.SpringArm->IsUsingAbsoluteLocation();
	m_bOriginalAbsoluteRotation = Context.SpringArm->IsUsingAbsoluteRotation();

	m_DieLockedSpringArmLocation = Context.SpringArm->GetComponentLocation();

	m_DieTargetArmLength =
		Context.TargetArmLength +
		Context.CameraParam->DieZoomOutDistance;

	m_DieTargetRotation = PC->GetControlRotation();
	m_DieTargetRotation.Pitch = Context.CameraParam->DieTargetPitch;
	m_DieTargetRotation.Roll = 0.f;

	Context.SpringArm->bUsePawnControlRotation = false;
	Context.SpringArm->SetUsingAbsoluteLocation(true);
	Context.SpringArm->SetUsingAbsoluteRotation(true);
	Context.SpringArm->SetWorldLocation(m_DieLockedSpringArmLocation);
	Context.SpringArm->SetWorldRotation(m_DieTargetRotation);

	Context.SpringArm->bDoCollisionTest = false;
	Context.SpringArm->bEnableCameraLag = false;
	Context.SpringArm->bEnableCameraRotationLag = false;

	if (Context.Camera)
	{
		Context.Camera->SetRelativeLocation(FVector::ZeroVector);
		Context.Camera->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

//死亡カメラを終了
void UCameraLayer_DieCamera::StopDieCamera(FCameraContext& Context)
{
	if (Context.SpringArm)
	{
		Context.SpringArm->SetUsingAbsoluteLocation(m_bOriginalAbsoluteLocation);
		Context.SpringArm->SetUsingAbsoluteRotation(m_bOriginalAbsoluteRotation);

		Context.SpringArm->bUsePawnControlRotation = m_bOriginalUsePawnControlRotation;
		Context.SpringArm->bDoCollisionTest = m_bOriginalCollisionTest;
		Context.SpringArm->bEnableCameraLag = m_bOriginalCameraLag;
		Context.SpringArm->bEnableCameraRotationLag = m_bOriginalRotationLag;
	}

	if (Context.Camera)
	{
		Context.Camera->SetRelativeLocation(FVector::ZeroVector);
		Context.Camera->SetRelativeRotation(FRotator::ZeroRotator);
	}

	m_bWasDieCameraActive = false;
	m_DieLockedSpringArmLocation = FVector::ZeroVector;
	m_DieTargetRotation = FRotator::ZeroRotator;
	m_DieTargetArmLength = 0.f;

	m_bOriginalUsePawnControlRotation = true;
	m_bOriginalCollisionTest = true;
	m_bOriginalCameraLag = true;
	m_bOriginalRotationLag = false;
	m_bOriginalAbsoluteLocation = false;
	m_bOriginalAbsoluteRotation = false;
}

//死亡カメラ値を適用
void UCameraLayer_DieCamera::ApplyDieCamera(FCameraContext& Context)
{
	if (!Context.SpringArm) return;

	Context.SpringArm->SetWorldLocation(m_DieLockedSpringArmLocation);
	Context.SpringArm->SetWorldRotation(m_DieTargetRotation);

	Context.TargetArmLength = m_DieTargetArmLength;
	Context.CumulativeRotationOffset = FRotator::ZeroRotator;
	Context.HardClampOffset = FVector::ZeroVector;

	if (Context.Camera)
	{
		Context.Camera->SetRelativeLocation(FVector::ZeroVector);
		Context.Camera->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

//PlayerControllerを取得
APlayerController* UCameraLayer_DieCamera::GetPlayerController(const FCameraContext& Context) const
{
	if (!Context.OwnerActor) return nullptr;

	return Context.OwnerActor->GetInstigatorController<APlayerController>();
}

//死亡要求か
bool UCameraLayer_DieCamera::IsDieRequest(const FCameraContext& Context) const
{
	return Context.CameraRequest.bIsActive &&
		Context.CameraRequest.RequestType == ECameraRequestType::Die;
}
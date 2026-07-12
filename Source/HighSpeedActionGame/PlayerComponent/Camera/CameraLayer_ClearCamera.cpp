#include "CameraLayer_ClearCamera.h"
#include "../../DataAssets/Player/PlayerCameraParameter.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

//初期化
UCameraLayer_ClearCamera::UCameraLayer_ClearCamera()
{
}

//レイヤー処理を適用
void UCameraLayer_ClearCamera::ApplyLayer(FCameraContext& Context)
{
	if (!CanApplyLayer(Context)) return;

	if (IsClearRequest(Context))
	{
		UpdateClearCamera(Context);
		return;
	}

	if (m_bWasClearCameraActive)
	{
		StopClearCamera(Context);
	}
}

//レイヤー名を取得
FName UCameraLayer_ClearCamera::GetLayerName() const
{
	return TEXT("CameraLayer_ClearCamera");
}

//クリアカメラを更新
void UCameraLayer_ClearCamera::UpdateClearCamera(FCameraContext& Context)
{
	if (!m_bWasClearCameraActive)
	{
		StartClearCamera(Context);
	}

	UpdateClearPhase(Context);
	UpdateClearZoom(Context);

	Context.TargetArmLength = m_CurrentClearArmLength;
	Context.CumulativeSocketOffset = m_CurrentClearSocketOffset;

	//クリア演出中は通常補正の残りを消す
	Context.CumulativeRotationOffset = FRotator::ZeroRotator;
	Context.HardClampOffset = FVector::ZeroVector;
}


//クリアカメラを開始
void UCameraLayer_ClearCamera::StartClearCamera(FCameraContext& Context)
{
	if (!Context.SpringArm) return;

	m_bWasClearCameraActive = true;
	m_CurrentClearPhase = EClearCameraPhase::None;

	m_CurrentClearArmLength = Context.TargetArmLength;
	m_TargetClearArmLength = Context.TargetArmLength;
	m_CurrentClearSocketOffset = Context.CumulativeSocketOffset;

	m_bOriginalUsePawnControlRotation = Context.SpringArm->bUsePawnControlRotation;

	Context.SpringArm->bUsePawnControlRotation = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement(Context))
	{
		m_bOriginalOrientRotationToMovement = MoveComp->bOrientRotationToMovement;
		MoveComp->bOrientRotationToMovement = false;
	}
}


//クリアカメラを終了
void UCameraLayer_ClearCamera::StopClearCamera(FCameraContext& Context)
{
	if (Context.SpringArm)
	{
		Context.SpringArm->bUsePawnControlRotation = m_bOriginalUsePawnControlRotation;
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement(Context))
	{
		MoveComp->bOrientRotationToMovement = m_bOriginalOrientRotationToMovement;
	}

	m_bWasClearCameraActive = false;
	m_CurrentClearPhase = EClearCameraPhase::None;
	m_CurrentClearArmLength = 0.f;
	m_TargetClearArmLength = 0.f;
	m_CurrentClearSocketOffset = FVector::ZeroVector;


	m_bOriginalUsePawnControlRotation = true;
	m_bOriginalOrientRotationToMovement = true;
}

//クリアカメラフェーズを更新
void UCameraLayer_ClearCamera::UpdateClearPhase(FCameraContext& Context)
{
	if (!Context.CameraParam) return;

	const float ElapsedTime = Context.CameraRequest.ElapsedTime;

	EClearCameraPhase NewPhase = EClearCameraPhase::None;

	if (ElapsedTime >= Context.CameraParam->ClearCameraFrontDelay)
	{
		NewPhase = EClearCameraPhase::Front;
	}
	else if (ElapsedTime >= Context.CameraParam->ClearCameraSideDelay)
	{
		NewPhase = EClearCameraPhase::Side;
	}
	else if (ElapsedTime >= Context.CameraParam->ClearCameraBackDelay)
	{
		NewPhase = EClearCameraPhase::Back;
	}

	if (m_CurrentClearPhase == NewPhase) return;

	ApplyClearPhase(Context, NewPhase);
}

//指定フェーズを適用
void UCameraLayer_ClearCamera::ApplyClearPhase(FCameraContext& Context, EClearCameraPhase NewPhase)
{
	if (!Context.CameraParam) return;

	APlayerController* PC = GetPlayerController(Context);
	if (!PC) return;

	m_CurrentClearPhase = NewPhase;
	Context.CameraRequest.ClearPhase = NewPhase;

	switch (NewPhase)
	{
	case EClearCameraPhase::Back:
		PC->SetControlRotation(
			MakeClearRotation(
				Context,
				Context.CameraParam->BackClearAddYaw,
				Context.CameraParam->BackClearPitch
			)
		);

		//背面フェーズだけ一度指定距離へ飛ばす
		m_CurrentClearArmLength = Context.CameraParam->BackClearTargetArmLength;
		m_TargetClearArmLength = Context.CameraParam->ClearZoomLength;
		m_CurrentClearSocketOffset = Context.CameraParam->BackClearSocketOffset;
		break;

	case EClearCameraPhase::Side:
		PC->SetControlRotation(
			MakeClearRotation(
				Context,
				Context.CameraParam->SideClearAddYaw,
				Context.CameraParam->SideClearPitch
			)
		);

		m_TargetClearArmLength = Context.CameraParam->ClearZoomLength;
		m_CurrentClearSocketOffset = Context.CameraParam->SideClearSocketOffset;
		break;

	case EClearCameraPhase::Front:
		PC->SetControlRotation(
			MakeClearRotation(
				Context,
				Context.CameraParam->FrontClearAddYaw,
				Context.CameraParam->FrontClearPitch
			)
		);

		m_TargetClearArmLength = Context.CameraParam->ClearZoomLength;
		m_CurrentClearSocketOffset = Context.CameraParam->FrontClearSocketOffset;
		break;

	default:
		break;
	}
}


//クリアカメラのズームを更新
void UCameraLayer_ClearCamera::UpdateClearZoom(FCameraContext& Context)
{
	if (!Context.CameraParam) return;

	m_CurrentClearArmLength =
		FMath::FInterpTo(
			m_CurrentClearArmLength,
			m_TargetClearArmLength,
			Context.DeltaTime,
			Context.CameraParam->ClearZoomInterpSpeed
		);
}

//クリア用の回転を作成
FRotator UCameraLayer_ClearCamera::MakeClearRotation(
	const FCameraContext& Context,
	float AddYaw,
	float Pitch
) const
{
	if (!Context.OwnerActor) return FRotator::ZeroRotator;

	FRotator ClearRotation = Context.OwnerActor->GetActorRotation();
	ClearRotation.Yaw += AddYaw;
	ClearRotation.Pitch = Pitch;
	ClearRotation.Roll = 0.f;

	return ClearRotation;
}

//PlayerControllerを取得
APlayerController* UCameraLayer_ClearCamera::GetPlayerController(const FCameraContext& Context) const
{
	if (!Context.OwnerActor) return nullptr;

	return Context.OwnerActor->GetInstigatorController<APlayerController>();
}

//CharacterMovementを取得
UCharacterMovementComponent* UCameraLayer_ClearCamera::GetCharacterMovement(const FCameraContext& Context) const
{
	const ACharacter* Character = Cast<ACharacter>(Context.OwnerActor.Get());
	if (!Character) return nullptr;

	return Character->GetCharacterMovement();
}

//クリア要求か
bool UCameraLayer_ClearCamera::IsClearRequest(const FCameraContext& Context) const
{
	return Context.CameraRequest.bIsActive &&
		Context.CameraRequest.RequestType == ECameraRequestType::Clear;
}
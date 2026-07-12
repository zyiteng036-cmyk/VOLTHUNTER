#include "CameraLayer_Action.h"
#include "../../DataAssets/Player/PlayerCameraParameter.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	//スキルズームイン到達判定
	constexpr float SkillZoomInTolerance = 5.f;

	//ブレンド最大値
	constexpr float MaxBlendAlpha = 1.f;

	//ブレンド最小値
	constexpr float MinBlendAlpha = 0.f;
}

//初期化
UCameraLayer_Action::UCameraLayer_Action()
{
}

//レイヤー処理を適用
void UCameraLayer_Action::ApplyLayer(FCameraContext& Context)
{
	if (!CanApplyLayer(Context)) return;

	UpdateActionCamera(Context);
	UpdateActionReturn(Context);
}

//レイヤー名を取得
FName UCameraLayer_Action::GetLayerName() const
{
	return TEXT("CameraLayer_Action");
}

//アクションカメラを更新
void UCameraLayer_Action::UpdateActionCamera(FCameraContext& Context)
{
	if (!Context.CameraRequest.bIsActive) return;

	switch (Context.CameraRequest.RequestType)
	{
	case ECameraRequestType::Skill:
		UpdateSkillCamera(Context);
		break;

	case ECameraRequestType::JustEvasiveLook:
		UpdateJustEvasiveLookCamera(Context);
		break;

	case ECameraRequestType::SplineMove:
		UpdateSplineMoveCamera(Context);
		break;

	case ECameraRequestType::AnimOverride:
		//今後AnimNotify由来のカメラ上書きを追加
		break;

	default:
		break;
	}
}

//アクションカメラ解除中の復帰処理
void UCameraLayer_Action::UpdateActionReturn(FCameraContext& Context)
{
	if (IsRequestType(Context, ECameraRequestType::Skill)) return;
	if (IsRequestType(Context, ECameraRequestType::SplineMove)) return;
	if (IsRequestType(Context, ECameraRequestType::JustEvasiveLook)) return;

	if (m_bWasSkillCameraActive)
	{
		ReturnSkillCamera(Context);
	}

	if (m_bWasSplineMoveCameraActive)
	{
		ReturnSplineMoveCamera(Context);
	}
}

//現在の要求が指定タイプか
bool UCameraLayer_Action::IsRequestType(
	const FCameraContext& Context,
	ECameraRequestType RequestType
) const
{
	return Context.CameraRequest.bIsActive &&
		Context.CameraRequest.RequestType == RequestType;
}

//スキルカメラを更新
void UCameraLayer_Action::UpdateSkillCamera(FCameraContext& Context)
{
	if (!Context.CameraParam) return;

	if (!m_bWasSkillCameraActive)
	{
		m_CurrentSkillArmLength = Context.TargetArmLength;
		m_SkillPhase = ECameraSkillPhase::ZoomIn;
		m_bWasSkillCameraActive = true;
	}

	UpdateSkillPhase(Context);
	ApplySkillCamera(Context);
}


//スキルカメラのフェーズを更新
void UCameraLayer_Action::UpdateSkillPhase(const FCameraContext& Context)
{
	if (!Context.CameraParam) return;
	if (m_SkillPhase != ECameraSkillPhase::ZoomIn) return;

	const bool bReachedZoomIn =
		FMath::Abs(m_CurrentSkillArmLength - Context.CameraParam->SkillZoomInArmLength)
		<= SkillZoomInTolerance;

	if (bReachedZoomIn)
	{
		m_SkillPhase = ECameraSkillPhase::ZoomOut;
	}
}

//スキルカメラ距離を適用
void UCameraLayer_Action::ApplySkillCamera(FCameraContext& Context)
{
	if (!Context.CameraParam) return;

	const float TargetArmLength =
		m_SkillPhase == ECameraSkillPhase::ZoomIn
		? Context.CameraParam->SkillZoomInArmLength
		: Context.CameraParam->SkillZoomOutArmLength;

	const float InterpSpeed =
		m_SkillPhase == ECameraSkillPhase::ZoomIn
		? Context.CameraParam->SkillZoomInSpeed
		: Context.CameraParam->SkillZoomOutSpeed;

	m_CurrentSkillArmLength =
		FMath::FInterpTo(
			m_CurrentSkillArmLength,
			TargetArmLength,
			Context.DeltaTime,
			InterpSpeed
		);

	//スキル中は距離を直接上書きする
	Context.TargetArmLength = m_CurrentSkillArmLength;
}


//スキルカメラを解除方向へ戻す
void UCameraLayer_Action::ReturnSkillCamera(FCameraContext& Context)
{
	if (!Context.CameraParam) return;

	const float NormalArmLength = Context.TargetArmLength;

	m_CurrentSkillArmLength =
		FMath::FInterpTo(
			m_CurrentSkillArmLength,
			NormalArmLength,
			Context.DeltaTime,
			Context.CameraParam->SkillCameraReturnSpeed
		);

	Context.TargetArmLength = m_CurrentSkillArmLength;

	if (
		FMath::IsNearlyEqual(
			m_CurrentSkillArmLength,
			NormalArmLength,
			SkillZoomInTolerance
		)
		)
	{
		m_CurrentSkillArmLength = 0.f;
		m_SkillCameraBlendAlpha = 0.f;
		m_SkillPhase = ECameraSkillPhase::None;
		m_bWasSkillCameraActive = false;
	}
}


//ジャスト回避後の敵注視カメラを更新
void UCameraLayer_Action::UpdateJustEvasiveLookCamera(FCameraContext& Context)
{
	if (!Context.CameraParam) return;
	if (!Context.CameraRequest.TargetInfo.bHasTarget) return;

	const FRotator TargetRotation =
		CalculateLookAtTargetRotation(Context);

	InterpControllerRotation(Context, TargetRotation);
}

//敵方向への回転を計算
FRotator UCameraLayer_Action::CalculateLookAtTargetRotation(
	const FCameraContext& Context
) const
{
	if (!Context.OwnerActor)
	{
		return FRotator::ZeroRotator;
	}

	APlayerController* PC = GetPlayerController(Context);
	if (!PC)
	{
		return FRotator::ZeroRotator;
	}

	const FRotator CurrentRotation =
		PC->GetControlRotation();

	FVector Direction =
		Context.CameraRequest.TargetInfo.TargetLocation -
		Context.CameraLocation;

	//高さ方向は変更せず、水平面上の敵方向だけを見る
	Direction.Z = 0.f;

	if (!Direction.Normalize())
	{
		return CurrentRotation;
	}

	FRotator TargetRotation =
		Direction.Rotation();

	//現在のカメラPitchを維持する
	TargetRotation.Pitch = CurrentRotation.Pitch;
	TargetRotation.Roll = 0.f;

	return TargetRotation;
}



//Controller回転を補間
void UCameraLayer_Action::InterpControllerRotation(
	const FCameraContext& Context,
	const FRotator& TargetRotation
) const
{
	APlayerController* PC = GetPlayerController(Context);
	if (!PC) return;
	if (!Context.CameraParam) return;

	const FRotator CurrentRotation =
		PC->GetControlRotation();

	const UWorld* World = PC->GetWorld();

	const float TimeDilation = World
		? UGameplayStatics::GetGlobalTimeDilation(World)
		: 1.f;

	//スロー中でもカメラ回転速度を維持する
	const float CameraDeltaTime =
		Context.DeltaTime *
		FMath::Clamp(TimeDilation, 0.05f, 1.f);

	FRotator HorizontalTargetRotation =
		TargetRotation;

	HorizontalTargetRotation.Pitch =
		CurrentRotation.Pitch;

	HorizontalTargetRotation.Roll = 0.f;

	FRotator NewRotation =
		FMath::RInterpTo(
			CurrentRotation,
			HorizontalTargetRotation,
			CameraDeltaTime,
			Context.CameraParam->JustEvasiveLookInterpSpeed
		);

	//補間誤差でもPitchが変わらないよう固定する
	NewRotation.Pitch = CurrentRotation.Pitch;
	NewRotation.Roll = 0.f;

	PC->SetControlRotation(NewRotation);
}


//PlayerControllerを取得
APlayerController* UCameraLayer_Action::GetPlayerController(const FCameraContext& Context) const
{
	if (!Context.OwnerActor) return nullptr;

	return Context.OwnerActor->GetInstigatorController<APlayerController>();
}

//Spline移動カメラを更新
void UCameraLayer_Action::UpdateSplineMoveCamera(FCameraContext& Context)
{
	if (!Context.CameraParam) return;

	m_bWasSplineMoveCameraActive = true;

	m_SplineMoveCameraBlendAlpha =
		FMath::FInterpTo(
			m_SplineMoveCameraBlendAlpha,
			1.f,
			Context.DeltaTime,
			Context.CameraParam->SplineMoveBlendInSpeed
		);

	Context.TargetArmLength =
		FMath::Lerp(
			Context.TargetArmLength,
			Context.CameraParam->SplineMoveArmLength,
			m_SplineMoveCameraBlendAlpha
		);

	Context.FinalFOV =
		FMath::Lerp(
			Context.FinalFOV,
			Context.CameraParam->SplineMoveFOV,
			m_SplineMoveCameraBlendAlpha
		);

	Context.CumulativeSocketOffset =
		FMath::Lerp(
			Context.CumulativeSocketOffset,
			Context.CumulativeSocketOffset + Context.CameraParam->SplineMoveSocketOffset,
			m_SplineMoveCameraBlendAlpha
		);
}

//Spline移動カメラを解除方向へ戻す
void UCameraLayer_Action::ReturnSplineMoveCamera(FCameraContext& Context)
{
	if (!Context.CameraParam) return;

	m_SplineMoveCameraBlendAlpha =
		FMath::FInterpTo(
			m_SplineMoveCameraBlendAlpha,
			0.f,
			Context.DeltaTime,
			Context.CameraParam->SplineMoveBlendOutSpeed
		);

	Context.TargetArmLength =
		FMath::Lerp(
			Context.TargetArmLength,
			Context.CameraParam->SplineMoveArmLength,
			m_SplineMoveCameraBlendAlpha
		);

	Context.FinalFOV =
		FMath::Lerp(
			Context.FinalFOV,
			Context.CameraParam->SplineMoveFOV,
			m_SplineMoveCameraBlendAlpha
		);

	Context.CumulativeSocketOffset =
		FMath::Lerp(
			Context.CumulativeSocketOffset,
			Context.CumulativeSocketOffset + Context.CameraParam->SplineMoveSocketOffset,
			m_SplineMoveCameraBlendAlpha
		);

	if (m_SplineMoveCameraBlendAlpha <= KINDA_SMALL_NUMBER)
	{
		m_SplineMoveCameraBlendAlpha = 0.f;
		m_bWasSplineMoveCameraActive = false;
	}
}

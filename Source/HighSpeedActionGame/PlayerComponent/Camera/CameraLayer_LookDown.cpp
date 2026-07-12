#include "CameraLayer_LookDown.h"
#include "../../DataAssets/Player/PlayerCameraParameter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	//落下速度判定用の最小値
	constexpr float MinFallSpeed = 1.f;
}

//初期化
UCameraLayer_LookDown::UCameraLayer_LookDown()
{
}

//レイヤー処理を適用
void UCameraLayer_LookDown::ApplyLayer(FCameraContext& Context)
{
	if (!CanApplyLayer(Context)) return;

	if (!CanApplyLookDown(Context))
	{
		ResetLookDown(Context);
		Context.CumulativeRotationOffset.Pitch += m_CurrentPitchOffset;
		return;
	}

	m_LookDownConditionTime += Context.DeltaTime;

	if (m_LookDownConditionTime < Context.CameraParam->LookDownDelayTime)
	{
		ResetLookDown(Context);
		Context.CumulativeRotationOffset.Pitch += m_CurrentPitchOffset;
		return;
	}

	const float TargetPitchOffset = CalculateTargetPitchOffset(Context);

	m_CurrentPitchOffset =
		FMath::FInterpTo(
			m_CurrentPitchOffset,
			TargetPitchOffset,
			Context.DeltaTime,
			Context.CameraParam->LookDownBlendInSpeed
		);

	Context.CumulativeRotationOffset.Pitch += m_CurrentPitchOffset;
}

//レイヤー名を取得
FName UCameraLayer_LookDown::GetLayerName() const
{
	return TEXT("CameraLayer_LookDown");
}

//見下ろしを適用できるか
bool UCameraLayer_LookDown::CanApplyLookDown(const FCameraContext& Context) const
{
	if (!Context.CameraParam) return false;
	if (!Context.OwnerActor) return false;

	if (Context.bIsBossBattle)
	{
		return false;
	}

	if (Context.bIsPlayerMovingCamera)
	{
		return false;
	}

	const ACharacter* Character = Cast<ACharacter>(Context.OwnerActor);
	if (!Character) return false;

	const UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp) return false;

	if (!MoveComp->IsFalling())
	{
		return false;
	}

	return Context.PlayerVelocity.Z < -MinFallSpeed;
}

//目標Pitchを計算
float UCameraLayer_LookDown::CalculateTargetPitchOffset(const FCameraContext& Context) const
{
	if (!Context.CameraParam) return 0.f;

	const float FallSpeed = FMath::Abs(Context.PlayerVelocity.Z);
	const float TargetFallSpeed = FMath::Abs(Context.CameraParam->LookDownTargetZVelocity);

	if (TargetFallSpeed <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	const float FallAlpha =
		FMath::Clamp(
			FallSpeed / TargetFallSpeed,
			0.f,
			1.f
		);

	return Context.CameraParam->MaxLookDownPitchOffset * FallAlpha;
}

//見下ろしを通常状態へ戻す
void UCameraLayer_LookDown::ResetLookDown(const FCameraContext& Context)
{
	m_LookDownConditionTime = 0.f;

	if (!Context.CameraParam) return;

	m_CurrentPitchOffset =
		FMath::FInterpTo(
			m_CurrentPitchOffset,
			0.f,
			Context.DeltaTime,
			Context.CameraParam->LookDownBlendOutSpeed
		);
}
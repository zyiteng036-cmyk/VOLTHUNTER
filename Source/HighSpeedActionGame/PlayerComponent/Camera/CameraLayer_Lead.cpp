#include "CameraLayer_Lead.h"
#include "../../DataAssets/Player/PlayerCameraParameter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

namespace
{
	//非アクティブ時の速度リセット補間スピード
	constexpr float InactiveResetInterpSpeed = 10.f;

	//先読みを発生させる最小移動速度
	constexpr float MinSpeedThreshold = 500.f;

	//Yaw先読みの補間速度
	constexpr float YawInterpSpeed = 10.f;
}

//初期化
UCameraLayer_Lead::UCameraLayer_Lead()
{
}

//レイヤー処理を適用
void UCameraLayer_Lead::ApplyLayer(FCameraContext& Context)
{
	if (!Context.CameraParam || !IsLayerActive())
	{
		m_SmoothedVelocityLead =
			FMath::VInterpTo(
				m_SmoothedVelocityLead,
				FVector::ZeroVector,
				Context.DeltaTime,
				InactiveResetInterpSpeed
			);

		m_SmoothedYawDelta =
			FMath::FInterpTo(
				m_SmoothedYawDelta,
				0.f,
				Context.DeltaTime,
				InactiveResetInterpSpeed
			);

		if (Context.CameraParam)
		{
			MapOffsetsToContext(Context, Context.CameraParam);
		}

		return;
	}

	if (!Context.OwnerActor) return;

	const ACharacter* Character =
		Cast<ACharacter>(Context.OwnerActor);

	const APlayerController* PlayerController =
		Character
		? Cast<APlayerController>(Character->GetController())
		: nullptr;

	if (!Character || !PlayerController) return;

	const UCameraParameter* CameraParam = Context.CameraParam;

	FVector TargetVelocityLead =
		CalculateTargetVelocityLead(
			Character,
			PlayerController,
			CameraParam
		);

	const float TargetYawDelta =
		CalculateTargetRotationLead(
			PlayerController,
			CameraParam
		);

	FilterInertiaWithSpring(
		TargetVelocityLead,
		TargetYawDelta,
		CameraParam,
		Context
	);

	m_PrevYaw = PlayerController->GetControlRotation().Yaw;

	MapOffsetsToContext(Context, CameraParam);

	DebugLeadLog(Character, TargetVelocityLead);
}


//レイヤー名を取得
FName UCameraLayer_Lead::GetLayerName() const
{
	return TEXT("CameraLayer_Lead");
}

//移動方向の先読み目標値を計算
FVector UCameraLayer_Lead::CalculateTargetVelocityLead(
	const ACharacter* Character,
	const APlayerController* PlayerController,
	const UCameraParameter* CameraParam
) const
{
	if (!Character) return FVector::ZeroVector;
	if (!PlayerController) return FVector::ZeroVector;
	if (!CameraParam) return FVector::ZeroVector;

	const UCharacterMovementComponent* MovementComponent =
		Character->GetCharacterMovement();

	if (!MovementComponent) return FVector::ZeroVector;

	//入力がない時は、ダッシュ慣性などで速度が残っていてもLeadしない
	if (
		CameraParam->bStopLeadWhenNoMoveInput &&
		!HasMoveInput(MovementComponent, CameraParam)
		)
	{
		return FVector::ZeroVector;
	}

	FVector Velocity = MovementComponent->Velocity;
	Velocity.Z = 0.f;

	const float Speed = Velocity.Size();

	if (Speed <= MinSpeedThreshold)
	{
		return FVector::ZeroVector;
	}

	const float SpeedRange =
		FMath::Max(
			1.f,
			CameraParam->VelocityLeadSpeedRef - MinSpeedThreshold
		);

	const float Alpha =
		FMath::Clamp(
			(Speed - MinSpeedThreshold) / SpeedRange,
			0.f,
			1.f
		);

	FVector CameraRight =
		PlayerController->GetControlRotation().RotateVector(FVector::RightVector);

	FVector CameraForward =
		PlayerController->GetControlRotation().RotateVector(FVector::ForwardVector);

	CameraRight.Z = 0.f;
	CameraForward.Z = 0.f;

	CameraRight.Normalize();
	CameraForward.Normalize();

	const FVector MoveDirection =
		Velocity.GetSafeNormal();

	const float RightDot =
		FVector::DotProduct(MoveDirection, CameraRight);

	const float ForwardDot =
		FVector::DotProduct(MoveDirection, CameraForward);

	const float ForwardLead =
		ForwardDot > 0.f
		? ForwardDot * CameraParam->LeadDistForward * Alpha
		: ForwardDot * CameraParam->LeadDistBackward * Alpha;

	const float RightLead =
		RightDot > 0.f
		? RightDot * CameraParam->LeadDistRight * Alpha
		: RightDot * CameraParam->LeadDistLeft * Alpha;

	return FVector(ForwardLead, RightLead, 0.f);
}


//回転先読みYawを計算
float UCameraLayer_Lead::CalculateTargetRotationLead(
	const APlayerController* PlayerController,
	const UCameraParameter* CameraParam
)
{
	if (!PlayerController) return 0.f;
	if (!CameraParam) return 0.f;

	const float CurrentYaw =
		PlayerController->GetControlRotation().Yaw;

	const float TargetYawDelta =
		FMath::FindDeltaAngleDegrees(
			m_PrevYaw,
			CurrentYaw
		);

	return FMath::Clamp(
		TargetYawDelta,
		-CameraParam->RotationLeadYawClamp,
		CameraParam->RotationLeadYawClamp
	);
}

//先読み値を補間
void UCameraLayer_Lead::FilterInertiaWithSpring(
	const FVector& TargetVelocityLead,
	float TargetYawDelta,
	const UCameraParameter* CameraParam,
	const FCameraContext& Context
)
{
	if (!CameraParam) return;

	const bool bIsReturning =
		TargetVelocityLead.IsNearlyZero();

	if (bIsReturning)
	{
		const float CurrentDistance =
			m_SmoothedVelocityLead.Size();

		const float ReturnAlpha =
			FMath::Clamp(
				CurrentDistance / CameraParam->LeadReturnSlowDistance,
				0.f,
				1.f
			);

		const float ReturnSpeed =
			FMath::Lerp(
				CameraParam->LeadReturnMinSpeed,
				CameraParam->LeadReturnConstantSpeed,
				ReturnAlpha
			);

		m_SmoothedVelocityLead =
			FMath::VInterpConstantTo(
				m_SmoothedVelocityLead,
				FVector::ZeroVector,
				Context.DeltaTime,
				ReturnSpeed
			);

		if (m_SmoothedVelocityLead.SizeSquared() <=
			FMath::Square(CameraParam->LeadReturnSnapTolerance))
		{
			m_SmoothedVelocityLead = FVector::ZeroVector;
		}

		m_CurrentInterpSpeed = ReturnSpeed;
	}
	else
	{
		m_CurrentInterpSpeed = CameraParam->LeadInterpSpeed;

		m_SmoothedVelocityLead =
			FMath::VInterpTo(
				m_SmoothedVelocityLead,
				TargetVelocityLead,
				Context.DeltaTime,
				m_CurrentInterpSpeed
			);
	}

	const float TargetReturnYawDelta =
		bIsReturning
		? 0.f
		: TargetYawDelta;

	m_SmoothedYawDelta =
		FMath::FInterpTo(
			m_SmoothedYawDelta,
			TargetReturnYawDelta,
			Context.DeltaTime,
			YawInterpSpeed
		);
}



//先読み値をContextへ反映
void UCameraLayer_Lead::MapOffsetsToContext(
	FCameraContext& Context,
	const UCameraParameter* CameraParam
) const
{
	if (!CameraParam) return;

	const float ForwardLead =
		m_SmoothedVelocityLead.X;

	if (ForwardLead > 0.f)
	{
		Context.CumulativeSocketOffset.Z +=
			ForwardLead * CameraParam->ForwardLeadFraction;
	}
	else
	{
		Context.TargetArmLength +=
			FMath::Abs(ForwardLead) * CameraParam->ForwardLeadFraction;
	}

	const float RotationLeadLocalY =
		m_SmoothedYawDelta * CameraParam->RotationLeadScale;

	Context.CumulativeSocketOffset.Y +=
		m_SmoothedVelocityLead.Y + RotationLeadLocalY;
}

//LeadのDebugログを出す
void UCameraLayer_Lead::DebugLeadLog(
	const ACharacter* Character,
	const FVector& TargetVelocityLead
) const
{
	if (!Character) return;
	if (!GEngine) return;

	//必要になったら一時的に解除
	//GEngine->AddOnScreenDebugMessage(11, 0.f, FColor::Cyan, FString::Printf(TEXT("[Lead] Speed: %.1f"), Character->GetVelocity().Size()));
	//GEngine->AddOnScreenDebugMessage(12, 0.f, FColor::Green, FString::Printf(TEXT("[Lead] Right: %.1f Target: %.1f"), m_SmoothedVelocityLead.Y, TargetVelocityLead.Y));
	//GEngine->AddOnScreenDebugMessage(13, 0.f, FColor::Yellow, FString::Printf(TEXT("[Lead] Forward: %.1f Target: %.1f"), m_SmoothedVelocityLead.X, TargetVelocityLead.X));
}

//移動入力があるか
bool UCameraLayer_Lead::HasMoveInput(
	const UCharacterMovementComponent* MovementComponent,
	const UCameraParameter* CameraParam
) const
{
	if (!MovementComponent) return false;
	if (!CameraParam) return false;

	FVector CurrentAcceleration =
		MovementComponent->GetCurrentAcceleration();

	CurrentAcceleration.Z = 0.f;

	return CurrentAcceleration.SizeSquared() >
		FMath::Square(CameraParam->LeadInputAccelerationThreshold);
}
#include "CameraActionHandler.h"
#include "../../../PlayerCharacter.h"
#include "../../Player_MovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"

void UCameraActionHandler::Initialize(APlayerCharacter* InPlayer, APlayerController* InController,
	USpringArmComponent* InSpringArm, UCameraComponent* InCamera,
	UPlayer_MovementComponent* InMovementComp)
{
	m_Player = InPlayer;
	m_PlayerController = InController;
	m_SpringArm = InSpringArm;
	m_Camera = InCamera;
	m_MovementComponent = InMovementComp;

	m_AutoFramingOffset = 0.f;

	// デフォルト値（SetupParamsが呼ばれなかった場合の保険）
	p_DefaultArmLength = 300.f;
	p_DefaultFOV = 90.f;
}

void UCameraActionHandler::SetupParams(float InDefaultLength, float InDefaultFOV,
	float InDashStartLength, float InDashStartFOV,
	float InDashMidLength, float InDashMidFOV)
{
	p_DefaultArmLength = InDefaultLength;
	p_DefaultFOV = InDefaultFOV;
	p_DashStartArmLength = InDashStartLength;
	p_DashStartFOV = InDashStartFOV;
	p_DashMidArmLength = InDashMidLength;
	p_DashMidFOV = InDashMidFOV;
}

void UCameraActionHandler::UpdateActionCamera(float DeltaTime)
{
	if (!m_MovementComponent || !m_SpringArm || !m_Camera) return;

	// --- 目標値の設定 ---
	float BaseTargetArmLength = p_DefaultArmLength;
	float TargetFOV = p_DefaultFOV;

	if (m_MovementComponent->GetIsDash())
	{
		if (m_MovementComponent->GetStartDash())
		{
			// ダッシュ開始時
			BaseTargetArmLength = p_DashStartArmLength;
			TargetFOV = p_DashStartFOV;

			// 初速寄せ完了判定（副作用：MovementComponentの状態を変更）
			if (FMath::IsNearlyEqual(m_SpringArm->TargetArmLength, BaseTargetArmLength, 1.f))
			{
				m_MovementComponent->SetStartDash(false);
			}
		}
		else
		{
			// 通常ダッシュ中
			BaseTargetArmLength = p_DashMidArmLength;
			TargetFOV = p_DashMidFOV;
		}
	}

	// オートフレーミング値取得
	float FramingOffset = GetAutoFramingOffset(DeltaTime);

	// 最終的な目標距離
	float FinalTagetLength = BaseTargetArmLength + FramingOffset;

	// 補間速度の決定
	float InterpSpeed = 10.f;

	if (FinalTagetLength > m_SpringArm->TargetArmLength)
	{
		// 遠ざかる時は速く
		InterpSpeed = 15.f;
	}
	else
	{
		// 近づく時
		if (FramingOffset < 10.f && m_MovementComponent->GetIsDash())
		{
			InterpSpeed = 8.f;
		}
		else
		{
			InterpSpeed = 5.f;
		}
	}

	if (m_MovementComponent->GetStartDash())
	{
		InterpSpeed = 20.f;
	}

	// --- 適用 ---
	m_SpringArm->TargetArmLength = FMath::FInterpTo(
		m_SpringArm->TargetArmLength,
		FinalTagetLength,
		DeltaTime,
		InterpSpeed
	);

	m_Camera->SetFieldOfView(FMath::FInterpTo(
		m_Camera->FieldOfView,
		TargetFOV,
		DeltaTime,
		InterpSpeed
	));
}

float UCameraActionHandler::GetAutoFramingOffset(float DeltaTime)
{
	if (!m_PlayerController || !m_Player) return 0.f;

	FVector PlayerLocation = m_Player->GetActorLocation();
	FVector2D ScreenLocation;
	bool bOnScreen = m_PlayerController->ProjectWorldLocationToScreen(PlayerLocation, ScreenLocation);

	int32 SizeX, SizeY;
	m_PlayerController->GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0) return m_AutoFramingOffset;

	// 正規化 (0.0 ~ 1.0)
	float NormX = ScreenLocation.X / (float)SizeX;
	float NormY = ScreenLocation.Y / (float)SizeY;

	// 中心(0.5)からの距離
	float DistX = FMath::Abs(NormX - 0.5f);
	float DistY = FMath::Abs(NormY - 0.5f);
	float MaxDist = FMath::Max(DistX, DistY);

	float TargetOffset = 0.f;

	// 画面端判定
	if (!bOnScreen || MaxDist > c_FramingSafeZone)
	{
		if (!bOnScreen)
		{
			TargetOffset = c_MaxFramingZoom;
		}
		else
		{
			TargetOffset = FMath::GetMappedRangeValueClamped(
				FVector2D(c_FramingSafeZone, 0.45),
				FVector2D(0.f, c_MaxFramingZoom),
				MaxDist
			);
		}
	}

	return TargetOffset;
}
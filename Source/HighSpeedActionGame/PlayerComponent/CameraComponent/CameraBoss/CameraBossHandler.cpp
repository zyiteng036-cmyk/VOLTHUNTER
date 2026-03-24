// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraBossHandler.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

void UCameraBossHandler::Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	SpringArm = InSpringArm;
	Camera = InCamera;
	m_IsActive = false;
}

void UCameraBossHandler::StartBossCamera()
{
	if (!SpringArm || !Camera)return;

	m_IsActive = true;

	//Œ»Ý‚Ì’l‚ð•ÛŽ
	m_DefaultArmLength = SpringArm->TargetArmLength;
	m_DefaultFOV = Camera->FieldOfView;
}

void UCameraBossHandler::EndBossCamera()
{
	m_IsActive = false;
}

bool UCameraBossHandler::UpdateBossCamera(float DeltaTime)
{
	if (!m_IsActive || !SpringArm || !Camera)return false;

	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength,
		m_BossArmLength,
		DeltaTime,
		m_TransitionSpeed
	);

	Camera->SetFieldOfView(FMath::FInterpTo(
		Camera->FieldOfView,
		m_BossFOV,
		DeltaTime,
		m_TransitionSpeed
	));

	return true;
}

bool UCameraBossHandler::IsActive()const {
	return m_IsActive;
}

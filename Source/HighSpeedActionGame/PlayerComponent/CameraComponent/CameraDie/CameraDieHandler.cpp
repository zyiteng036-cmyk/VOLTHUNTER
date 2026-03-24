// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraDieHandler.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"

void UCameraDieHandler::Initialize(USpringArmComponent* InSpringArm, APlayerController* InController)
{
	SpringArm = InSpringArm;
	PlayerController = InController;
}

void UCameraDieHandler::StartDieCamera()
{
	if (!SpringArm || !PlayerController) return;

	m_IsActive = true;

	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;

	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	// 距離を瞬時に変更
	SpringArm->TargetArmLength += m_DieZoomOutDistance;

	// 角度を直接 SpringArm にセットする（ControlRotation 経由ではなく直接）
	FRotator DieRot = PlayerController->GetControlRotation();
	DieRot.Pitch = m_TargetPitch;

	SpringArm->SetRelativeRotation(DieRot);
}

void UCameraDieHandler::EndDieCamera()
{
	// アクティブじゃなければ何もしない
	if (!m_IsActive) return;

	m_IsActive = false;
}


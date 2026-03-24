// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraOverrideHandler.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

void UCameraOverrideHandler::Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	TargetSpringArm = InSpringArm;
	TargetCamera = InCamera;
}

void UCameraOverrideHandler::StartOverride(float TargetLength, float TargetFOV, float Speed)
{
	m_IsActive = true;
	m_InterpSpeed = Speed;

	// 値のバリデーション（-1なら変更なし）
	if (TargetLength >= 0.0f)
	{
		m_TargetArmLength = TargetLength;
		m_OverrideLength = true;
	}
	else
	{
		m_OverrideLength = false;
	}

	if (TargetFOV > 0.0f)
	{
		m_TargetFOV = TargetFOV;
		m_OverrideFOV = true;
	}
	else
	{
		m_OverrideFOV = false;
	}
}

void UCameraOverrideHandler::EndOverride()
{
	m_IsActive = false;
	// ここでパラメータをリセットする必要はない
	// （呼び出し元のコンポーネントが次のフレームで通常処理に戻るため）
}

bool UCameraOverrideHandler::UpdateOverride(float DeltaTime)
{
	if (!m_IsActive || !TargetSpringArm || !TargetCamera)
	{
		return false;
	}

	// 距離の補間
	if (m_OverrideLength)
	{
		float NewLen = FMath::FInterpTo(TargetSpringArm->TargetArmLength, m_TargetArmLength, DeltaTime, m_InterpSpeed);
		TargetSpringArm->TargetArmLength = NewLen;
	}

	// FOVの補間
	if (m_OverrideFOV)
	{
		float NewFOV = FMath::FInterpTo(TargetCamera->FieldOfView, m_TargetFOV, DeltaTime, m_InterpSpeed);
		TargetCamera->SetFieldOfView(NewFOV);
	}

	return true;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraOpeningHandler.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

void UCameraOpeningHandler::Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera, APlayerController* InPC)
{
	SpringArm = InSpringArm;
	Camera = InCamera;
	PlayerController = InPC;
	m_IsActive = false;
}

void UCameraOpeningHandler::StartOpening(float WaitTime, float MoveDuration, float FaceDistance, float StartYawOffset)
{
	if (!SpringArm || !PlayerController)return;

	m_IsActive = true;
	m_CurrentTime = 0.0;
	// 時間を保存
	m_WaitTime = WaitTime;         // 待機時間
	m_MoveDuration = MoveDuration; // 移動にかける時間
	m_StartYawOffset = StartYawOffset;
	m_IsUiHidden = false;

	//UIの生成と表示
	if (OpeningWidgetClass && PlayerController)
	{
		// 既存があれば念の為消す
		if (m_CreatedWidget)
		{
			m_CreatedWidget->RemoveFromParent();
			m_CreatedWidget = nullptr;
		}

		// 生成 (CreateWidget)
		m_CreatedWidget = CreateWidget<UUserWidget>(PlayerController, OpeningWidgetClass);

		// 画面に追加 (AddToViewport)
		if (m_CreatedWidget)
		{
			m_CreatedWidget->AddToViewport();
		}
	}

	//目標値（通常プレイ状態）を保存
	m_InitialArmLength = SpringArm->TargetArmLength;
	m_InitialSocketOffset = SpringArm->SocketOffset;
	m_InitialRotation = PlayerController->GetControlRotation();

	//開始位置
	m_StartArmLength = FaceDistance;

	//顔の高さへ
	m_FaceSocketOffset = m_InitialSocketOffset;
	m_FaceSocketOffset.Z = 70.f;

	//カメラを開始位置へ
	FRotator StartRotation = m_InitialRotation;
	StartRotation.Yaw += m_StartYawOffset;
	StartRotation.Pitch = -10.f;

	PlayerController->SetControlRotation(StartRotation);
	SpringArm->TargetArmLength = m_StartArmLength;
	SpringArm->SocketOffset = m_FaceSocketOffset;


}

bool UCameraOpeningHandler::UpdateOpening(float DeltaTime)
{
	if (!m_IsActive || !SpringArm || !PlayerController)return false;

	m_CurrentTime += DeltaTime;

	if (m_CurrentTime < m_WaitTime)
	{
		return true;
	}

	// 経過時間から待機時間を引いて、移動に使った時間出す
	float MoveTime = m_CurrentTime - m_WaitTime;

	if (!m_IsUiHidden && m_CreatedWidget)
	{
		// フェードにかける時間
		float FadeDuration = 0.5f;

		// 「消える時間」のどれくらい手前か
		// RemainingTime が 0.5 以下になったらフェード開始
		float RemainingTime = UiHideDelayTime - MoveTime;

		if (RemainingTime <= 0.0f)
		{
			// 時間切れ：完全に削除
			m_CreatedWidget->RemoveFromParent();
			m_CreatedWidget = nullptr;
			m_IsUiHidden = true;
		}
		else if (RemainingTime <= FadeDuration)
		{
			// フェード中：透明度を計算 (1.0 -> 0.0)
			float Opacity = RemainingTime / FadeDuration;
			m_CreatedWidget->SetRenderOpacity(Opacity);
		}
		else
		{
			// まだ余裕がある：不透明(1.0)を維持
			m_CreatedWidget->SetRenderOpacity(1.0f);
		}
	}
	// Alphaの計算 (分母は動作時間)
	float Alpha = FMath::Clamp(MoveTime / m_MoveDuration, 0.f, 1.f);

	//イージング関数でなめらか
	float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 4.0f);

	//距離の補間
	float NewArmLength = FMath::Lerp(m_StartArmLength, m_InitialArmLength, SmoothAlpha);
	SpringArm->TargetArmLength = NewArmLength;

	//高さの補間
	FVector NewSocketOffset = FMath::Lerp(m_FaceSocketOffset, m_InitialSocketOffset, SmoothAlpha);
	SpringArm->SocketOffset = NewSocketOffset;

	//回転の補間
	FRotator StartRotation = m_InitialRotation;
	StartRotation.Yaw += m_StartYawOffset;
	StartRotation.Pitch = -10.f;

	FRotator NewRotation = FMath::Lerp(StartRotation, m_InitialRotation, SmoothAlpha);
	PlayerController->SetControlRotation(NewRotation);

	//終了判定
	if (Alpha >= 1.f)
	{
		StopOpening();
		return false;
	}
	return true;
}

void UCameraOpeningHandler::StopOpening()
{
	m_IsActive = false;

	if (m_CreatedWidget)
	{
		m_CreatedWidget->RemoveFromParent();
		m_CreatedWidget = nullptr;
	}

	//最終値にセット
	if (SpringArm)
	{
		SpringArm->TargetArmLength = m_InitialArmLength;
		SpringArm->SocketOffset = m_InitialSocketOffset;
	}
	if (PlayerController)
	{
		PlayerController->SetControlRotation(m_InitialRotation);
	}
}

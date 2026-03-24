// Fill out your copyright notice in the Description page of Project Settings.


#include "TitlePlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "../Widget/TitleModeSelectWidge.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../Widget/SettingsWidget.h"

namespace {
	constexpr int8 MaxMenuWindowNum = 3;
}

void ATitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

		if (Subsystem)
		{
			Subsystem->AddMappingContext(TitleMappingContext, 0);
		}
	}

	// ウィジェット生成＆表示
	if (ModeSelectWidgetClass)
	{
		ModeSelectWidget = CreateWidget<UTitleModeSelectWidge>(this, ModeSelectWidgetClass);
		if (ModeSelectWidget)
		{
			ModeSelectWidget->AddToViewport();
			IsModeSelectOpen = false;
		}
	}
}
void ATitlePlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIComp = CastChecked<UEnhancedInputComponent>(InputComponent);

	EIComp->BindAction(m_DecisionAction, ETriggerEvent::Started, this, &ATitlePlayerController::DecisionAction);
	EIComp->BindAction(m_ReturnAction, ETriggerEvent::Started, this, &ATitlePlayerController::ReturnAction);
	EIComp->BindAction(m_MoveUpSelectionAction, ETriggerEvent::Started, this, &ATitlePlayerController::MoveUpSelection);
	EIComp->BindAction(m_MoveDownSelectionAction, ETriggerEvent::Started, this, &ATitlePlayerController::MoveDownSelection);
	EIComp->BindAction(m_MoveRightSelectionAction, ETriggerEvent::Triggered, this, &ATitlePlayerController::MoveRightSelection);
	EIComp->BindAction(m_MoveLeftSelectionAction, ETriggerEvent::Triggered, this, &ATitlePlayerController::MoveLeftSelection);

	EIComp->BindAction(m_MoveRightSelectionAction, ETriggerEvent::Completed, this, &ATitlePlayerController::ResetSettingInput);
	EIComp->BindAction(m_MoveLeftSelectionAction, ETriggerEvent::Completed, this, &ATitlePlayerController::ResetSettingInput);
}

void ATitlePlayerController::DecisionAction(const FInputActionValue& Value)
{
	if (m_IsSettingOpen && SettingsWidget) {
		//アニメーション中は操作させない
		if (SettingsWidget->GetIsAnimationPlaying()) return;

		//決定ボタンの処理を設定ウィジェットに伝える
		SettingsWidget->DecisionCurrentSetting();
		return;
	}
	if (ModeSelectWidget) {
		if (!ModeSelectWidget->GetIsAnimationPlaying() && !IsModeSelectOpen) {
			ETitleMenuInputType Type = ETitleMenuInputType::Decision;
			ModeSelectWidget->NotifyInput(Type, m_CurrentIndex);
			IsModeSelectOpen = true;
			return;
		}
		else if (MaxMenuWindowNum == m_CurrentIndex) {
			APlayerController* PC = this;
			if (PC) {
				UKismetSystemLibrary::QuitGame(PC, PC, EQuitPreference::Quit, false);
			}
		}
		else if (!ModeSelectWidget->GetIsAnimationPlaying() && IsModeSelectOpen) {
			if (m_CurrentIndex == 2)
			{
				if (SettingsWidgetClass)
				{
					if (!SettingsWidget)
					{
						SettingsWidget = CreateWidget<USettingsWidget>(this, SettingsWidgetClass);
						//アニメーションの終了通知
						SettingsWidget->OnSettingsClosedDelegate.AddDynamic(this, &ATitlePlayerController::OnSettingsWidgetClosed);
					}
					if (SettingsWidget && !SettingsWidget->IsInViewport()) {
						SettingsWidget->AddToViewport();
						m_IsSettingOpen = true; 

						
						FInputModeGameAndUI InputMode;
						InputMode.SetWidgetToFocus(SettingsWidget->TakeWidget());
						SetInputMode(InputMode);

						//アニメーション開始
						SettingsWidget->PlayOpenAnimation();
					}
				}
			}
			
			else if (m_CurrentIndex == 0 || m_CurrentIndex == 1) {
				if (!LevelAssetToLoad[m_CurrentIndex].IsNull()) {
					UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelAssetToLoad[m_CurrentIndex]);
				}
			}
		}
	}
}

void ATitlePlayerController::ReturnAction(const FInputActionValue& Value) {
	//設定画面が開いていれば閉じる
	if (m_IsSettingOpen && SettingsWidget)
	{
		if (SettingsWidget->GetIsAnimationPlaying()) return;
		SettingsWidget->PlayCloseAnimation();
		return;
	}
	
	if (ModeSelectWidget) {
		if (!ModeSelectWidget->GetIsAnimationPlaying() && IsModeSelectOpen) {
			ETitleMenuInputType Type = ETitleMenuInputType::Cancel;
			ModeSelectWidget->NotifyInput(Type, m_CurrentIndex);
			IsModeSelectOpen = false;
		}

	}
}

void ATitlePlayerController::MoveUpSelection(const FInputActionValue& Value) {
	if (m_IsSettingOpen && SettingsWidget) {
		if (SettingsWidget->GetIsAnimationPlaying()) return;
		SettingsWidget->ChangeSelection(-1);
		return;
	}
	
	if (!IsModeSelectOpen && ModeSelectWidget->GetIsAnimationPlaying())return;

	--m_CurrentIndex;

	if (m_CurrentIndex < 0) {
		m_CurrentIndex = MaxMenuWindowNum;
	}
	ETitleMenuInputType Type = ETitleMenuInputType::MoveUpSelection;
	ModeSelectWidget->NotifyInput(Type, m_CurrentIndex);

}
void ATitlePlayerController::MoveDownSelection(const FInputActionValue& Value) {
	if (m_IsSettingOpen && SettingsWidget) {
		if (SettingsWidget->GetIsAnimationPlaying()) return;
		SettingsWidget->ChangeSelection(1);
		return;
	}
	
	if (!IsModeSelectOpen && ModeSelectWidget->GetIsAnimationPlaying())return;

	++m_CurrentIndex;
	if (m_CurrentIndex > MaxMenuWindowNum) {
		m_CurrentIndex = 0;
	}
	ETitleMenuInputType Type = ETitleMenuInputType::MoveDownSelection;
	ModeSelectWidget->NotifyInput(Type, m_CurrentIndex);

}

void ATitlePlayerController::MoveRightSelection(const FInputActionValue& Value)
{
	if (m_IsSettingOpen && SettingsWidget) {
		if (SettingsWidget->GetIsAnimationPlaying()) return;
		SettingsWidget->ChangeValue(1.0f, GetWorld()->GetDeltaSeconds());
	}
}

void ATitlePlayerController::MoveLeftSelection(const FInputActionValue& Value)
{
	if (m_IsSettingOpen && SettingsWidget) {
		if (SettingsWidget->GetIsAnimationPlaying()) return;
		SettingsWidget->ChangeValue(-1.0f, GetWorld()->GetDeltaSeconds());
	}
}

void ATitlePlayerController::ResetSettingInput(const FInputActionValue& Value)
{
	if (m_IsSettingOpen && SettingsWidget) {
		SettingsWidget->ResetInputState();
	}
}

void ATitlePlayerController::OnSettingsWidgetClosed()
{
	if (SettingsWidget)
	{
		SettingsWidget->RemoveFromParent();
	}
	m_IsSettingOpen = false;

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

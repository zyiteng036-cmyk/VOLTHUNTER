// Fill out your copyright notice in the Description page of Project Settings.


#include "ResultPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "../Widget/ResultWidget.h"

void AResultPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer)
	{
		auto* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

		if (Subsystem)
		{
			Subsystem->AddMappingContext(ResultMappingContext, 0);
		}
	}

	// ウィジェット生成＆表示
	if (ResultWidgetClass)
	{
		ResultWidget = CreateWidget<UResultWidget>(this, ResultWidgetClass);
		if (ResultWidget)
		{
			ResultWidget->AddToViewport();
		}
	}
}
void AResultPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIComp = CastChecked<UEnhancedInputComponent>(InputComponent);

	EIComp->BindAction(m_DecisionAction, ETriggerEvent::Triggered, this, &AResultPlayerController::DecisionAction);
	EIComp->BindAction(m_MoveUpSelectionAction, ETriggerEvent::Started, this, &AResultPlayerController::MoveUpSelection);
	EIComp->BindAction(m_MoveDownSelectionAction, ETriggerEvent::Started, this, &AResultPlayerController::MoveDownSelection);

}

void AResultPlayerController::DecisionAction(const FInputActionValue& Value) {

}
void AResultPlayerController::MoveUpSelection(const FInputActionValue& Value) {

}
void AResultPlayerController::MoveDownSelection(const FInputActionValue& Value) {

}



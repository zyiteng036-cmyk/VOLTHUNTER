// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ResultPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UResultWidget;
/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API AResultPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void DecisionAction(const FInputActionValue& Value);
	void MoveUpSelection(const FInputActionValue& Value);
	void MoveDownSelection(const FInputActionValue& Value);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* ResultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_DecisionAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_MoveUpSelectionAction;


	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_MoveDownSelectionAction;


	UPROPERTY()
	UResultWidget* ResultWidget = nullptr;  // ウィジェット保持用

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UResultWidget> ResultWidgetClass;
};

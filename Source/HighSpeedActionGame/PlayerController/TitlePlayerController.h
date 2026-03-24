// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitlePlayerController.generated.h"

class UTitleModeSelectWidge;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class USettingsWidget;
/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Title")
	void SetIsCanControl(const bool _isCanControl) { m_IsCanControl = _isCanControl; }
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void DecisionAction(const FInputActionValue& Value);
	void ReturnAction(const FInputActionValue& Value);
	void MoveUpSelection(const FInputActionValue& Value);
	void MoveDownSelection(const FInputActionValue& Value);
	void MoveRightSelection(const FInputActionValue& Value);
	void MoveLeftSelection(const FInputActionValue& Value);

	//ボタンを離したときの処理
	void ResetSettingInput(const FInputActionValue& Value);
	//アニメーション終了時
	UFUNCTION()
	void OnSettingsWidgetClosed();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* TitleMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_DecisionAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_ReturnAction;


	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_MoveUpSelectionAction;


	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_MoveDownSelectionAction;

	UPROPERTY(EditAnywhere,Category = "Input")
	UInputAction* m_MoveRightSelectionAction;

	UPROPERTY(EditAnywhere,Category = "Input")
	UInputAction* m_MoveLeftSelectionAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<USettingsWidget> SettingsWidgetClass;


	bool IsModeSelectOpen = false;

	bool m_IsCanControl = false;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	int32 m_CurrentIndex = 0;

	UPROPERTY()
	UTitleModeSelectWidge* ModeSelectWidget = nullptr;  // ウィジェット保持用

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTitleModeSelectWidge> ModeSelectWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Level")
	TArray<TSoftObjectPtr<UWorld>> LevelAssetToLoad;

private:
	UPROPERTY()
	class USettingsWidget* SettingsWidget;

	bool m_IsSettingOpen = false;
};

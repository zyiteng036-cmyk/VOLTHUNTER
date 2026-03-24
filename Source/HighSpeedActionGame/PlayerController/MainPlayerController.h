//担当
//伊藤直樹

//インゲーム内でコントローラーを管理するクラス

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

class UTitleModeSelectWidge;
class UInputMappingContext;
class UInputAction;
class UResultWidget;
struct FInputActionValue;


UENUM(BlueprintType)
enum class EPlayerInputMode : uint8
{
	Gameplay,
	Result
};

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent()override;

public:
	//Resultモード切替
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void EnterResultMode();
	void ExitResultMode();

private:
	void SetInputModeState(EPlayerInputMode NewMode);

protected:
	void ResultDecision(const FInputActionValue& Value);
	void ResultMoveUp(const FInputActionValue& Value);
	void ResultMoveDown(const FInputActionValue& Value);

protected:
	// 通常ゲーム用 InputMappingContext
	// PlayerCharacter 側の操作で使用する
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* GameplayMappingContext;

	// Result 画面用 InputMappingContext
	// メニュー操作専用
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* ResultMappingContext;


	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ResultDecisionAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ResultMoveUpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ResultMoveDownAction;

	UPROPERTY()
	EPlayerInputMode CurrentInputMode = EPlayerInputMode::Gameplay;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Result")
	TSubclassOf<UResultWidget> ResultWidgetClass;

	UPROPERTY()
	UResultWidget* ResultWidget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Level")
	TArray<TSoftObjectPtr<UWorld>> LevelAssetToLoad;


	int32 ResultCurrentIndex = 0;

	bool bIsResultConfirmed = false;

private:
	//アニメーションが終わってから入力できる
	UPROPERTY()
	bool bIsResultInputActive = false;

	FTimerHandle ResultIntroTimerHandle;
};

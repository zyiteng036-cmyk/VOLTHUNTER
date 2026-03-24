// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "../Widget/ResultWidget.h"
namespace {
	constexpr int8 MaxResultMenuIndex = 1;
	constexpr float ResultAnimationDuration = 9.0f; 
}



void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputModeState(EPlayerInputMode::Gameplay);
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Result 用入力のみ PlayerController 側でバインドする
	// Gameplay 入力は PlayerCharacter 側で管理するため、ここでは扱わない
	UEnhancedInputComponent* EIComp = CastChecked<UEnhancedInputComponent>(InputComponent);

	EIComp->BindAction(ResultDecisionAction, ETriggerEvent::Triggered,
		this, &AMainPlayerController::ResultDecision);

	EIComp->BindAction(ResultMoveUpAction, ETriggerEvent::Started,
		this, &AMainPlayerController::ResultMoveUp);

	EIComp->BindAction(ResultMoveDownAction, ETriggerEvent::Started,
		this, &AMainPlayerController::ResultMoveDown);
}
void AMainPlayerController::EnterResultMode()
{
	if (CurrentInputMode == EPlayerInputMode::Result) return;

	SetInputModeState(EPlayerInputMode::Result);

	// Result 画面を開いた直後は先頭項目を選択状態にする
	ResultCurrentIndex = 0;
	bIsResultInputActive = false;

	GetWorldTimerManager().SetTimer(
		ResultIntroTimerHandle,
		[this]()
		{
			// PCが死んでいないか、Widgetがまだあるかチェック
			if (!IsValid(this) || !ResultWidget) return;

			// 入力解禁
			bIsResultInputActive = true;

			ResultWidget->NotifyInput(EResultMenuInputType::MoveDownSelection, ResultCurrentIndex);

			UE_LOG(LogTemp, Log, TEXT("Result UI Input Activated"));
		},
		ResultAnimationDuration, // 定義した秒数待つ
		false
	);
}

void AMainPlayerController::ExitResultMode()
{
	if (CurrentInputMode == EPlayerInputMode::Gameplay)
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(ResultIntroTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(ResultIntroTimerHandle);
	}

	SetInputModeState(EPlayerInputMode::Gameplay);
}

void AMainPlayerController::SetInputModeState(EPlayerInputMode NewMode)
{
	// 既に同じモードなら処理しない
	if (CurrentInputMode == NewMode)
	{
		return;
	}

	CurrentInputMode = NewMode;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	// MappingContext は必ず一度全解除する
	Subsystem->ClearAllMappings();

	if (NewMode == EPlayerInputMode::Gameplay)
	{
		// Gameplay 入力のみ有効
		Subsystem->AddMappingContext(GameplayMappingContext, 0);

		// Result UI を閉じる
		if (ResultWidget)
		{
			ResultWidget->RemoveFromParent();
			ResultWidget = nullptr;
		}

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
	else
	{
		// Result 入力のみ有効
		Subsystem->AddMappingContext(ResultMappingContext, 0);

		if (ResultWidgetClass)
		{
			ResultWidget = CreateWidget<UResultWidget>(this, ResultWidgetClass);
			ResultWidget->AddToViewport(200);

		}

		FInputModeGameOnly InputMode;

		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}

void AMainPlayerController::ResultDecision(const FInputActionValue& Value)
{
	// Result モード以外では処理しない
	if (CurrentInputMode != EPlayerInputMode::Result) return;

	if (!ResultWidget) return;

	if (!bIsResultInputActive) return;

	// アニメーション再生中は入力を受け付けない
	if (ResultWidget->GetIsAnimationPlaying()) return;

	// 決定入力を Widget に通知する
	// Controller は「何が押されたか」と「どこを選んでいるか」だけを渡す
	ResultWidget->NotifyInput(EResultMenuInputType::Decision,ResultCurrentIndex);

	if (LevelAssetToLoad.IsValidIndex(ResultCurrentIndex)
		&& !LevelAssetToLoad[ResultCurrentIndex].IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this,LevelAssetToLoad[ResultCurrentIndex]);
	}
}

void AMainPlayerController::ResultMoveUp(const FInputActionValue& Value)
{
	if (CurrentInputMode != EPlayerInputMode::Result) return;
	if (!ResultWidget) return;

	if (!bIsResultInputActive) return;

	// アニメーション中はカーソル移動させない
	if (ResultWidget->GetIsAnimationPlaying()) return;

	--ResultCurrentIndex;

	// 範囲外に出た場合はループさせる
	if (ResultCurrentIndex < 0)
	{
		ResultCurrentIndex = MaxResultMenuIndex;
	}

	UE_LOG(LogTemp, Log, TEXT("ResultCurrentIndex = %d"), ResultCurrentIndex);

	ResultWidget->NotifyInput(EResultMenuInputType::MoveUpSelection, ResultCurrentIndex);
}

void AMainPlayerController::ResultMoveDown(const FInputActionValue& Value)
{
	if (CurrentInputMode != EPlayerInputMode::Result) return;
	if (!ResultWidget) return;

	if (!bIsResultInputActive) return;

	if (ResultWidget->GetIsAnimationPlaying()) return;

	++ResultCurrentIndex;

	if (ResultCurrentIndex > MaxResultMenuIndex)
	{
		ResultCurrentIndex = 0;
	}

	UE_LOG(LogTemp, Log, TEXT("ResultCurrentIndex = %d"), ResultCurrentIndex);

	ResultWidget->NotifyInput(EResultMenuInputType::MoveDownSelection, ResultCurrentIndex);
}


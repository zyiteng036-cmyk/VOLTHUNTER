#include "MainPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "../Widget/ResultWidget.h"

namespace MainPlayerControllerConstants
{
	constexpr int8 MaxResultMenuIndex = 1; //メニュー項目の最大インデックス
	constexpr float ResultAnimationDuration = 9.0f; //リザルト演出アニメーションの再生時間
	constexpr int32 DefaultMappingPriority = 0; //インプットマッピングのデフォルト優先度
	constexpr int32 ResultWidgetZOrder = 200; //リザルトUI表示時のZオーダー値
	constexpr int32 InitialMenuIndex = 0; //メニューの初期選択位置
}

//ゲーム開始時の処理
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//初期の入力モードをゲームプレイ用に設定
	SetInputModeState(EPlayerInputMode::Gameplay);
}

//入力コンポーネントの初期化
void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//Result用入力のみPlayerController側でバインドする
	//Gameplay入力はPlayerCharacter側で管理するため、ここでは扱わない
	UEnhancedInputComponent* EIComp = CastChecked<UEnhancedInputComponent>(InputComponent);

	//リザルト用のキー入力を各関数にバインド
	EIComp->BindAction(m_ResultDecisionAction, ETriggerEvent::Triggered, this, &AMainPlayerController::ResultDecision);
	EIComp->BindAction(m_ResultMoveUpAction, ETriggerEvent::Started, this, &AMainPlayerController::ResultMoveUp);
	EIComp->BindAction(m_ResultMoveDownAction, ETriggerEvent::Started, this, &AMainPlayerController::ResultMoveDown);
}

//リザルトモード開始
void AMainPlayerController::EnterResultMode()
{
	//既にリザルトモードなら何もしない
	if (m_CurrentInputMode == EPlayerInputMode::Result) return;

	//入力ステートをリザルトモードに移行
	SetInputModeState(EPlayerInputMode::Result);

	//Result画面を開いた直後は先頭項目を選択状態にする
	m_ResultCurrentIndex = MainPlayerControllerConstants::InitialMenuIndex;
	m_IsResultInputActive = false;

	//演出アニメーション終了後に操作を受け付けるためのタイマーを開始
	GetWorldTimerManager().SetTimer(
		m_ResultIntroTimerHandle,
		[this]()
		{
			//PCが死んでいないか、Widgetがまだあるかチェック
			if (!IsValid(this) || !m_ResultWidget) return;

			//入力解禁
			m_IsResultInputActive = true;

			//UI側に現在のフォーカス位置を通達
			m_ResultWidget->NotifyInput(EResultMenuInputType::MoveDownSelection, m_ResultCurrentIndex);
		},
		MainPlayerControllerConstants::ResultAnimationDuration, //定義した秒数待つ
		false
	);
}

//リザルトモード終了
void AMainPlayerController::ExitResultMode()
{
	//既にゲームプレイモードなら何もしない
	if (m_CurrentInputMode == EPlayerInputMode::Gameplay)
	{
		return;
	}

	//稼働中のタイマーがあればクリアする
	if (GetWorldTimerManager().IsTimerActive(m_ResultIntroTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(m_ResultIntroTimerHandle);
	}

	//ゲームプレイモードへ復帰
	SetInputModeState(EPlayerInputMode::Gameplay);
}

//入力モードの状態を設定
void AMainPlayerController::SetInputModeState(EPlayerInputMode NewMode)
{
	//既に同じモードなら処理しない
	if (m_CurrentInputMode == NewMode)
	{
		return;
	}

	m_CurrentInputMode = NewMode;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	//MappingContextは必ず一度全解除する
	Subsystem->ClearAllMappings();

	if (NewMode == EPlayerInputMode::Gameplay)
	{
		//Gameplay入力のみ有効
		Subsystem->AddMappingContext(m_GameplayMappingContext, MainPlayerControllerConstants::DefaultMappingPriority);

		//ResultUIを閉じる
		if (m_ResultWidget)
		{
			m_ResultWidget->RemoveFromParent();
			m_ResultWidget = nullptr;
		}

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
	else
	{
		//Result入力のみ有効
		Subsystem->AddMappingContext(m_ResultMappingContext, MainPlayerControllerConstants::DefaultMappingPriority);

		//UIを生成して画面に最前面で追加
		if (m_ResultWidgetClass)
		{
			m_ResultWidget = CreateWidget<UResultWidget>(this, m_ResultWidgetClass);
			if (m_ResultWidget)
			{
				m_ResultWidget->AddToViewport(MainPlayerControllerConstants::ResultWidgetZOrder);
			}
		}

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}

//決定入力時のコールバック
void AMainPlayerController::ResultDecision(const FInputActionValue& Value)
{
	//Resultモード以外では処理しない
	if (m_CurrentInputMode != EPlayerInputMode::Result) return;

	if (!m_ResultWidget) return;

	if (!m_IsResultInputActive) return;

	//アニメーション再生中は入力を受け付けない
	if (m_ResultWidget->GetIsAnimationPlaying()) return;

	//決定入力をWidgetに通知する
	//Controllerは「何が押されたか」と「どこを選んでいるか」だけを渡す
	m_ResultWidget->NotifyInput(EResultMenuInputType::Decision, m_ResultCurrentIndex);

	//選択中のインデックスに応じたレベルが登録されていればレベル遷移を行う
	if (m_LevelAssetToLoad.IsValidIndex(m_ResultCurrentIndex)
		&& !m_LevelAssetToLoad[m_ResultCurrentIndex].IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, m_LevelAssetToLoad[m_ResultCurrentIndex]);
	}
}

//上移動入力時のコールバック
void AMainPlayerController::ResultMoveUp(const FInputActionValue& Value)
{
	if (m_CurrentInputMode != EPlayerInputMode::Result) return;
	if (!m_ResultWidget) return;

	if (!m_IsResultInputActive) return;

	//アニメーション中はカーソル移動させない
	if (m_ResultWidget->GetIsAnimationPlaying()) return;

	--m_ResultCurrentIndex;

	//範囲外に出た場合はループさせる
	if (m_ResultCurrentIndex < MainPlayerControllerConstants::InitialMenuIndex)
	{
		m_ResultCurrentIndex = MainPlayerControllerConstants::MaxResultMenuIndex;
	}

	m_ResultWidget->NotifyInput(EResultMenuInputType::MoveUpSelection, m_ResultCurrentIndex);
}

//下移動入力時のコールバック
void AMainPlayerController::ResultMoveDown(const FInputActionValue& Value)
{
	if (m_CurrentInputMode != EPlayerInputMode::Result) return;
	if (!m_ResultWidget) return;

	if (!m_IsResultInputActive) return;

	if (m_ResultWidget->GetIsAnimationPlaying()) return;

	++m_ResultCurrentIndex;

	//上限を超えた場合はループさせる
	if (m_ResultCurrentIndex > MainPlayerControllerConstants::MaxResultMenuIndex)
	{
		m_ResultCurrentIndex = MainPlayerControllerConstants::InitialMenuIndex;
	}

	m_ResultWidget->NotifyInput(EResultMenuInputType::MoveDownSelection, m_ResultCurrentIndex);
}
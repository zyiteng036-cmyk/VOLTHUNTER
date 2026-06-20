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

//プレイヤー入力を制御する列挙型
UENUM(BlueprintType)
enum class EPlayerInputMode : uint8
{
	Gameplay,
	Result
};

UCLASS()
class HIGHSPEEDACTIONGAME_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	//ゲーム開始時の初期化
	virtual void BeginPlay() override;

	//入力コンポーネントのセットアップ
	virtual void SetupInputComponent() override;

public:
	//Resultモード切替
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void EnterResultMode();

	//Resultモード終了
	void ExitResultMode();

private:
	//入力モードの状態を設定
	void SetInputModeState(EPlayerInputMode NewMode);

protected:
	//入力コールバック
	void ResultDecision(const FInputActionValue& Value);
	void ResultMoveUp(const FInputActionValue& Value);
	void ResultMoveDown(const FInputActionValue& Value);

protected:
	//通常ゲーム用InputMappingContext
	//PlayerCharacter側の操作で使用する
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* m_GameplayMappingContext = nullptr;

	//Result画面用InputMappingContext
	//メニュー操作専用
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* m_ResultMappingContext = nullptr;

	//決定アクション
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_ResultDecisionAction = nullptr;

	//上移動アクション
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_ResultMoveUpAction = nullptr;

	//下移動アクション
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* m_ResultMoveDownAction = nullptr;

	//現在の入力モード
	UPROPERTY()
	EPlayerInputMode m_CurrentInputMode = EPlayerInputMode::Gameplay;

	//生成するリザルトUIのクラス
	UPROPERTY(EditDefaultsOnly, Category = "UI|Result")
	TSubclassOf<UResultWidget> m_ResultWidgetClass = nullptr;

	//生成されたリザルトUIのインスタンス
	UPROPERTY()
	UResultWidget* m_ResultWidget = nullptr;

	//ロード対象のレベル資産リスト
	UPROPERTY(EditAnywhere, Category = "Level")
	TArray<TSoftObjectPtr<UWorld>> m_LevelAssetToLoad;

	//現在の選択インデックス
	int32 m_ResultCurrentIndex = 0;

	//リザルト画面で決定されたかどうかのフラグ
	bool m_IsResultConfirmed = false;

private:
	//アニメーションが終わってから入力できる
	UPROPERTY()
	bool m_IsResultInputActive = false;

	//リザルト演出用のタイマーハンドル
	FTimerHandle m_ResultIntroTimerHandle;
};
//担当
//伊藤直樹

//ゲーム内のカメラ感度、反転設定、画面の明るさなどのオプションを管理・変更する設定画面のUIウィジェットクラス
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class USlider;
class UTextBlock;
class UCheckBox;

//アニメーションが終了したときのデリゲート
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsClosedSignature);

UCLASS()
class HIGHSPEEDACTIONGAME_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//ウィジェット構築時の初期化処理
	virtual void NativeConstruct() override;

public:
	//上下入力で選択項目を変更する
	void ChangeSelection(int32 Direction);

	//左右入力でパラメーターの値を変更する 
	void ChangeValue(float Direction, float DeltaTime);

	//決定ボタンが押された時
	void DecisionCurrentSetting();

protected:
	//選択状態に応じたUIの見た目を更新するBlueprintイベント
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateSelectionUI();

	//アニメーションが終了したら自動で呼ばれる
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

public:
	//連続入力状態のリセット
	void ResetInputState();

	//開くアニメーションの再生
	void PlayOpenAnimation();

	//閉じるアニメーションの再生
	void PlayCloseAnimation();

	//アニメーションが再生中かどうかを取得
	bool GetIsAnimationPlaying() const;

	//コントローラーに通知するためのイベント変数
	UPROPERTY(BlueprintAssignable)
	FOnSettingsClosedSignature m_OnSettingsClosedDelegate;

public:
	//カメラ感度スライダー
	UPROPERTY(meta = (BindWidget))
	USlider* m_CameraSensitivitySlider = nullptr;

	//カメラ感度テキスト
	UPROPERTY(meta = (BindWidget))
	UTextBlock* m_CameraSensitivityText = nullptr;

	//カメラX軸反転チェックボックス
	UPROPERTY(meta = (BindWidget))
	UCheckBox* m_CameraInvertXCheckBox = nullptr;

	//カメラX軸反転テキスト
	UPROPERTY(meta = (BindWidget))
	UTextBlock* m_CameraInvertXText = nullptr;

	//カメラY軸反転チェックボックス
	UPROPERTY(meta = (BindWidget))
	UCheckBox* m_CameraInvertYCheckBox = nullptr;

	//カメラY軸反転テキスト
	UPROPERTY(meta = (BindWidget))
	UTextBlock* m_CameraInvertYText = nullptr;

	//画面の明るさスライダー
	UPROPERTY(meta = (BindWidget))
	USlider* m_ScreenBrightnessSlider = nullptr;

	//オープンアニメーション
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* m_OpenAnim = nullptr;

	//クローズアニメーション
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* m_CloseAnim = nullptr;

protected:
	//現在選択している設定項目のインデックス
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	int32 m_CurrentSettingIndex = 0;

	//設定項目の最大インデックス
	int32 m_MaxSettingIndex = 4;

private:
	//デフォルト値になった時に硬直するタイマー
	float m_StopCooldown = 0.0f;

	//硬直させる元の値
	float m_DefaultSensitivity = 0.9f;

	//入力時間を記録
	float m_LastInputTime = 0.0f;

	//最初の一回目かどうかを判定
	bool m_IsFirstPress = true;

	//明るさのデフォルト値
	float m_DafultBrightness = 2.2f;
};
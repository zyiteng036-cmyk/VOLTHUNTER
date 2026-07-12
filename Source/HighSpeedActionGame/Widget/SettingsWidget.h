//担当
//伊藤直樹

//設定画面ウィジェットクラス
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class USlider;
class UTextBlock;
class UCheckBox;

//アニメーションが終了したときのデリゲート
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsClosedSignature);
/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct()override;

public:
	//上下入力で選択項目を変更する
	void ChangeSelection(int32 Direction);

	//左右入力でパラメーターの値を変更する 
	void ChangeValue(float Direction, float DeltaTime);

	//決定ボタンが押された時
	void DecisionCurrentSetting();
protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateSelectionUI();

	//アニメーションが終了したら自動で呼ばれる
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
public:
	void ResetInputState();

	void PlayOpenAnimation();
	void PlayCloseAnimation();
	bool GetIsAnimationPlaying() const;

	// コントローラーに通知するためのイベント変数
	UPROPERTY(BlueprintAssignable)
	FOnSettingsClosedSignature OnSettingsClosedDelegate;
public:
	//カメラ感度
	UPROPERTY(meta = (BindWidget))
	USlider* CameraSensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CameraSensitivityText;

	//カメラ反転
	UPROPERTY(meta = (BindWidget))
	UCheckBox* CameraInvertXCheckBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CameraInvertXText;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* CameraInvertYCheckBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CameraInvertYText;
	
	//画面の明るさ
	UPROPERTY(meta = (BindWidget))
	USlider* ScreenBrightnessSlider;

	//アニメーション
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CloseAnim;
protected:
	//現在選択している設定項目のインデックス
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	int32 m_CurrentSettingIndex = 0;

	//設定項目の最大インデックス
	int32 m_MaxSettingIndex = 4;

private:
	//デフォルト値になった時に硬直するタイマー
	float m_StopCooldown = 0.f;
	//硬直させる元の値
	float m_DefaultSensitivity = 0.9f;

	//入力時間を記録
	float m_LastInputTime = 0.f;

	//最初の一回目かどうかを判定
	bool m_IsFirstPress = true;

	//明るさのデフォルト値
	float m_DafultBrightness = 2.2f;
};

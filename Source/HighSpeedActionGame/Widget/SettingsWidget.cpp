// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "../PlayerSettingsSubsystem/PlayerSettingsSubsystem.h"
#include "../PlayerCharacter.h" 
#include "Camera/CameraComponent.h"


void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UPlayerSettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UPlayerSettingsSubsystem>();
	if (SettingsSubsystem && CameraSensitivitySlider)
	{
		// 現在の設定値をスライダーに反映
		CameraSensitivitySlider->SetValue(SettingsSubsystem->GetCameraSensitivity());

		if (CameraSensitivityText) {
			CameraSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), SettingsSubsystem->GetCameraSensitivity())));
		}

		//左右リバース(X)の反映
		bool bInvertX = SettingsSubsystem->GetInvertCameraX();
		if (CameraInvertXCheckBox) {
			CameraInvertXCheckBox->SetIsChecked(bInvertX);
		}
		if (CameraInvertXText) {
			//trueなら Reverse、falseなら Normal
			CameraInvertXText->SetText(bInvertX ? FText::FromString(TEXT("Reverse")) : FText::FromString(TEXT("Normal")));
		}

		//上下リバース(Y)の反映
		bool bInvertY = SettingsSubsystem->GetInvertCameraY();
		if (CameraInvertYCheckBox) {
			CameraInvertYCheckBox->SetIsChecked(bInvertY);
		}
		if (CameraInvertYText) {
			CameraInvertYText->SetText(bInvertY ? FText::FromString(TEXT("Reverse")) : FText::FromString(TEXT("Normal")));
		}

		float CurrentBrightness = SettingsSubsystem->GetScreenBrightness();

		if (ScreenBrightnessSlider) {
			ScreenBrightnessSlider->SetValue(CurrentBrightness);
		}

		if (GEngine)
		{
			GEngine->DisplayGamma = CurrentBrightness;
		}
	}

	// 初期表示時に見た目を更新
	UpdateSelectionUI();
}

void USettingsWidget::ChangeSelection(int32 Direction)
{
	m_CurrentSettingIndex += Direction;

	// 項目をループさせる
	if (m_CurrentSettingIndex < 0) {
		m_CurrentSettingIndex = m_MaxSettingIndex;
	}
	else if (m_CurrentSettingIndex > m_MaxSettingIndex) {
		m_CurrentSettingIndex = 0;
	}

	// 選択が切り替わったらBP側の見た目更新イベントを呼ぶ
	UpdateSelectionUI();
}

void USettingsWidget::ChangeValue(float Direction, float DeltaTime)
{
	//クールダウン中は無視
	if (m_StopCooldown > 0.f)
	{
		m_StopCooldown -= DeltaTime;
		return;
	}

	UPlayerSettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UPlayerSettingsSubsystem>();
	if (!SettingsSubsystem) return;

	//カメラ感度の変更
	if (m_CurrentSettingIndex == 0)
	{
		float CurrentVal = SettingsSubsystem->GetCameraSensitivity();
		float NewVal = CurrentVal + (Direction * 0.1f);
		NewVal = FMath::RoundToFloat(NewVal * 10.f) / 10.f;

		//スライダーの範囲
		NewVal = FMath::Clamp(NewVal, 0.1f, 2.0f);

		if (FMath::IsNearlyEqual(CurrentVal, NewVal)) return;

		//デフォルト値を通過しようとしたら、0.9でピタッと止める
		if ((CurrentVal < m_DefaultSensitivity && NewVal >= m_DefaultSensitivity) ||
			(CurrentVal > m_DefaultSensitivity && NewVal <= m_DefaultSensitivity))
		{
			NewVal = m_DefaultSensitivity;
			m_StopCooldown = 0.3f;
		}
		else
		{
			if (m_IsFirstPress) {
				//1回目は長めにディレイを入れる（短押しで複数回動くのを防ぐ）
				m_StopCooldown = 0.3f;
				m_IsFirstPress = false; // 次からは長押し扱い
			}
			else {
				//長押し中は高速で連続移動させる
				m_StopCooldown = 0.05f;
			}
		}

		SettingsSubsystem->SetCameraSensitivity(NewVal);

		//スライダーの更新
		if (CameraSensitivitySlider) {
			CameraSensitivitySlider->SetValue(NewVal);
		}

		//テキストの更新
		if (CameraSensitivityText) {
			CameraSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), NewVal)));
		}
	}

	//画面の明るさ
	else if (m_CurrentSettingIndex == 3)
	{
		float CurrentVal = SettingsSubsystem->GetScreenBrightness();
		float NewVal = CurrentVal + (Direction * 0.1f);
		NewVal = FMath::RoundToFloat(NewVal * 10.f) / 10.f;

		if (FMath::IsNearlyEqual(CurrentVal, NewVal))return;

		// ガンマ値の範囲（1.0 がかなり暗め、3.0 がかなり明るめです）
		NewVal = FMath::Clamp(NewVal, 0.1f, 4.f);

		//デフォルト値を通過しようとしたら止める
		m_DafultBrightness = 2.2f;

		if ((CurrentVal < m_DafultBrightness && NewVal >= m_DafultBrightness) ||
			(CurrentVal > m_DafultBrightness && NewVal <= m_DafultBrightness))
		{
			NewVal = m_DafultBrightness;
			m_StopCooldown = 0.3f;
		}
		else
		{
			if (m_IsFirstPress) {
				//1回目は長めにディレイを入れる（短押しで複数回動くのを防ぐ）
				m_StopCooldown = 0.3f;
				m_IsFirstPress = false; // 次からは長押し扱い
			}
			else {
				//長押し中は高速で連続移動させる
				m_StopCooldown = 0.05f;
			}
		}

		SettingsSubsystem->SetScreenBrightness(NewVal);

		if (GEngine)
		{
			GEngine->DisplayGamma = NewVal;
		}

		if (ScreenBrightnessSlider)
		{
			ScreenBrightnessSlider->SetValue(NewVal);
		}
	}
}

void USettingsWidget::DecisionCurrentSetting()
{
	UPlayerSettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UPlayerSettingsSubsystem>();
	if (!SettingsSubsystem) return;

	//X反転の項目を選んでいる時
	if (m_CurrentSettingIndex == 1)
	{
		bool bNewState = !SettingsSubsystem->GetInvertCameraX();

		SettingsSubsystem->SetInvertCameraX(bNewState);

		if (CameraInvertXCheckBox) {
			CameraInvertXCheckBox->SetIsChecked(bNewState);
		}
		if (CameraInvertXText) {
			CameraInvertXText->SetText(bNewState ? FText::FromString(TEXT("Reverse")) : FText::FromString(TEXT("Normal")));
		}
	}
	//Y反転の項目を選んでいる時
	else if (m_CurrentSettingIndex == 2)
	{
		bool bNewState = !SettingsSubsystem->GetInvertCameraY();

		SettingsSubsystem->SetInvertCameraY(bNewState);

		if (CameraInvertYCheckBox) {
			CameraInvertYCheckBox->SetIsChecked(bNewState);
		}
		if (CameraInvertYText) {
			CameraInvertYText->SetText(bNewState ? FText::FromString(TEXT("Reverse")) : FText::FromString(TEXT("Normal")));
		}
	}

	else if (m_CurrentSettingIndex == 4)
	{
		//サブシステムの値をデフォルト値で上書き
		SettingsSubsystem->SetCameraSensitivity(0.9f);
		SettingsSubsystem->SetInvertCameraX(false);
		SettingsSubsystem->SetInvertCameraY(false);
		SettingsSubsystem->SetScreenBrightness(2.2f);

		//UIの見た目をすべて初期値に更新
		if (CameraSensitivitySlider) { CameraSensitivitySlider->SetValue(0.9f); }
		if (CameraSensitivityText) { CameraSensitivityText->SetText(FText::FromString(TEXT("0.9"))); }

		if (CameraInvertXCheckBox) { CameraInvertXCheckBox->SetIsChecked(false); }
		if (CameraInvertXText) { CameraInvertXText->SetText(FText::FromString(TEXT("Normal"))); }

		if (CameraInvertYCheckBox) { CameraInvertYCheckBox->SetIsChecked(false); }
		if (CameraInvertYText) { CameraInvertYText->SetText(FText::FromString(TEXT("Normal"))); }

		if (ScreenBrightnessSlider) { ScreenBrightnessSlider->SetValue(2.2f); }

		//画面全体の明るさを即座にリセット
		if (GEngine)
		{
			GEngine->DisplayGamma = 2.2f;
		}
	}
}


void USettingsWidget::ResetInputState()
{
	m_StopCooldown = 0.f;
	m_IsFirstPress = true;
}

void USettingsWidget::PlayOpenAnimation()
{
	if (OpenAnim) {
		PlayAnimation(OpenAnim);
	}
}

void USettingsWidget::PlayCloseAnimation()
{
	if (CloseAnim) {
		PlayAnimation(CloseAnim);
	}
}

bool USettingsWidget::GetIsAnimationPlaying() const
{
	// どちらかのアニメーションが再生中なら true を返す
	bool bIsPlaying = false;
	if (OpenAnim && IsAnimationPlaying(OpenAnim)) bIsPlaying = true;
	if (CloseAnim && IsAnimationPlaying(CloseAnim)) bIsPlaying = true;
	return bIsPlaying;
}

void USettingsWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	// 再生が終わったアニメーションが「閉じるアニメーション」だった場合
	if (Animation == CloseAnim)
	{
		OnSettingsClosedDelegate.Broadcast();
	}
}

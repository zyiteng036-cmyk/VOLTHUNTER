#include "SettingsWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "../PlayerSettingsSubsystem/PlayerSettingsSubsystem.h"
#include "../PlayerCharacter.h" 
#include "Camera/CameraComponent.h"

namespace SettingsWidgetConstants
{
	//設定項目のインデックス定義
	constexpr int32 IndexCameraSensitivity = 0;
	constexpr int32 IndexCameraInvertX = 1;
	constexpr int32 IndexCameraInvertY = 2;
	constexpr int32 IndexScreenBrightness = 3;
	constexpr int32 IndexResetSettings = 4;

	//ループ処理用インデックス
	constexpr int32 MinSettingIndex = 0;

	//数値調整用のステップ値およびクランプ範囲定数
	constexpr float ValueStep = 0.1f;
	constexpr float SensitivityMin = 0.1f;
	constexpr float SensitivityMax = 2.0f;
	constexpr float BrightnessMin = 0.1f;
	constexpr float BrightnessMax = 4.0f;
	constexpr float RoundMultiplier = 10.0f;

	//クールダウン・ディレイ時間定数
	constexpr float CooldownZero = 0.0f;
	constexpr float CooldownDefaultPass = 0.3f;
	constexpr float CooldownFirstPress = 0.3f;
	constexpr float CooldownContinuous = 0.05f;
}

//ウィジェット構築時の初期化処理
void USettingsWidget::NativeConstruct()
{
	//親クラスの構築処理を呼び出し
	Super::NativeConstruct();

	//セーブデータを管理するサブシステムを取得
	UPlayerSettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UPlayerSettingsSubsystem>();
	if (SettingsSubsystem && m_CameraSensitivitySlider)
	{
		//現在の設定値をスライダーに反映
		m_CameraSensitivitySlider->SetValue(SettingsSubsystem->GetCameraSensitivity());

		//感度テキストの更新
		if (m_CameraSensitivityText) {
			m_CameraSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), SettingsSubsystem->GetCameraSensitivity())));
		}

		//左右リバース(X)の反映
		bool bInvertX = SettingsSubsystem->GetInvertCameraX();
		if (m_CameraInvertXCheckBox) {
			m_CameraInvertXCheckBox->SetIsChecked(bInvertX);
		}
		if (m_CameraInvertXText) {
			//trueならReverse、falseならNormal
			m_CameraInvertXText->SetText(bInvertX ? FText::FromString(TEXT("Reverse")) : FText::FromString(TEXT("Normal")));
		}

		//上下リバース(Y)の反映
		bool bInvertY = SettingsSubsystem->GetInvertCameraY();
		if (m_CameraInvertYCheckBox) {
			m_CameraInvertYCheckBox->SetIsChecked(bInvertY);
		}
		if (m_CameraInvertYText) {
			m_CameraInvertYText->SetText(bInvertY ? FText::FromString(TEXT("Reverse")) : FText::FromString(TEXT("Normal")));
		}

		//現在の明るさを取得してスライダーとエンジンガンマ値に反映
		float CurrentBrightness = SettingsSubsystem->GetScreenBrightness();

		if (m_ScreenBrightnessSlider) {
			m_ScreenBrightnessSlider->SetValue(CurrentBrightness);
		}

		if (GEngine)
		{
			GEngine->DisplayGamma = CurrentBrightness;
		}
	}

	//初期表示時に見た目を更新
	UpdateSelectionUI();
}

//上下入力による選択項目の変更
void USettingsWidget::ChangeSelection(int32 Direction)
{
	//選択インデックスを増減
	m_CurrentSettingIndex += Direction;

	//項目をループさせる
	if (m_CurrentSettingIndex < SettingsWidgetConstants::MinSettingIndex) {
		m_CurrentSettingIndex = m_MaxSettingIndex;
	}
	else if (m_CurrentSettingIndex > m_MaxSettingIndex) {
		m_CurrentSettingIndex = SettingsWidgetConstants::MinSettingIndex;
	}

	//選択が切り替わったらBP側の見た目更新イベントを呼ぶ
	UpdateSelectionUI();
}

//左右入力による設定値の変更
void USettingsWidget::ChangeValue(float Direction, float DeltaTime)
{
	//クールダウン中は入力を無視してタイマーを減算
	if (m_StopCooldown > SettingsWidgetConstants::CooldownZero)
	{
		m_StopCooldown -= DeltaTime;
		return;
	}

	//サブシステムの有効性チェック
	UPlayerSettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UPlayerSettingsSubsystem>();
	if (!SettingsSubsystem) return;

	//カメラ感度の変更処理
	if (m_CurrentSettingIndex == SettingsWidgetConstants::IndexCameraSensitivity)
	{
		float CurrentVal = SettingsSubsystem->GetCameraSensitivity();
		float NewVal = CurrentVal + (Direction * SettingsWidgetConstants::ValueStep);

		//小数第一位で四捨五入
		NewVal = FMath::RoundToFloat(NewVal * SettingsWidgetConstants::RoundMultiplier) / SettingsWidgetConstants::RoundMultiplier;

		//スライダーの範囲にクランプ
		NewVal = FMath::Clamp(NewVal, SettingsWidgetConstants::SensitivityMin, SettingsWidgetConstants::SensitivityMax);

		//変化がなければ処理を抜ける
		if (FMath::IsNearlyEqual(CurrentVal, NewVal)) return;

		//デフォルト値を通過しようとしたら、特定の初期値でピタッと止める
		if ((CurrentVal < m_DefaultSensitivity && NewVal >= m_DefaultSensitivity) ||
			(CurrentVal > m_DefaultSensitivity && NewVal <= m_DefaultSensitivity))
		{
			NewVal = m_DefaultSensitivity;
			m_StopCooldown = SettingsWidgetConstants::CooldownDefaultPass;
		}
		else
		{
			if (m_IsFirstPress) {
				//1回目は長めにディレイを入れる(短押しで複数回動くのを防ぐ)
				m_StopCooldown = SettingsWidgetConstants::CooldownFirstPress;
				m_IsFirstPress = false; //次からは長押し扱い
			}
			else {
				//長押し中は高速で連続移動させる
				m_StopCooldown = SettingsWidgetConstants::CooldownContinuous;
			}
		}

		//変更した値をサブシステムに保存
		SettingsSubsystem->SetCameraSensitivity(NewVal);

		//スライダーの更新
		if (m_CameraSensitivitySlider) {
			m_CameraSensitivitySlider->SetValue(NewVal);
		}

		//テキストの更新
		if (m_CameraSensitivityText) {
			m_CameraSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), NewVal)));
		}
	}

	//画面の明るさの変更処理
	else if (m_CurrentSettingIndex == SettingsWidgetConstants::IndexScreenBrightness)
	{
		float CurrentVal = SettingsSubsystem->GetScreenBrightness();
		float NewVal = CurrentVal + (Direction * SettingsWidgetConstants::ValueStep);

		//小数第一位で四捨五入
		NewVal = FMath::RoundToFloat(NewVal * SettingsWidgetConstants::RoundMultiplier) / SettingsWidgetConstants::RoundMultiplier;

		//変化がなければ処理を抜ける
		if (FMath::IsNearlyEqual(CurrentVal, NewVal)) return;

		//ガンマ値の範囲にクランプ
		NewVal = FMath::Clamp(NewVal, SettingsWidgetConstants::BrightnessMin, SettingsWidgetConstants::BrightnessMax);

		//デフォルト値を通過しようとしたら止める
		if ((CurrentVal < m_DafultBrightness && NewVal >= m_DafultBrightness) ||
			(CurrentVal > m_DafultBrightness && NewVal <= m_DafultBrightness))
		{
			NewVal = m_DafultBrightness;
			m_StopCooldown = SettingsWidgetConstants::CooldownDefaultPass;
		}
		else
		{
			if (m_IsFirstPress) {
				//1回目は長めにディレイを入れる(短押しで複数回動くのを防ぐ)
				m_StopCooldown = SettingsWidgetConstants::CooldownFirstPress;
				m_IsFirstPress = false; //次からは長押し扱い
			}
			else {
				//長押し中は高速で連続移動させる
				m_StopCooldown = SettingsWidgetConstants::CooldownContinuous;
			}
		}

		//変更した明るさをサブシステムとエンジンに適用
		SettingsSubsystem->SetScreenBrightness(NewVal);

		if (GEngine)
		{
			GEngine->DisplayGamma = NewVal;
		}

		if (m_ScreenBrightnessSlider)
		{
			m_ScreenBrightnessSlider->SetValue(NewVal);
		}
	}
}

//決定ボタン押下時の各項目のトグル・リセット処理
void USettingsWidget::DecisionCurrentSetting()
{
	//サブシステムの有効性チェック
	UPlayerSettingsSubsystem* SettingsSubsystem = GetGameInstance()->GetSubsystem<UPlayerSettingsSubsystem>();
	if (!SettingsSubsystem) return;

	//X反転の項目を選んでいる時
	if (m_CurrentSettingIndex == SettingsWidgetConstants::IndexCameraInvertX)
	{
		bool bNewState = !SettingsSubsystem->GetInvertCameraX();

		SettingsSubsystem->SetInvertCameraX(bNewState);

		if (m_CameraInvertXCheckBox) {
			m_CameraInvertXCheckBox->SetIsChecked(bNewState);
		}
		if (m_CameraInvertXText) {
			m_CameraInvertXText->SetText(bNewState ? FText::FromString(TEXT("Reverse")) : FText::FromString(TEXT("Normal")));
		}
	}
	//Y反転の項目を選んでいる時
	else if (m_CurrentSettingIndex == SettingsWidgetConstants::IndexCameraInvertY)
	{
		bool bNewState = !SettingsSubsystem->GetInvertCameraY();

		SettingsSubsystem->SetInvertCameraY(bNewState);

		if (m_CameraInvertYCheckBox) {
			m_CameraInvertYCheckBox->SetIsChecked(bNewState);
		}
		if (m_CameraInvertYText) {
			m_CameraInvertYText->SetText(bNewState ? FText::FromString(TEXT("Reverse")) : FText::FromString(TEXT("Normal")));
		}
	}
	//オールリセット項目を選んでいる時
	else if (m_CurrentSettingIndex == SettingsWidgetConstants::IndexResetSettings)
	{
		//サブシステムの値をデフォルト値で上書き
		SettingsSubsystem->SetCameraSensitivity(m_DefaultSensitivity);
		SettingsSubsystem->SetInvertCameraX(false);
		SettingsSubsystem->SetInvertCameraY(false);
		SettingsSubsystem->SetScreenBrightness(m_DafultBrightness);

		//UIの見た目をすべて初期値に更新
		if (m_CameraSensitivitySlider) { m_CameraSensitivitySlider->SetValue(m_DefaultSensitivity); }
		if (m_CameraSensitivityText) { m_CameraSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), m_DefaultSensitivity))); }

		if (m_CameraInvertXCheckBox) { m_CameraInvertXCheckBox->SetIsChecked(false); }
		if (m_CameraInvertXText) { m_CameraInvertXText->SetText(FText::FromString(TEXT("Normal"))); }

		if (m_CameraInvertYCheckBox) { m_CameraInvertYCheckBox->SetIsChecked(false); }
		if (m_CameraInvertYText) { m_CameraInvertYText->SetText(FText::FromString(TEXT("Normal"))); }

		if (m_ScreenBrightnessSlider) { m_ScreenBrightnessSlider->SetValue(m_DafultBrightness); }

		//画面全体の明るさを即座にリセット
		if (GEngine)
		{
			GEngine->DisplayGamma = m_DafultBrightness;
		}
	}
}

//連続入力状態のリセット
void USettingsWidget::ResetInputState()
{
	m_StopCooldown = SettingsWidgetConstants::CooldownZero;
	m_IsFirstPress = true;
}

//開くアニメーションの再生
void USettingsWidget::PlayOpenAnimation()
{
	if (m_OpenAnim) {
		PlayAnimation(m_OpenAnim);
	}
}

//閉じるアニメーションの再生
void USettingsWidget::PlayCloseAnimation()
{
	if (m_CloseAnim) {
		PlayAnimation(m_CloseAnim);
	}
}

//アニメーションが再生中か判定
bool USettingsWidget::GetIsAnimationPlaying() const
{
	//どちらかのアニメーションが再生中ならtrueを返す
	bool bIsPlaying = false;
	if (m_OpenAnim && IsAnimationPlaying(m_OpenAnim)) bIsPlaying = true;
	if (m_CloseAnim && IsAnimationPlaying(m_CloseAnim)) bIsPlaying = true;
	return bIsPlaying;
}

//アニメーション終了時のコールバック実装
void USettingsWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	//親クラスの終了処理を呼び出し
	Super::OnAnimationFinished_Implementation(Animation);

	//再生が終わったアニメーションが「閉じるアニメーション」だった場合
	if (Animation == m_CloseAnim)
	{
		//外部への通知デリゲートをブロードキャスト
		m_OnSettingsClosedDelegate.Broadcast();
	}
}
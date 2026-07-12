#include "PlayerSettingsSubsystem.h"
#include "Engine/Engine.h"

//Subsystemを初期化
void UPlayerSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//初期設定の明るさを画面へ反映
	ApplyScreenBrightness();
}

//Subsystemを終了
void UPlayerSettingsSubsystem::Deinitialize()
{
	if (GEngine)
	{
		GEngine->DisplayGamma = 2.2f;
	}
	Super::Deinitialize();
}

//画面の明るさを設定
void UPlayerSettingsSubsystem::SetScreenBrightness(float Brightness)
{
	m_ScreenBrightness = Brightness;

	//設定画面の変更を即座に反映
	ApplyScreenBrightness();
}

//保存中の明るさを画面へ反映
void UPlayerSettingsSubsystem::ApplyScreenBrightness() const
{
	if (!GEngine) return;

	GEngine->DisplayGamma = m_ScreenBrightness;
}
//担当：佐々木奏太 クラス生成者
//伊藤直樹 中身担当

//-----------------------------------------------------
//プレイヤー設定Subsystem
//
//カメラ感度、カメラ反転、画面の明るさを管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerSettingsSubsystem.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayerSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//Subsystemを初期化
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	//Subsystemを終了
	virtual void Deinitialize() override;

	//カメラ感度を設定
	UFUNCTION(BlueprintCallable, Category = "PlayerSettings|Camera")
	void SetCameraSensitivity(float Sensitivity) { m_CameraSensitivity = Sensitivity; }

	//カメラ感度を取得
	UFUNCTION(BlueprintPure, Category = "PlayerSettings|Camera")
	float GetCameraSensitivity() const { return m_CameraSensitivity; }

	//X軸のカメラ反転を設定
	UFUNCTION(BlueprintCallable, Category = "PlayerSettings|Camera")
	void SetInvertCameraX(bool bInvert) { m_bInvertCameraX = bInvert; }

	//X軸のカメラ反転を取得
	UFUNCTION(BlueprintPure, Category = "PlayerSettings|Camera")
	bool GetInvertCameraX() const { return m_bInvertCameraX; }

	//Y軸のカメラ反転を設定
	UFUNCTION(BlueprintCallable, Category = "PlayerSettings|Camera")
	void SetInvertCameraY(bool bInvert) { m_bInvertCameraY = bInvert; }

	//Y軸のカメラ反転を取得
	UFUNCTION(BlueprintPure, Category = "PlayerSettings|Camera")
	bool GetInvertCameraY() const { return m_bInvertCameraY; }

	//画面の明るさを設定
	UFUNCTION(BlueprintCallable, Category = "PlayerSettings|Display")
	void SetScreenBrightness(float Brightness);

	//画面の明るさを取得
	UFUNCTION(BlueprintPure, Category = "PlayerSettings|Display")
	float GetScreenBrightness() const { return m_ScreenBrightness; }

private:
	//保存中の明るさを画面へ反映
	void ApplyScreenBrightness() const;

private:
	//カメラ感度
	float m_CameraSensitivity = 0.9f;

	//X軸のカメラ反転
	bool m_bInvertCameraX = false;

	//Y軸のカメラ反転
	bool m_bInvertCameraY = false;

	//画面の明るさ
	float m_ScreenBrightness = 2.2f;
};
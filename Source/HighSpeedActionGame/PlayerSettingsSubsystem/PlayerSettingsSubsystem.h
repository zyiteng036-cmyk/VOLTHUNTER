//担当：佐々木奏太 クラス生成者
//伊藤直樹　中身担当

//プレイヤーの設定等を保存しておくゲームインスタンスサブシステム

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerSettingsSubsystem.generated.h"

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayerSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()


public:
	//=== カメラ感度 ===
	UFUNCTION(BlueprintCallable, Category = "PlayerSettings|Camera")
	void SetCameraSensitivity(float _sensitivity) { m_CameraSensitivity = _sensitivity; }

	UFUNCTION(BlueprintPure, Category = "PlayerSettings|Camera")
	float GetCameraSensitivity() const { return m_CameraSensitivity; } // const 追加

	//=== カメラ反転 (X軸) ===
	UFUNCTION(BlueprintCallable, Category = "PlayerSettings|Camera")
	void SetInvertCameraX(bool _bInvert) { m_bInvertCameraX = _bInvert; }

	UFUNCTION(BlueprintPure, Category = "PlayerSettings|Camera")
	bool GetInvertCameraX() const { return m_bInvertCameraX; }

	//=== カメラ反転 (Y軸) ===
	UFUNCTION(BlueprintCallable, Category = "PlayerSettings|Camera")
	void SetInvertCameraY(bool _bInvert) { m_bInvertCameraY = _bInvert; }

	UFUNCTION(BlueprintPure, Category = "PlayerSettings|Camera")
	bool GetInvertCameraY() const { return m_bInvertCameraY; }

	//=== 画面の明るさ (Gamma) ===
	UFUNCTION(BlueprintCallable, Category = "PlayerSettings|Display")
	void SetScreenBrightness(float _Brightness) { m_ScreenBrightness = _Brightness; }

	UFUNCTION(BlueprintPure, Category = "PlayerSettings|Display")
	float GetScreenBrightness() const { return m_ScreenBrightness; }
	virtual void Deinitialize()override;
private:
	//カメラ感度
	float m_CameraSensitivity = 0.9f;

	//カメラ反転
	bool m_bInvertCameraX = false;
	bool m_bInvertCameraY = false;

	//画面の明るさ
	float m_ScreenBrightness = 2.2;
};

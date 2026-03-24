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
	//カメラ感度
	void SetCameraSensitivity(float _sensitivity) { m_CameraSensitivity = _sensitivity; }
	float GetCameraSensitivity() { return m_CameraSensitivity; }
	//カメラ反転
	void SetInvertCameraX(bool _bInvert) { m_bInvertCameraX = _bInvert; }
	bool GetInvertCameraX() const { return m_bInvertCameraX; }

	void SetInvertCameraY(bool _bInvert) { m_bInvertCameraY = _bInvert; }
	bool GetInvertCameraY() const { return m_bInvertCameraY; }

	//画面の明るさ
	void SetScreenBrightness(float _Brightness) { m_ScreenBrightness = _Brightness; }
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

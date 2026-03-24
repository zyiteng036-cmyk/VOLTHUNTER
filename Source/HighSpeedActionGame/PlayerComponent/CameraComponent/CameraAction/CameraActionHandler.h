//担当
//伊藤直樹

//カメラの距離などを管理するハンドラー

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CameraActionHandler.generated.h"

class UPlayer_CameraComponent;
class USpringArmComponent;
class UCameraComponent;
class APlayerCharacter;
class APlayerController;
class UPlayer_MovementComponent;
/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraActionHandler : public UObject
{
	GENERATED_BODY()
	
public:
	//初期化
	void Initialize(APlayerCharacter* InPlayer, APlayerController* InController,
		USpringArmComponent* InSpringArm, UCameraComponent* InCamera,
		UPlayer_MovementComponent* InMovementComp);

	//毎フレームの更新
	void UpdateActionCamera(float DeltaTime);

	//パラメータ設定用（ComponentのPlayerParamから値をコピーしてくる）
	void SetupParams(float InDefaultLength, float InDefaultFOV,
		float InDashStartLength, float InDashStartFOV,
		float InDashMidLength, float InDashMidFOV);

private:
	//オートフレーミング計算（privateに隠蔽）
	float GetAutoFramingOffset(float DeltaTime);

private:
	//参照ポインタ
	UPROPERTY()
	APlayerCharacter* m_Player;

	UPROPERTY()
	APlayerController* m_PlayerController;

	UPROPERTY()
	USpringArmComponent* m_SpringArm;

	UPROPERTY()
	UCameraComponent* m_Camera;

	UPROPERTY()
	UPlayer_MovementComponent* m_MovementComponent;

	//--- 内部ステート変数 ---
	float m_AutoFramingOffset;

	//--- パラメータ (Componentから受け取る) ---
	float p_DefaultArmLength;
	float p_DefaultFOV;
	float p_DashStartArmLength;
	float p_DashStartFOV;
	float p_DashMidArmLength;
	float p_DashMidFOV;

	//定数（必要ならこれもSetupParamsで受け取れるようにする）
	const float c_FramingSafeZone = 0.2f;
	const float c_MaxFramingZoom = 800.f;
};

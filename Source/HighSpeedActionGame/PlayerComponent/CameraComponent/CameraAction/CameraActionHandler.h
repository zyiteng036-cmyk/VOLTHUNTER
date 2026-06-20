//担当
//伊藤直樹

//プレイヤーのアクション(ダッシュ等)や画面端への移動(オートフレーミング)に合わせてカメラを動的に制御するクラス

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

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraActionHandler : public UObject
{
	GENERATED_BODY()

public:
	//初期化処理
	void Initialize(APlayerCharacter* InPlayer, APlayerController* InController,
		USpringArmComponent* InSpringArm, UCameraComponent* InCamera,
		UPlayer_MovementComponent* InMovementComp);

	//毎フレームの更新
	void UpdateActionCamera(float DeltaTime);

	//パラメータ設定用(ComponentのPlayerParamから値をコピーしてくる)
	void SetupParams(float InDefaultLength, float InDefaultFOV,
		float InDashStartLength, float InDashStartFOV,
		float InDashMidLength, float InDashMidFOV);

private:
	//オートフレーミング計算(privateに隠蔽)
	float GetAutoFramingOffset(float DeltaTime);

private:
	//参照ポインタ
	UPROPERTY()
	TWeakObjectPtr<APlayerCharacter> m_Player = nullptr;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> m_PlayerController = nullptr;

	UPROPERTY()
	TWeakObjectPtr<USpringArmComponent> m_SpringArm = nullptr;

	UPROPERTY()
	TWeakObjectPtr<UCameraComponent> m_Camera = nullptr;

	UPROPERTY()
	TWeakObjectPtr<UPlayer_MovementComponent> m_MovementComponent = nullptr;

	//内部ステート変数
	float m_AutoFramingOffset = 0.0f;

	//パラメータ(Componentから受け取る)
	float m_DefaultArmLength = 300.0f;
	float m_DefaultFOV = 90.0f;
	float m_DashStartArmLength = 0.0f;
	float m_DashStartFOV = 0.0f;
	float m_DashMidArmLength = 0.0f;
	float m_DashMidFOV = 0.0f;
};
//担当
//伊藤直樹

//ボスシーンでのカメラハンドラー

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CameraBossHandler.generated.h"

class USpringArmComponent;
class UCameraComponent;
class APlayerController;
/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew)
class HIGHSPEEDACTIONGAME_API UCameraBossHandler : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	//ボス戦カメラ開始
	void StartBossCamera();

	//ボス戦カメラ終了
	void EndBossCamera();

	//更新処理
	bool UpdateBossCamera(float DeltaTime);

	bool IsActive()const;

protected:
	//ボス専用パラメータ
	UPROPERTY(EditAnywhere, Category = "BossCamera")
	float m_BossArmLength=600.f;

	UPROPERTY(EditAnywhere, Category = "Boss Camera")
	float m_BossFOV = 100.0f;        // 画角を広げて巨大感を出す


	UPROPERTY(EditAnywhere, Category = "Boss Camera")
	float m_TransitionSpeed = 2.0f;   // 移行する速さ

private:
	USpringArmComponent* SpringArm;
	UCameraComponent* Camera;

	bool m_IsActive;

	// 元に戻すためのキャッシュ
	float m_DefaultArmLength;
	float m_DefaultFOV;
};

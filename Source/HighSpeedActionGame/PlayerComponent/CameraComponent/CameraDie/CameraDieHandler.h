//担当
//伊藤直樹

//死亡時のカメラハンドラー

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CameraDieHandler.generated.h"

class USpringArmComponent;
class UCameraComponent;
class APlayerController;

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew)
class HIGHSPEEDACTIONGAME_API UCameraDieHandler : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize(USpringArmComponent* InSpringArm, APlayerController* InController);

	//死亡演出開始
	void StartDieCamera();

	bool IsActive()const { return m_IsActive; }

	void EndDieCamera();
private:
	UPROPERTY()
	USpringArmComponent* SpringArm;

	UPROPERTY()
	APlayerController* PlayerController;

	bool m_IsActive;

	// 最終的なアームの長さ
	float m_TargetArmLength;
protected:
	//死亡時に追加で引く
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DieCamera")
	float m_DieZoomOutDistance = 150.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DieCamera")
	float m_TargetPitch = -50.0f;

private:
	// 元のラグ設定を保存しておく変数
	bool m_bSavedLagParams = false;
	bool m_bOriginalCameraLag = false;
	bool m_bOriginalRotationLag = false;
};

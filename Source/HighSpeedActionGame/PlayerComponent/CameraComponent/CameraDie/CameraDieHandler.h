//担当
//伊藤直樹

//キャラクター死亡時のカメラ挙動(位置固定、ズームアウト、角度変更など)を制御するハンドラー

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CameraDieHandler.generated.h"

class USpringArmComponent;
class UCameraComponent;
class APlayerController;

UCLASS(Blueprintable, EditInlineNew)
class HIGHSPEEDACTIONGAME_API UCameraDieHandler : public UObject
{
	GENERATED_BODY()

public:
	//初期化処理
	void Initialize(USpringArmComponent* InSpringArm, APlayerController* InController);

	//死亡演出開始
	void StartDieCamera();

	//現在演出中かどうかを取得
	bool IsActive() const { return m_IsActive; }

	//死亡演出終了(元の状態に戻す)
	void EndDieCamera();

private:
	//対象のスプリングアーム
	UPROPERTY()
	TWeakObjectPtr<USpringArmComponent> m_SpringArm = nullptr;

	//対象のプレイヤーコントローラー
	UPROPERTY()
	TWeakObjectPtr<APlayerController> m_PlayerController = nullptr;

	//演出中フラグ
	bool m_IsActive = false;

	//最終的なアームの長さ
	float m_TargetArmLength = 0.0f;

protected:
	//死亡時に追加で引く距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DieCamera")
	float m_DieZoomOutDistance = 250.0f;

	//死亡時の見下ろし角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DieCamera")
	float m_TargetPitch = -50.0f;

private:
	//元のラグ設定を保存しておく変数
	bool m_IsSavedLagParams = false;
	bool m_IsOriginalCameraLag = false;
	bool m_IsOriginalRotationLag = false;
};
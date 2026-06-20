//担当
//伊藤直樹

//ボス戦時のカメラ挙動(距離の調整やFOVの拡張による巨大感の演出)を制御するハンドラークラス

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CameraBossHandler.generated.h"

class USpringArmComponent;
class UCameraComponent;
class APlayerController;

UCLASS(Blueprintable, EditInlineNew)
class HIGHSPEEDACTIONGAME_API UCameraBossHandler : public UObject
{
	GENERATED_BODY()

public:
	//初期化処理
	void Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	//ボス戦カメラ開始
	void StartBossCamera();

	//ボス戦カメラ終了
	void EndBossCamera();

	//更新処理(trueを返している間は処理継続)
	bool UpdateBossCamera(float DeltaTime);

	//現在演出中かどうかを取得
	bool IsActive() const { return m_IsActive; }

protected:
	//ボス専用パラメータ
	//ボス戦時のカメラの基本距離
	UPROPERTY(EditAnywhere, Category = "BossCamera")
	float m_BossArmLength = 600.0f;

	//画角を広げて巨大感を出すためのFOV設定
	UPROPERTY(EditAnywhere, Category = "BossCamera")
	float m_BossFOV = 100.0f;

	//通常状態からボス用カメラへ移行する際の補間スピード
	UPROPERTY(EditAnywhere, Category = "BossCamera")
	float m_TransitionSpeed = 2.0f;

private:
	//対象のスプリングアーム
	TWeakObjectPtr<USpringArmComponent> m_SpringArm = nullptr;

	//対象のカメラ
	TWeakObjectPtr<UCameraComponent> m_Camera = nullptr;

	//ボス戦カメラ演出中フラグ
	bool m_IsActive = false;

	//元に戻すためのキャッシュ
	//開始前のデフォルトのカメラ距離
	float m_DefaultArmLength = 0.0f;

	//開始前のデフォルトのFOV
	float m_DefaultFOV = 0.0f;
};
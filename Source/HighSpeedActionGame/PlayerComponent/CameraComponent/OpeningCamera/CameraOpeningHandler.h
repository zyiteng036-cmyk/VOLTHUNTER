//担当
//伊藤直樹

//ゲーム開始時のオープニング演出(カメラとUI)を管理するクラス

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CameraOpeningHandler.generated.h"

class USpringArmComponent;
class UCameraComponent;
class APlayerController;

UCLASS(Blueprintable, EditInlineNew)
class HIGHSPEEDACTIONGAME_API UCameraOpeningHandler : public UObject
{
	GENERATED_BODY()

public:
	//初期化処理
	void Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera, APlayerController* InPC);

	//演出開始
	void StartOpening(float WaitTime, float MoveDuration, float FaceDistance, float StartYawOffset);

	//更新処理(trueを返している間は演出中)
	bool UpdateOpening(float DeltaTime);

	//強制終了
	void StopOpening();

	//現在演出中かどうかを取得
	bool IsActive() const { return m_IsActive; }

public:
	//オープニング時に表示するUIクラス
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> m_OpeningWidgetClass = nullptr;

	//移動開始からUIが消え始めるまでの時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Opening Settings")
	float m_UiHideDelayTime = 2.0f;

private:
	//対象のスプリングアーム
	TWeakObjectPtr<USpringArmComponent> m_SpringArm = nullptr;

	//対象のプレイヤーコントローラー
	UPROPERTY()
	TWeakObjectPtr<APlayerController> m_PlayerController = nullptr;

	//演出中フラグ
	bool m_IsActive = false;

	//現在の経過時間
	float m_CurrentTime = 0.0f;

	//移動にかける時間
	float m_MoveDuration = 0.0f;

	//パラメータキャッシュ
	//最終的な距離(通常の距離)
	float m_InitialArmLength = 0.0f;
	//開始時の距離(顔アップ)
	float m_StartArmLength = 0.0f;

	//最終的な回転
	FRotator m_InitialRotation = FRotator::ZeroRotator;
	//開始時の回転オフセット
	float m_StartYawOffset = 0.0f;

	//顔の位置に合わせるためのオフセット
	FVector m_InitialSocketOffset = FVector::ZeroVector;
	//顔の高さ(Z軸)へのオフセット
	FVector m_FaceSocketOffset = FVector::ZeroVector;

	//停止時間
	float m_WaitTime = 0.0f;

	//生成したWidgetを保持しておく変数
	UPROPERTY()
	UUserWidget* m_CreatedWidget = nullptr;

	//UIを消したかどうかのフラグ
	bool m_IsUiHidden = false;
};
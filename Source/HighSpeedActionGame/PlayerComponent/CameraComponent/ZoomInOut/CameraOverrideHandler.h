//担当
//伊藤直樹

//カメラのアーム長やFOVを一時的に上書きし補間するためのハンドラークラス


#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CameraOverrideHandler.generated.h"

//前方宣言
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraOverrideHandler : public UObject
{
	GENERATED_BODY()

public:
	//初期化(SpringArmなどへの参照をもらう)
	void Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	//Notifyから呼ばれる開始処理
	void StartOverride(float TargetLength, float TargetFOV, float Speed);

	//Notifyから呼ばれる終了処理
	void EndOverride();

	//毎フレームの更新処理(isActiveならtrueを返す)
	bool UpdateOverride(float DeltaTime);

	//現在オーバーライド中か？
	bool IsActive() const { return m_IsActive; }

private:
	//操作対象への弱い参照(クラッシュ防止のためWeakObjectPtr推奨だが、ポインタでも管理可)
	UPROPERTY()
	TWeakObjectPtr<USpringArmComponent> m_TargetSpringArm = nullptr;

	//操作対象カメラへの弱い参照
	UPROPERTY()
	TWeakObjectPtr<UCameraComponent> m_TargetCamera = nullptr;

	//状態管理
	bool m_IsActive = false;

	//パラメータ
	float m_TargetArmLength = 0.0f;
	float m_TargetFOV = 0.0f;
	float m_InterpSpeed = 0.0f;

	//どの値を上書きするか
	bool m_OverrideLength = false;
	bool m_OverrideFOV = false;
};
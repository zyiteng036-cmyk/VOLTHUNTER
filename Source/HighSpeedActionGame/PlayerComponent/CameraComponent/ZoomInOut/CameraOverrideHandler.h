//担当
//伊藤直樹

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CameraOverrideHandler.generated.h"


// 前方宣言
class USpringArmComponent;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraOverrideHandler : public UObject
{
	GENERATED_BODY()
public:
	// 初期化（SpringArmなどへの参照をもらう）
	void Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	// Notifyから呼ばれる開始処理
	void StartOverride(float TargetLength, float TargetFOV, float Speed);

	// Notifyから呼ばれる終了処理
	void EndOverride();

	// 毎フレームの更新処理（isActiveなら true を返す）
	bool UpdateOverride(float DeltaTime);

	// 現在オーバーライド中か？
	bool IsActive() const { return m_IsActive; }

private:
	// 操作対象への弱い参照（クラッシュ防止のためWeakObjectPtr推奨だが、ポインタでも管理可）
	UPROPERTY()
	USpringArmComponent* TargetSpringArm;

	UPROPERTY()
	UCameraComponent* TargetCamera;

	// 状態管理
	bool m_IsActive = false;

	// パラメータ
	float m_TargetArmLength;
	float m_TargetFOV;
	float m_InterpSpeed;

	// どの値を上書きするか
	bool m_OverrideLength;
	bool m_OverrideFOV;
};

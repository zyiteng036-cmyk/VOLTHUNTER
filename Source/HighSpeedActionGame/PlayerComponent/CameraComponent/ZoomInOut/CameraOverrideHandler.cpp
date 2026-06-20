
#include "CameraOverrideHandler.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

namespace CameraOverrideConstants
{
	constexpr float InvalidValueThreshold = 0.0f; //無効な値を判定するための閾値
}

//初期化(SpringArmなどへの参照をもらう)
void UCameraOverrideHandler::Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	//操作対象のコンポーネントを保持
	m_TargetSpringArm = InSpringArm;
	m_TargetCamera = InCamera;
}

//Notifyから呼ばれる開始処理
void UCameraOverrideHandler::StartOverride(float TargetLength, float TargetFOV, float Speed)
{
	//オーバーライド状態を有効化し補間スピードを設定
	m_IsActive = true;
	m_InterpSpeed = Speed;

	//距離の値のバリデーション(-1等の閾値未満なら変更なし)
	if (TargetLength >= CameraOverrideConstants::InvalidValueThreshold)
	{
		//目標距離を設定し上書きフラグを有効化
		m_TargetArmLength = TargetLength;
		m_OverrideLength = true;
	}
	else
	{
		//無効な値なら上書きしない
		m_OverrideLength = false;
	}

	//FOVのバリデーション(閾値以下なら変更なし)
	if (TargetFOV > CameraOverrideConstants::InvalidValueThreshold)
	{
		//目標FOVを設定し上書きフラグを有効化
		m_TargetFOV = TargetFOV;
		m_OverrideFOV = true;
	}
	else
	{
		//無効な値なら上書きしない
		m_OverrideFOV = false;
	}
}

//Notifyから呼ばれる終了処理
void UCameraOverrideHandler::EndOverride()
{
	//オーバーライド状態を無効化
	m_IsActive = false;
}

//毎フレームの更新処理(isActiveならtrueを返す)
bool UCameraOverrideHandler::UpdateOverride(float DeltaTime)
{
	//非アクティブ、または対象コンポーネントが無効な場合は処理を終了
	if (!m_IsActive || !m_TargetSpringArm.IsValid() || !m_TargetCamera.IsValid())
	{
		return false;
	}

	//距離の補間処理
	if (m_OverrideLength)
	{
		//現在の距離から目標距離へ補間計算
		float NewLen = FMath::FInterpTo(
			m_TargetSpringArm->TargetArmLength,
			m_TargetArmLength,
			DeltaTime,
			m_InterpSpeed
		);

		//計算結果を適用
		m_TargetSpringArm->TargetArmLength = NewLen;
	}

	//FOVの補間処理
	if (m_OverrideFOV)
	{
		//現在のFOVから目標FOVへ補間計算
		float NewFOV = FMath::FInterpTo(
			m_TargetCamera->FieldOfView,
			m_TargetFOV,
			DeltaTime,
			m_InterpSpeed
		);

		//計算結果を適用
		m_TargetCamera->SetFieldOfView(NewFOV);
	}

	//更新が実行されたためtrueを返す
	return true;
}
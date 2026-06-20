#include "CameraBossHandler.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

//初期化処理
void UCameraBossHandler::Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	//操作対象のコンポーネントを保持
	m_SpringArm = InSpringArm;
	m_Camera = InCamera;

	//非アクティブ状態で初期化
	m_IsActive = false;
}

//ボス戦カメラ開始
void UCameraBossHandler::StartBossCamera()
{
	//必要なコンポーネントが無効な場合は処理を抜ける
	if (!m_SpringArm.IsValid() || !m_Camera.IsValid()) return;

	//アクティブ状態をオンにする
	m_IsActive = true;

	//現在の値をデフォルト値としてキャッシュに保持
	m_DefaultArmLength = m_SpringArm->TargetArmLength;
	m_DefaultFOV = m_Camera->FieldOfView;
}

//ボス戦カメラ終了
void UCameraBossHandler::EndBossCamera()
{
	//アクティブ状態をオフにする
	m_IsActive = false;
}

//更新処理(trueを返している間は処理継続)
bool UCameraBossHandler::UpdateBossCamera(float DeltaTime)
{
	//非アクティブ、または対象コンポーネントが無効な場合は処理を抜ける
	if (!m_IsActive || !m_SpringArm.IsValid() || !m_Camera.IsValid()) return false;

	//ボス用のカメラ距離へ徐々に近づけるための補間計算と適用
	m_SpringArm->TargetArmLength = FMath::FInterpTo(
		m_SpringArm->TargetArmLength,
		m_BossArmLength,
		DeltaTime,
		m_TransitionSpeed
	);

	//ボス用のFOVへ徐々に近づけるための補間計算と適用
	m_Camera->SetFieldOfView(FMath::FInterpTo(
		m_Camera->FieldOfView,
		m_BossFOV,
		DeltaTime,
		m_TransitionSpeed
	));

	//更新が実行されたためtrueを返す
	return true;
}
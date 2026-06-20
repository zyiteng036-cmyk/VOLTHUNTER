#include "CameraDieHandler.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"

namespace CameraDieConstants
{
	constexpr float ZeroRollAngle = 0.0f; //ロール回転の初期化用ゼロ値
}

//初期化処理
void UCameraDieHandler::Initialize(USpringArmComponent* InSpringArm, APlayerController* InController)
{
	//操作対象のコンポーネントを保持
	m_SpringArm = InSpringArm;
	m_PlayerController = InController;
	m_IsActive = false;
}

//死亡演出開始
void UCameraDieHandler::StartDieCamera()
{
	//必要なコンポーネントが無効な場合は処理を抜ける
	if (!m_SpringArm.IsValid() || !m_PlayerController.IsValid()) return;

	//アクティブ状態をオンにする
	m_IsActive = true;

	//プレイヤーによるカメラ操作無効
	m_SpringArm->bUsePawnControlRotation = false;

	//死んだ瞬間のワールド座標を保存
	FVector CurrentWorldLoc = m_SpringArm->GetComponentLocation();

	//親(プレイヤー)の動きを完全に無視してその場に留まる
	m_SpringArm->SetUsingAbsoluteLocation(true);

	//親(プレイヤーの体)が倒れる回転の影響を完全に無視する(絶対回転化)
	m_SpringArm->SetUsingAbsoluteRotation(true);

	//保存しておいた死んだ瞬間の位置に引き戻して固定
	m_SpringArm->SetWorldLocation(CurrentWorldLoc);

	//カメラが床にぶつかってガクガク押し出されるのを防ぐ
	m_SpringArm->bDoCollisionTest = false;

	//カメラのラグを無効化し即座に追従させる
	m_SpringArm->bEnableCameraLag = false;
	m_SpringArm->bEnableCameraRotationLag = false;

	//距離を瞬時に変更
	m_SpringArm->TargetArmLength += m_DieZoomOutDistance;

	//コントローラーの回転を取得
	FRotator DieRot = m_PlayerController->GetControlRotation();

	//ピッチ角度を目標値に設定
	DieRot.Pitch = m_TargetPitch;

	//画面が斜めに傾かないように強制リセット
	DieRot.Roll = CameraDieConstants::ZeroRollAngle;

	//World(絶対)で回転をセットする
	m_SpringArm->SetWorldRotation(DieRot);
}

//死亡演出終了(元の状態に戻す)
void UCameraDieHandler::EndDieCamera()
{
	//アクティブじゃなければ何もしない
	if (!m_IsActive) return;

	//アクティブ状態をオフにする
	m_IsActive = false;

	//SpringArmが有効なら設定を一部元に戻す
	if (m_SpringArm.IsValid())
	{
		//絶対座標と絶対回転を解除し親への追従を再開
		m_SpringArm->SetUsingAbsoluteLocation(false);
		m_SpringArm->SetUsingAbsoluteRotation(false);

		//コリジョンテストを有効化
		m_SpringArm->bDoCollisionTest = true;
	}
}
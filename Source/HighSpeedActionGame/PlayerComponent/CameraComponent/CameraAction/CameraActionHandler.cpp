//設定でプロジェクトのコピーライト通知を記述してください

#include "CameraActionHandler.h"
#include "../../../PlayerCharacter.h"
#include "../../Player_MovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"

namespace CameraActionConstants
{
	//補間速度(InterpSpeed)
	constexpr float DefaultInterpSpeed = 10.0f;
	constexpr float ZoomOutInterpSpeed = 15.0f;
	constexpr float DashZoomInInterpSpeed = 8.0f;
	constexpr float NormalZoomInInterpSpeed = 5.0f;
	constexpr float DashStartInterpSpeed = 20.0f;

	//オートフレーミング関連
	constexpr float FramingSafeZone = 0.2f; //画面中心からの安全領域(この外に出たら引き始める)
	constexpr float ScreenEdgeThreshold = 0.45f; //これ以上画面端に行ったら最大ズームにする
	constexpr float MaxFramingZoom = 800.0f; //引く距離の最大値

	//マジックナンバー排除用定数
	constexpr float ZeroValue = 0.0f; //ゼロの定数
	constexpr float DistanceTolerance = 1.0f; //距離比較時の許容誤差
	constexpr float FramingOffsetThreshold = 10.0f; //ズームイン判定用のフレーミングオフセット閾値
	constexpr float ScreenCenter = 0.5f; //画面の中心座標(正規化用)
	constexpr int32 ScreenSizeMin = 0; //画面サイズの最小有効値
}

//初期化処理
void UCameraActionHandler::Initialize(APlayerCharacter* InPlayer, APlayerController* InController,
	USpringArmComponent* InSpringArm, UCameraComponent* InCamera,
	UPlayer_MovementComponent* InMovementComp)
{
	//操作対象や参照するコンポーネントを保持
	m_Player = InPlayer;
	m_PlayerController = InController;
	m_SpringArm = InSpringArm;
	m_Camera = InCamera;
	m_MovementComponent = InMovementComp;
}

//パラメータ設定用(ComponentのPlayerParamから値をコピーしてくる)
void UCameraActionHandler::SetupParams(float InDefaultLength, float InDefaultFOV,
	float InDashStartLength, float InDashStartFOV,
	float InDashMidLength, float InDashMidFOV)
{
	//受け取ったパラメータをメンバ変数に保存
	m_DefaultArmLength = InDefaultLength;
	m_DefaultFOV = InDefaultFOV;
	m_DashStartArmLength = InDashStartLength;
	m_DashStartFOV = InDashStartFOV;
	m_DashMidArmLength = InDashMidLength;
	m_DashMidFOV = InDashMidFOV;
}

//毎フレームの更新
void UCameraActionHandler::UpdateActionCamera(float DeltaTime)
{
	//必要なコンポーネントが無効な場合は処理を抜ける
	if (!m_MovementComponent.IsValid() || !m_SpringArm.IsValid() || !m_Camera.IsValid()) return;

	//目標値の設定(最初はデフォルト値を使用)
	float BaseTargetArmLength = m_DefaultArmLength;
	float TargetFOV = m_DefaultFOV;

	//ダッシュ状態の判定
	if (m_MovementComponent->GetIsDash())
	{
		if (m_MovementComponent->GetStartDash())
		{
			//ダッシュ開始時の目標値を設定
			BaseTargetArmLength = m_DashStartArmLength;
			TargetFOV = m_DashStartFOV;

			//初速寄せ完了判定(副作用:MovementComponentの状態を変更)
			if (FMath::IsNearlyEqual(m_SpringArm->TargetArmLength, BaseTargetArmLength, CameraActionConstants::DistanceTolerance))
			{
				m_MovementComponent->SetStartDash(false);
			}
		}
		else
		{
			//通常ダッシュ中の目標値を設定
			BaseTargetArmLength = m_DashMidArmLength;
			TargetFOV = m_DashMidFOV;
		}
	}

	//オートフレーミング値取得
	float FramingOffset = GetAutoFramingOffset(DeltaTime);

	//最終的な目標距離を算出
	float FinalTagetLength = BaseTargetArmLength + FramingOffset;

	//補間速度の決定(デフォルト値で初期化)
	float InterpSpeed = CameraActionConstants::DefaultInterpSpeed;

	if (FinalTagetLength > m_SpringArm->TargetArmLength)
	{
		//遠ざかる時は速く補間
		InterpSpeed = CameraActionConstants::ZoomOutInterpSpeed;
	}
	else
	{
		//近づく時
		if (FramingOffset < CameraActionConstants::FramingOffsetThreshold && m_MovementComponent->GetIsDash())
		{
			//ダッシュ中のズームイン
			InterpSpeed = CameraActionConstants::DashZoomInInterpSpeed;
		}
		else
		{
			//通常のズームイン
			InterpSpeed = CameraActionConstants::NormalZoomInInterpSpeed;
		}
	}

	//ダッシュ開始時はカメラ追従を早くする
	if (m_MovementComponent->GetStartDash())
	{
		InterpSpeed = CameraActionConstants::DashStartInterpSpeed;
	}

	//スプリングアームの距離へ補間を適用
	m_SpringArm->TargetArmLength = FMath::FInterpTo(
		m_SpringArm->TargetArmLength,
		FinalTagetLength,
		DeltaTime,
		InterpSpeed
	);

	//カメラのFOVへ補間を適用
	m_Camera->SetFieldOfView(FMath::FInterpTo(
		m_Camera->FieldOfView,
		TargetFOV,
		DeltaTime,
		InterpSpeed
	));
}

//オートフレーミング計算(privateに隠蔽)
float UCameraActionHandler::GetAutoFramingOffset(float DeltaTime)
{
	//必要なポインタが無効な場合はオフセット0を返す
	if (!m_PlayerController.IsValid() || !m_Player.IsValid()) return CameraActionConstants::ZeroValue;

	FVector PlayerLocation = m_Player->GetActorLocation();
	FVector2D ScreenLocation;

	//プレイヤーのワールド座標を画面上の2D座標に変換
	bool bOnScreen = m_PlayerController->ProjectWorldLocationToScreen(PlayerLocation, ScreenLocation);

	int32 SizeX, SizeY;
	m_PlayerController->GetViewportSize(SizeX, SizeY);

	//画面サイズが不正な場合は現在のオフセットを維持
	if (SizeX <= CameraActionConstants::ScreenSizeMin || SizeY <= CameraActionConstants::ScreenSizeMin) return m_AutoFramingOffset;

	//画面サイズで割って正規化(0.0~1.0)
	float NormX = ScreenLocation.X / static_cast<float>(SizeX);
	float NormY = ScreenLocation.Y / static_cast<float>(SizeY);

	//中心(0.5)からの距離を計算
	float DistX = FMath::Abs(NormX - CameraActionConstants::ScreenCenter);
	float DistY = FMath::Abs(NormY - CameraActionConstants::ScreenCenter);
	float MaxDist = FMath::Max(DistX, DistY);

	float TargetOffset = CameraActionConstants::ZeroValue;

	//画面端の判定
	if (!bOnScreen || MaxDist > CameraActionConstants::FramingSafeZone)
	{
		if (!bOnScreen)
		{
			//画面外に出た場合は最大まで引く
			TargetOffset = CameraActionConstants::MaxFramingZoom;
		}
		else
		{
			//画面端に近づくにつれて徐々に引く距離を増やす
			TargetOffset = FMath::GetMappedRangeValueClamped(
				FVector2D(CameraActionConstants::FramingSafeZone, CameraActionConstants::ScreenEdgeThreshold),
				FVector2D(CameraActionConstants::ZeroValue, CameraActionConstants::MaxFramingZoom),
				MaxDist
			);
		}
	}

	return TargetOffset;
}
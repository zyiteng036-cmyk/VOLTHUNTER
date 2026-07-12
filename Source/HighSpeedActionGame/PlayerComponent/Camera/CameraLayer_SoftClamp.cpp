#include "CameraLayer_SoftClamp.h"
#include "../../DataAssets/Player/PlayerCameraParameter.h"
#include "../Solver/CameraSoftClampSolver.h"
#include "GameFramework/PlayerController.h"

//レイヤー処理を適用
void UCameraLayer_SoftClamp::ApplyLayer(FCameraContext& Context)
{
	if (!CanApplyLayer(Context)) return;

	//Boss戦中はBoss専用構図を優先
	if (Context.bIsBossBattle) return;

	InitializeSolver();

	if (!m_SoftClampSolver || !Context.OwnerActor || !Context.CameraParam) return;

	//専用演出カメラ中は画面内補正を停止
	if (ShouldSkipForCameraRequest(Context)) return;

	APlayerController* PlayerController = Context.OwnerActor->GetInstigatorController<APlayerController>();
	if (!PlayerController) return;

	FVector RealCameraLocation = FVector::ZeroVector;
	FRotator RealCameraRotation = FRotator::ZeroRotator;
	PlayerController->GetPlayerViewPoint(RealCameraLocation, RealCameraRotation);

	//実際のカメラ位置から画面内へ戻すオフセットを計算
	const FVector ClampOffset = m_SoftClampSolver->EvaluateClampOffset(
		Context.OwnerActor,
		RealCameraLocation,
		RealCameraRotation,
		Context.FinalFOV,
		GetScreenOffset(Context),
		GetDeadzone(Context)
	);

	if (ClampOffset.IsNearlyZero()) return;

	//TargetOffsetではなく最終ワールド補正へ加算
	Context.HardClampOffset += ClampOffset;
}

//レイヤー名を取得
FName UCameraLayer_SoftClamp::GetLayerName() const
{
	return TEXT("CameraLayer_SoftClamp");
}

//SoftClampソルバーを初期化
void UCameraLayer_SoftClamp::InitializeSolver()
{
	if (m_SoftClampSolver) return;

	m_SoftClampSolver = NewObject<UCameraSoftClampSolver>(this);
}

//使用する画面中心オフセットを取得
FVector2D UCameraLayer_SoftClamp::GetScreenOffset(const FCameraContext& Context) const
{
	if (!Context.CameraParam) return FVector2D::ZeroVector;

	return Context.bIsBossBattle
		? Context.CameraParam->BossScreenOffset
		: Context.CameraParam->NormalScreenOffset;
}

//使用するデッドゾーンを取得
FVector2D UCameraLayer_SoftClamp::GetDeadzone(const FCameraContext& Context) const
{
	if (!Context.CameraParam) return FVector2D::ZeroVector;

	return Context.bIsBossBattle
		? Context.CameraParam->BossDeadzone
		: Context.CameraParam->NormalDeadzone;
}

//SoftClampを停止するカメラ要求か
bool UCameraLayer_SoftClamp::ShouldSkipForCameraRequest(const FCameraContext& Context) const
{
	if (!Context.CameraRequest.bIsActive) return false;

	switch (Context.CameraRequest.RequestType)
	{
	case ECameraRequestType::Opening:
	case ECameraRequestType::Clear:
	case ECameraRequestType::Die:
		return true;

	default:
		return false;
	}
}
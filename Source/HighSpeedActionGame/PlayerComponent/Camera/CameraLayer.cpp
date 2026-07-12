#include "CameraLayer.h"

//初期化
UCameraLayer::UCameraLayer()
{
}

//レイヤー処理を適用
void UCameraLayer::ApplyLayer(FCameraContext& Context)
{
}

//レイヤー有効状態を設定
void UCameraLayer::SetLayerActive(bool bActive)
{
	m_bLayerActive = bActive;
}

//レイヤー名を取得
FName UCameraLayer::GetLayerName() const
{
	return GetClass() ? GetClass()->GetFName() : NAME_None;
}

//レイヤー処理可能か
bool UCameraLayer::CanApplyLayer(const FCameraContext& Context) const
{
	if (!m_bLayerActive) return false;
	if (!Context.World) return false;
	if (!Context.OwnerActor) return false;
	if (!Context.CameraParam) return false;

	return true;
}
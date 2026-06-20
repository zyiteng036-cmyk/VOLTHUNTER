#include "PlayerNotifySubsystem.h"

//ジャスト回避成功を通知
void UPlayerNotifySubsystem::NotifyJustEvasive(const AActor* Attacker)
{
	//デリゲートにイベントがバインドされているか確認
	if (m_OnJustEvasiveOccurred.IsBound())
	{
		//バインドされているすべてのリスナーにイベントを配信
		m_OnJustEvasiveOccurred.Broadcast(Attacker);
	}
}

//プレイヤーが瀕死状態になったことを通知
void UPlayerNotifySubsystem::NotifyPlayerDying(AActor* DyingActor)
{
	//デリゲートにイベントがバインドされているか確認
	if (m_OnPlayerDyingOccurred.IsBound())
	{
		//バインドされているすべてのリスナーにイベントを配信
		m_OnPlayerDyingOccurred.Broadcast(DyingActor);
	}
}

//プレイヤー死亡時に呼び出されるイベント
void UPlayerNotifySubsystem::NotifyPlayerDied(AActor* DeadActor)
{
	//デリゲートにイベントがバインドされているか確認
	if (m_OnPlayerDiedOccurred.IsBound())
	{
		//バインドされているすべてのリスナーにイベントを配信
		m_OnPlayerDiedOccurred.Broadcast(DeadActor);
	}
}
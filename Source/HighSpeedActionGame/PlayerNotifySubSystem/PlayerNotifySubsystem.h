//担当
//伊藤直樹
// 
//プレイヤーの状態を通知するクラス
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PlayerNotifySubsystem.generated.h"

//デリゲート定義
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJustEvasiveDelegate, const AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDyingDelegate_Sub, AActor*, DyingActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedDelegate_Sub, AActor*, DeadActor);

UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayerNotifySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	//ジャスト回避成功を通知
	UFUNCTION(BlueprintCallable, Category = "PlayerNotify")
	void NotifyJustEvasive(const AActor* Attacker);

	//プレイヤーが瀕死状態になったことを通知
	UFUNCTION(BlueprintCallable, Category = "PlayerNotify")
	void NotifyPlayerDying(AActor* DyingActor);

	//死亡を通知
	UFUNCTION(BlueprintCallable, Category = "PlayerNotify")
	void NotifyPlayerDied(AActor* DeadActor);

	//---BPが受け取るためのイベント---

	//ジャスト回避発生時に呼び出されるイベント
	UPROPERTY(BlueprintAssignable, Category = "PlayerNotify|Event")
	FOnJustEvasiveDelegate m_OnJustEvasiveOccurred;

	//プレイヤー瀕死時に呼び出されるイベント
	UPROPERTY(BlueprintAssignable, Category = "PlayerNotify|Event")
	FOnPlayerDyingDelegate_Sub m_OnPlayerDyingOccurred;

	//プレイヤー死亡時に呼び出されるイベント
	UPROPERTY(BlueprintAssignable, Category = "PlayerNotify|Event")
	FOnPlayerDiedDelegate_Sub m_OnPlayerDiedOccurred;
};
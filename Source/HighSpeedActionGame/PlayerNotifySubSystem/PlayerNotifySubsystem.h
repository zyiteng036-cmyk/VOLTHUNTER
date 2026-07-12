//担当
//伊藤直樹

//プレイヤーの状態を通知するクラス

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PlayerNotifySubsystem.generated.h"


// BPでイベントを受け取るためのデリゲート定義
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJustEvasiveDelegate, const AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDyingDelegate_Sub, AActor*, DyingActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedDelegate_Sub, AActor*, DeadActor);
/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayerNotifySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	// ジャスト回避成功を通知
	UFUNCTION(BlueprintCallable, Category = "PlayerNotify")
	void NotifyJustEvasive(const AActor* Attacker);

	UFUNCTION(BlueprintCallable, Category = "PlayerNotify")
	void NotifyPlayerDying(AActor* DyingActor);

	// 死亡を通知
	UFUNCTION(BlueprintCallable, Category = "PlayerNotify")
	void NotifyPlayerDied(AActor* DeadActor);

	// --- BPが受け取るためのイベント ---

	UPROPERTY(BlueprintAssignable, Category = "PlayerNotify|Event")
	FOnJustEvasiveDelegate OnJustEvasiveOccurred;

	UPROPERTY(BlueprintAssignable, Category = "PlayerNotify|Event")
	FOnPlayerDyingDelegate_Sub OnPlayerDyingOccurred;

	UPROPERTY(BlueprintAssignable, Category = "PlayerNotify|Event")
	FOnPlayerDiedDelegate_Sub OnPlayerDiedOccurred;
};

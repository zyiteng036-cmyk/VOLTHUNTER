//担当
//伊藤直樹

//ポストプロセスクラス

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PostProsess.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UPostProsess : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// 外部（スキルなど）から呼ぶ関数
	void SetPostProsessActive(bool _Active);

	AActor* FindPostProsessActor();
private:
	// Actorを見つけたらここに保存
	TWeakObjectPtr<AActor> CachedEffectActor;

};

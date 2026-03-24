//佐々木奏太担当
//死亡通知クラス

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EnemyAnimNotifyDeath.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UEnemyAnimNotifyDeath : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};

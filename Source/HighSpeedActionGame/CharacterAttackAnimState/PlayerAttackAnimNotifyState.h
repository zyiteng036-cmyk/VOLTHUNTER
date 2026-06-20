// 担当佐々木奏太
//攻撃判定クラスをプレイヤーように強化攻撃を判断できるように変更

#pragma once

#include "CoreMinimal.h"
#include "CharacterAttackAnimNotifyState.h"
#include "PlayerAttackAnimNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayerAttackAnimNotifyState : public UCharacterAttackAnimNotifyState
{
	GENERATED_BODY()

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
};

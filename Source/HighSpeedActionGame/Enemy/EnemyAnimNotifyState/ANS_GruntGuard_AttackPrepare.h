// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_GruntGuard_AttackPrepare.generated.h"

class AEnemy_GruntGuard;

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UANS_GruntGuard_AttackPrepare : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	AEnemy_GruntGuard* m_GruntGuard;
};

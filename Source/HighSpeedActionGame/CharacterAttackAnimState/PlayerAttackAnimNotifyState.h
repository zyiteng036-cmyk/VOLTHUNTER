// Fill out your copyright notice in the Description page of Project Settings.

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

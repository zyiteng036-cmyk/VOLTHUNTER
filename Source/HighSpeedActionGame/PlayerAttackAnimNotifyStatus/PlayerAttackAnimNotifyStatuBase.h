// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PlayerAttackAnimNotifyStatuBase.generated.h"

class AAttackCollisionDetection;

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayerAttackAnimNotifyStatuBase : public UAnimNotifyState
{
	GENERATED_BODY()
public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    //攻撃開始時の共通の処理
    void AttackStart();

    //攻撃終了時の共通の処理
    void AttackEnd();
protected:
    TArray<AAttackCollisionDetection*> m_UseAttackCollisionDetections;//使用する衝突判定の配列
};

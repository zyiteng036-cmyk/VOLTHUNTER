// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NiagaraSocketAnimNotifyState.generated.h"


class UNiagaraSystem;
class UNiagaraComponent;

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UNiagaraSocketAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

    // Niagaraエフェクトアセット
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    UNiagaraSystem* NiagaraSystem;

    // 追従させるソケット名
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FName SocketName = NAME_None;

    // オフセット位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FVector LocationOffset = FVector::ZeroVector;

    // オフセット回転
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FRotator RotationOffset = FRotator::ZeroRotator;
private:
    UPROPERTY(Transient)
    UNiagaraComponent* SpawnedNiagaraComponent = nullptr;
};

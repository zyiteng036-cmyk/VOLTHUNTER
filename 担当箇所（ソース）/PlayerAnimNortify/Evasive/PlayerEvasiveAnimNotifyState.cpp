// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerEvasiveAnimNotifyState.h"
#include "../../PlayerComponent/Player_EvasiveComponent.h"
#include "../../PlayerCharacter.h"

void UPlayerEvasiveAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        if (UPlayer_EvasiveComponent* EvasiveComp = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>())
        {
            EvasiveComp->CreateJustEvasiveCollision(); //回避コリジョン生成
        }
    }
}

void UPlayerEvasiveAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        if (UPlayer_EvasiveComponent* EvasiveComp = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>())
        {
            EvasiveComp->DestroyJustEvasiveCollision(); //回避コリジョン破棄
        }
    }
}

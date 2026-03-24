// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyPlaySE.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

void UAnimNotifyPlaySE::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) {
    if (m_SoundToPlay && MeshComp && MeshComp->GetOwner())
    {
        UGameplayStatics::PlaySoundAtLocation(MeshComp->GetOwner()->GetWorld(), m_SoundToPlay, MeshComp->GetComponentLocation());
    }
}


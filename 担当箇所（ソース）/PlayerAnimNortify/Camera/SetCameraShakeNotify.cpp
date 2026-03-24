// Fill out your copyright notice in the Description page of Project Settings.


#include "SetCameraShakeNotify.h"

void USetCameraShakeNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!CameraShakeClass || !MeshComp) return;

    if (APlayerController* PC = MeshComp->GetWorld()->GetFirstPlayerController())
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraShake(CameraShakeClass, ShakeScale);
        }
    }
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "HitNotifyState.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "../../PlayerComponent/Player_EvasiveComponent.h"
#include "../../PlayerCharacter.h"

void UHitNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    bHitTriggered = false;
    ActiveCameraShake = nullptr;
}

void UHitNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
    if (bHitTriggered || !MeshComp)
    {
        return;
    }

    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
    if (!PlayerCharacter)
    {
        return;
    }

    if (!PlayerCharacter->GetIsHit())
    {
        return;
    }

    bHitTriggered = true;

    // ===== Sound =====
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            PlayerCharacter,
            HitSound,
            PlayerCharacter->GetActorLocation()
        );
    }

    // ===== Effect =====
    if (HitEffect)
    {
        if (bAttachToMesh)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                HitEffect,
                MeshComp,
                AttachSocketName,
                EffectLocationOffset,
                EffectRotationOffset,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }
        else
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                MeshComp->GetWorld(),
                HitEffect,
                MeshComp->GetComponentLocation() + EffectLocationOffset,
                EffectRotationOffset
            );
        }
    }

    // ===== CameraShake =====
    if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
    {
        if (PC->PlayerCameraManager && CameraShakeClass)
        {
            ActiveCameraShake =
                PC->PlayerCameraManager->StartCameraShake(
                    CameraShakeClass,
                    CameraShakeScale
                );
        }
    }

    // ===== âÒîâã÷=====
    if (UPlayer_EvasiveComponent* EvasiveComp =
        PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>())
    {
        EvasiveComp->SetCanEvasive(true);
    }
}

void UHitNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    // ÉqÉbÉgÇµÇƒÇ¢Ç»Ç¢èÍçáÇÃÇ› NoHitSound Çñ¬ÇÁÇ∑
    if (bHitTriggered)
    {
        return;
    }

    if (!MeshComp)
    {
        return;
    }

    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
    if (!PlayerCharacter)
    {
        return;
    }

    APlayerController* PC =
        Cast<APlayerController>(PlayerCharacter->GetController());
    if (!PC || !PC->PlayerCameraManager)
    {
        return;
    }

    // ===== CameraShake í‚é~ =====
    if (ActiveCameraShake)
    {
        PC->PlayerCameraManager->StopCameraShake(
            ActiveCameraShake,
            false   // false = ë¶í‚é~
        );

        ActiveCameraShake = nullptr;
    }

    // ===== ÉqÉbÉgÇµÇƒÇ¢Ç»Ç¢èÍçáÇÃÇ› NoHitSound =====
    if (NoHitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            PlayerCharacter,
            NoHitSound,
            PlayerCharacter->GetActorLocation()
        );
    }
}

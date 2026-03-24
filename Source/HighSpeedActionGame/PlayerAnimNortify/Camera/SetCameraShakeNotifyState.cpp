

#include "SetCameraShakeNotifyState.h"
#include "NiagaraFunctionLibrary.h"
#include "../../PlayerComponent/Player_AttackComponent.h"
#include "../../PlayerCharacter.h"



void USetCameraShakeNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	bTriggered = false;
}

void USetCameraShakeNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
    if (bTriggered)
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

    UPlayer_AttackComponent* AttackComp =
        PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>();
    if (!AttackComp)
    {
        return;
    }

    const FPlayerParam& PlayerParam = PlayerCharacter->GetPlayerParam();

    // 条件成立チェック
    if (AttackComp->GetCurrentHeavyChargeTime() < PlayerParam.HeavyChargeLong)
    {
        return;
    }

    // ====== カメラシェイク ======
    if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
    {
        if (PC->PlayerCameraManager && CameraShakeClass)
        {
            PC->PlayerCameraManager->StartCameraShake(CameraShakeClass, ShakeScale);
        }
    }

    // ====== エフェクト ======
    if (Effect)
    {
        const FVector SpawnLocation =
            EffectSocketName != NAME_None
            ? MeshComp->GetSocketLocation(EffectSocketName)
            : PlayerCharacter->GetActorLocation();

        SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            MeshComp->GetWorld(),
            Effect,
            SpawnLocation + EffectOffset
        );
    }

    bTriggered = true;
}


void USetCameraShakeNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	bTriggered = false;

    if (SpawnedEffect)
    {
        SpawnedEffect->Deactivate();

        SpawnedEffect = nullptr;
    }
}

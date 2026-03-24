//’S“–
//ˆÉ“¡’¼Ž÷

//Hit‚µ‚½‚Æ‚«ƒNƒ‰ƒX

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "HitNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UHitNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation, float TotalDuration) override;

    virtual void NotifyTick(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation, float FrameDeltaTime) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation) override;

protected:
    // ===== ‹¤’Ê =====
    bool bHitTriggered = false;

    // ===== Sound =====
    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* HitSound = nullptr;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* NoHitSound = nullptr;

    // ===== Effect =====
    UPROPERTY(EditAnywhere, Category = "Effect")
    UNiagaraSystem* HitEffect = nullptr;

    UPROPERTY(EditAnywhere, Category = "Effect")
    bool bAttachToMesh = true;

    UPROPERTY(EditAnywhere, Category = "Effect")
    FName AttachSocketName = NAME_None;

    UPROPERTY(EditAnywhere, Category = "Effect")
    FVector EffectLocationOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Effect")
    FRotator EffectRotationOffset = FRotator::ZeroRotator;

    // ===== Camera =====
    UPROPERTY(EditAnywhere, Category = "Camera")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    UPROPERTY()
    TObjectPtr<UCameraShakeBase> ActiveCameraShake;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraShakeScale = 1.f;
};

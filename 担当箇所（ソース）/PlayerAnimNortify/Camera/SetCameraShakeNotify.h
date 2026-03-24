//’S“–
//ˆÉ“¡’¼Ž÷

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SetCameraShakeNotify.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API USetCameraShakeNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, Category = "CameraShake")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    UPROPERTY(EditAnywhere, Category = "CameraShake")
    float ShakeScale = 1.0f;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};

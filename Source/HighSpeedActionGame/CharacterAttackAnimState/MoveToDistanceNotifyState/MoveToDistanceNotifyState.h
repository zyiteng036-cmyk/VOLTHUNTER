// 担当：佐々木奏太
//指定の距離へNotifyStateが続く間ちょうどで着くように進むクラス

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MoveToDistanceNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UMoveToDistanceNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float DeltaTime)override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float m_Distance = 100.f;//進む距離

	float m_Speed = 0.f;

	FVector m_OffsetVelocity = FVector::ZeroVector;

	FVector m_ForwardVector = FVector::ZeroVector;

	TWeakObjectPtr<ACharacter> m_CachedCharacter = nullptr;
};

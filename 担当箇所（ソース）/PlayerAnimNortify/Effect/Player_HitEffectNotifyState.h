//担当
//伊藤直樹

//Hit時のエフェクト発動クラス

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NiagaraSystem.h"
#include "Player_HitEffectNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayer_HitEffectNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:

	
	// NotifyState開始
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	// Tick	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	// NotifyState終了
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	bool m_PrevHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitEffect")
	UNiagaraSystem* HitNiagara;

	// アタッチするソケット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitEffect")
	FName AttachSocketName = NAME_None;

	// 位置オフセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitEffect")
	FVector LocationOffset = FVector::ZeroVector;

	// 回転オフセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitEffect")
	FRotator RotationOffset = FRotator::ZeroRotator;

	// アタッチするか
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitEffect")
	bool m_AttachToMesh = true;
};

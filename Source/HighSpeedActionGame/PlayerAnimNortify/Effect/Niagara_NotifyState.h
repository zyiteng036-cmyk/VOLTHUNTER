//担当
//伊藤直樹

//Niagaraを発動させるクラス

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Niagara_NotifyState.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UNiagara_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	// NotifyState開始
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	
	// NotifyState終了
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
public:
	//再生するNiagaraエフェクト
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	UNiagaraSystem* NiagaraSystem;

	// アタッチするソケット名
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	FName AttachSocketName;

	// 位置オフセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	FVector LocationOffset;

	// 回転オフセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	FRotator RotationOffset;

private:
	// Begin と End を結びつけるための保持用
	UPROPERTY()
	UNiagaraComponent* SpawnedNiagara;
};

//担当
//伊藤直樹

//-----------------------------------------------------
//Niagara再生NotifyState
//
//指定ソケットへNiagaraを生成し、NotifyState終了時に停止する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Niagara_NotifyState.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class HIGHSPEEDACTIONGAME_API UNiagara_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//Niagaraエフェクトを生成
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//生成したNiagaraエフェクトを停止
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	//再生するNiagaraエフェクト
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	//エフェクトを取り付けるソケット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	FName AttachSocketName = NAME_None;

	//ソケットからの位置オフセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	FVector LocationOffset = FVector::ZeroVector;

	//ソケットからの回転オフセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara")
	FRotator RotationOffset = FRotator::ZeroRotator;

private:
	//NotifyState中に生成したNiagaraComponent
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SpawnedNiagara = nullptr;
};
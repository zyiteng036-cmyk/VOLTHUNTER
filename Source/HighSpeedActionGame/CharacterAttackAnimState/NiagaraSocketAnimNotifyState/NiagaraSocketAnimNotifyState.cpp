// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraSocketAnimNotifyState.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"

void UNiagaraSocketAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	if (!MeshComp || !NiagaraSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("NiagaraSocketAnimNotifyState::NotifyBegin - Invalid MeshComp or NiagaraSystem"));
		return;
	}

	// Niagaraシステムをソケットにアタッチしてスポーン
	SpawnedNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,
		MeshComp,
		SocketName,
		LocationOffset,
		RotationOffset,
		EAttachLocation::SnapToTarget,
		true,       // bAutoDestroy
		true        // bAutoActivate
	);

	if (!SpawnedNiagaraComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("NiagaraSocketAnimNotifyState::NotifyBegin - Failed to spawn Niagara system on socket %s"), *SocketName.ToString());
	}
}

void UNiagaraSocketAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (!SpawnedNiagaraComponent) { return; }

	if (SpawnedNiagaraComponent)
	{
		SpawnedNiagaraComponent->Deactivate();
		SpawnedNiagaraComponent->DestroyComponent();
		SpawnedNiagaraComponent = nullptr;
	}
}
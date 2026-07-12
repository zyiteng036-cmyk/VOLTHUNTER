#include "Niagara_NotifyState.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

//Niagaraエフェクトを生成
void UNiagara_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !NiagaraSystem) return;

	//所有Actorが存在しないMeshではエフェクトを生成しない
	if (!MeshComp->GetOwner()) return;

	//指定ソケットへ相対位置と相対回転を維持して取り付ける
	SpawnedNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,
		MeshComp,
		AttachSocketName,
		LocationOffset,
		RotationOffset,
		EAttachLocation::KeepRelativeOffset,
		false
	);
}

//生成したNiagaraエフェクトを停止
void UNiagara_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!SpawnedNiagara) return;

	//NotifyState終了後も残らないように停止
	SpawnedNiagara->Deactivate();
	SpawnedNiagara = nullptr;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_GruntGuard_AttackPrepare.h"
#include "../Enemy_GruntGuard.h"

void UANS_GruntGuard_AttackPrepare::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) {
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	m_GruntGuard = Cast<AEnemy_GruntGuard>(Owner);
	if (!m_GruntGuard) return;
	// UŒ‚€”õ’†ƒtƒ‰ƒOtrue
	m_GruntGuard->SetIsPreparingAttack(true);
}
void UANS_GruntGuard_AttackPrepare::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) {
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!m_GruntGuard) return;

	// UŒ‚€”õ’†ƒtƒ‰ƒOfalse
	m_GruntGuard->SetIsPreparingAttack(false);
	m_GruntGuard = nullptr;
}
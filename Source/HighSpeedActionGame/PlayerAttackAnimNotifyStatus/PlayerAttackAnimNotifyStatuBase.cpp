// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttackAnimNotifyStatuBase.h"
#include "../AttackCollisionDetection/AttackCollisionDetection.h"


void UPlayerAttackAnimNotifyStatuBase::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyBegin: Enabling attack collision for %s"), *MeshComp->GetOwner()->GetName());

	}
}

void UPlayerAttackAnimNotifyStatuBase::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyTick: %s ticking, DeltaTime: %f"), *MeshComp->GetOwner()->GetName(), FrameDeltaTime);

		// 毎フレーム必要な処理があればここで行う
	}
}

void UPlayerAttackAnimNotifyStatuBase::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyEnd: Disabling attack collision for %s"), *MeshComp->GetOwner()->GetName());


	}
}

void UPlayerAttackAnimNotifyStatuBase::AttackStart() {

}

void UPlayerAttackAnimNotifyStatuBase::AttackEnd() {
	for (int i = 0; i < m_UseAttackCollisionDetections.Num(); ++i) {
		if (!m_UseAttackCollisionDetections[i])continue;
		m_UseAttackCollisionDetections[i]->SetActive(false);
	}

	m_UseAttackCollisionDetections.Empty();
}
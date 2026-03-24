// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyToggleVisibility.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyToggleVisibility::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animtion)
{
	if (MeshComp) {
		MeshComp->SetVisibility(m_ShowMesh, true);
	}
}

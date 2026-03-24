// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_SetVisibility.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraComponent.h"
#include "../../PlayerSword/PlayerSword.h"

void UPlayer_SetVisibility::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (!MeshComp) return;

	// プレイヤー本体
	MeshComp->SetVisibility(false,false);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		TArray<AActor*> AttachedActors;
		OwnerActor->GetAttachedActors(AttachedActors);

		for (AActor* AttachedActor : AttachedActors)
		{
			if (APlayerSword* Sword = Cast<APlayerSword>(AttachedActor))
			{
				Sword->SetSwordMeshVisibility(false);
			}
		}
	}
}

void UPlayer_SetVisibility::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	MeshComp->SetVisibility(true, false);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		TArray<AActor*> AttachedActors;
		OwnerActor->GetAttachedActors(AttachedActors);

		for (AActor* AttachedActor : AttachedActors)
		{
			if (APlayerSword* Sword = Cast<APlayerSword>(AttachedActor))
			{
				Sword->SetSwordMeshVisibility(true);
			}
		}
	}
}

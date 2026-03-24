// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_SwordEffect.h"
#include "../../PlayerSword/PlayerSword.h"

void UPlayer_SwordEffect::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp) return;

	if (!MeshComp || !TargetSwordClass) return;

	TArray<USceneComponent*> Children;
	MeshComp->GetChildrenComponents(true, Children);

	for (USceneComponent* Comp : Children)
	{
		APlayerSword* Sword = Cast<APlayerSword>(Comp->GetOwner());
		if (!Sword) continue;

		// クラス一致チェック
		if (Sword->IsA(TargetSwordClass))
		{
			Sword->SetAttackActive(true);
		}
	}
}

void UPlayer_SwordEffect::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !TargetSwordClass) return;

	TArray<USceneComponent*> Children;
	MeshComp->GetChildrenComponents(true, Children);

	for (USceneComponent* Comp : Children)
	{
		APlayerSword* Sword = Cast<APlayerSword>(Comp->GetOwner());
		if (!Sword) continue;

		if (Sword->IsA(TargetSwordClass))
		{
			Sword->SetAttackActive(false);
		}
	}
}

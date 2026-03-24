// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimNotifyDeath.h"
#include "../EnemyBase.h"

void UEnemyAnimNotifyDeath::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) {
	if (!MeshComp) return;
	AEnemyBase* Enemy = Cast<AEnemyBase>(MeshComp->GetOwner());
	if (Enemy)
	{
		Enemy->OnDeath(); //éÄñSèàóù
	}
}
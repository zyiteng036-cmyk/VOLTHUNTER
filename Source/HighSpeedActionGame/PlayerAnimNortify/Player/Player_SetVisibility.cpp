// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_SetVisibility.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraComponent.h"
#include "../../PlayerSword/PlayerSword.h"

void UPlayer_SetVisibility::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	//メッシュコンポーネントが無効な場合は処理を抜ける
	if (!MeshComp) return;

	//プレイヤー本体を非表示に設定
	MeshComp->SetVisibility(false,false);

	//オーナーアクターを取得してアタッチされているアクタを走査
	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		TArray<AActor*> AttachedActors;
		OwnerActor->GetAttachedActors(AttachedActors);

		for (AActor* AttachedActor : AttachedActors)
		{
			//アタッチされているアクタが武器(APlayerSword)であれば非表示にする
			if (APlayerSword* Sword = Cast<APlayerSword>(AttachedActor))
			{
				Sword->SetSwordMeshVisibility(false);
			}
		}
	}
}

void UPlayer_SetVisibility::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//メッシュコンポーネントが無効な場合は処理を抜ける
	if (!MeshComp) return;

	//プレイヤー本体を表示に設定
	MeshComp->SetVisibility(true, false);

	//オーナーアクターを取得してアタッチされているアクタを走査
	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		TArray<AActor*> AttachedActors;
		OwnerActor->GetAttachedActors(AttachedActors);

		for (AActor* AttachedActor : AttachedActors)
		{
			//アタッチされているアクタが武器(APlayerSword)であれば表示にする
			if (APlayerSword* Sword = Cast<APlayerSword>(AttachedActor))
			{
				Sword->SetSwordMeshVisibility(true);
			}
		}
	}
}

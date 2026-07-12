#include "Player_SetVisibility.h"
#include "../../PlayerSword/PlayerSword.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

//プレイヤーと剣を非表示にする
void UPlayer_SetVisibility::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	SetPlayerVisibility(MeshComp, false);
}

//プレイヤーと剣を再表示する
void UPlayer_SetVisibility::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	SetPlayerVisibility(MeshComp, true);
}

//プレイヤー本体と装備中の剣の表示状態を切り替える
void UPlayer_SetVisibility::SetPlayerVisibility(USkeletalMeshComponent* MeshComp, bool bVisible) const
{
	if (!MeshComp) return;

	//プレイヤー本体のMeshだけを切り替える
	MeshComp->SetVisibility(bVisible, false);

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	TArray<AActor*> AttachedActors;
	OwnerActor->GetAttachedActors(AttachedActors);

	//プレイヤーに取り付けられている剣も同じ表示状態にする
	for (AActor* AttachedActor : AttachedActors)
	{
		APlayerSword* Sword = Cast<APlayerSword>(AttachedActor);
		if (!Sword) continue;

		Sword->SetSwordMeshVisibility(bVisible);
	}
}
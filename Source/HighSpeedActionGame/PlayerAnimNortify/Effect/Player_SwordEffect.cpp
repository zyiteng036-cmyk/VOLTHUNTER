#include "Player_SwordEffect.h"
#include "../../PlayerSword/PlayerSword.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

//剣の攻撃エフェクトを有効化
void UPlayer_SwordEffect::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	SetSwordAttackActive(MeshComp, true);
}

//剣の攻撃エフェクトを無効化
void UPlayer_SwordEffect::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	SetSwordAttackActive(MeshComp, false);
}

//対象の剣Actorへ攻撃状態を設定
void UPlayer_SwordEffect::SetSwordAttackActive(USkeletalMeshComponent* MeshComp, bool bActive) const
{
	if (!MeshComp || !TargetSwordClass) return;

	TArray<USceneComponent*> ChildComponents;
	MeshComp->GetChildrenComponents(true, ChildComponents);

	for (USceneComponent* ChildComponent : ChildComponents)
	{
		if (!ChildComponent) continue;

		APlayerSword* Sword = Cast<APlayerSword>(ChildComponent->GetOwner());
		if (!Sword) continue;

		//Notifyで指定された剣クラスだけを切り替える
		if (Sword->IsA(TargetSwordClass))
		{
			Sword->SetAttackActive(bActive);
		}
	}
}
#include "PlayerEvasiveAnimNotifyState.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerEvasiveComponent.h"
#include "Components/SkeletalMeshComponent.h"

//ジャスト回避判定を開始
void UPlayerEvasiveAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UPlayerEvasiveComponent* EvasiveComponent = FindEvasiveComponent(MeshComp);
	if (!EvasiveComponent) return;

	//NotifyState中に使用するジャスト回避判定を生成
	EvasiveComponent->CreateJustEvasiveCollision();
}

//ジャスト回避判定を終了
void UPlayerEvasiveAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UPlayerEvasiveComponent* EvasiveComponent = FindEvasiveComponent(MeshComp);
	if (!EvasiveComponent) return;

	//NotifyState終了時にジャスト回避判定を破棄
	EvasiveComponent->DestroyJustEvasiveCollision();
}

//Mesh所有者から回避コンポーネントを取得
UPlayerEvasiveComponent* UPlayerEvasiveAnimNotifyState::FindEvasiveComponent(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp) return nullptr;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return nullptr;

	return PlayerCharacter->FindComponentByClass<UPlayerEvasiveComponent>();
}
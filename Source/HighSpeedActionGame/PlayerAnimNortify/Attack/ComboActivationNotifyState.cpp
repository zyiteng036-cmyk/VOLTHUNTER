#include "ComboActivationNotifyState.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerAttackComponent.h"
#include "Components/SkeletalMeshComponent.h"

//攻撃発動受付を開始
void UComboActivationNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UPlayerAttackComponent* AttackComponent = FindAttackComponent(MeshComp);
	if (!AttackComponent) return;

	//ジャスト回避反撃専用の発動受付を開く
	if (WindowMode == EAttackInputWindowMode::JustEvasiveCounter)
	{
		AttackComponent->OpenJustEvasiveCounterActivationWindow();
		return;
	}

	//通常コンボの発動受付を開く
	AttackComponent->OpenComboActivationWindow();
}

//攻撃発動受付を終了
void UComboActivationNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UPlayerAttackComponent* AttackComponent = FindAttackComponent(MeshComp);
	if (!AttackComponent) return;

	//ジャスト回避反撃専用の発動受付を閉じる
	if (WindowMode == EAttackInputWindowMode::JustEvasiveCounter)
	{
		AttackComponent->CloseJustEvasiveCounterActivationWindow();
		return;
	}

	//通常コンボの発動受付を閉じる
	AttackComponent->CloseComboActivationWindow();
}

//Mesh所有者から攻撃コンポーネントを取得
UPlayerAttackComponent* UComboActivationNotifyState::FindAttackComponent(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp) return nullptr;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return nullptr;

	return PlayerCharacter->FindComponentByClass<UPlayerAttackComponent>();
}
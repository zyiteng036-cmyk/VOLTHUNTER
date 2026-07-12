#include "ComboReserveNotifyState.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerAttackComponent.h"
#include "Components/SkeletalMeshComponent.h"

//攻撃入力予約を開始
void UComboReserveNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UPlayerAttackComponent* AttackComponent = FindAttackComponent(MeshComp);
	if (!AttackComponent) return;

	//ジャスト回避反撃専用の入力予約を開く
	if (WindowMode == EAttackInputWindowMode::JustEvasiveCounter)
	{
		AttackComponent->OpenJustEvasiveCounterReserveWindow();
		return;
	}

	//通常コンボの入力予約を開く
	AttackComponent->OpenComboReserveWindow();
}

//攻撃入力予約を終了
void UComboReserveNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UPlayerAttackComponent* AttackComponent = FindAttackComponent(MeshComp);
	if (!AttackComponent) return;

	//ジャスト回避反撃専用の入力予約を閉じる
	if (WindowMode == EAttackInputWindowMode::JustEvasiveCounter)
	{
		AttackComponent->CloseJustEvasiveCounterReserveWindow();
		return;
	}

	//通常コンボの入力予約を閉じる
	AttackComponent->CloseComboReserveWindow();
}

//Mesh所有者から攻撃コンポーネントを取得
UPlayerAttackComponent* UComboReserveNotifyState::FindAttackComponent(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp) return nullptr;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return nullptr;

	return PlayerCharacter->FindComponentByClass<UPlayerAttackComponent>();
}
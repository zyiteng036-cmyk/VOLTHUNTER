#include "AttackGetEnemyLocation.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerAttackComponent.h"
#include "Components/SkeletalMeshComponent.h"

//攻撃踏み込みを開始
void UAttackGetEnemyLocation::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UPlayerAttackComponent* AttackComponent = FindAttackComponent(MeshComp);
	if (!AttackComponent) return;

	//現在の攻撃種別に応じた踏み込み処理を開始
	AttackComponent->AttackFirstStepBegin();
}

//攻撃踏み込みを更新
void UAttackGetEnemyLocation::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	UPlayerAttackComponent* AttackComponent = FindAttackComponent(MeshComp);
	if (!AttackComponent) return;

	//NotifyState中は毎フレーム踏み込み位置を更新
	AttackComponent->AttackFirstStepTick();
}

//攻撃踏み込みを終了
void UAttackGetEnemyLocation::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UPlayerAttackComponent* AttackComponent = FindAttackComponent(MeshComp);
	if (!AttackComponent) return;

	//踏み込み終了時に移動状態を戻す
	AttackComponent->AttackFirstStepEnd();
}

//Mesh所有者から攻撃コンポーネントを取得
UPlayerAttackComponent* UAttackGetEnemyLocation::FindAttackComponent(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp) return nullptr;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return nullptr;

	return PlayerCharacter->FindComponentByClass<UPlayerAttackComponent>();
}
//担当
//伊藤直樹

//-----------------------------------------------------
//攻撃踏み込みNotifyState
//
//攻撃中の踏み込み開始、更新、終了をAttackComponentへ通知する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AttackGetEnemyLocation.generated.h"

class UPlayerAttackComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API UAttackGetEnemyLocation : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//攻撃踏み込みを開始
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//攻撃踏み込みを更新
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	//攻撃踏み込みを終了
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	//Mesh所有者から攻撃コンポーネントを取得
	UPlayerAttackComponent* FindAttackComponent(USkeletalMeshComponent* MeshComp) const;
};
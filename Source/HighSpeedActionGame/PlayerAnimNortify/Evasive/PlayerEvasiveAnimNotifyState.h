//担当
//伊藤直樹

//-----------------------------------------------------
//ジャスト回避判定NotifyState
//
//NotifyState中だけジャスト回避判定用Collisionを有効にする
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PlayerEvasiveAnimNotifyState.generated.h"

class UPlayerEvasiveComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayerEvasiveAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//ジャスト回避判定を開始
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//ジャスト回避判定を終了
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	//Mesh所有者から回避コンポーネントを取得
	UPlayerEvasiveComponent* FindEvasiveComponent(USkeletalMeshComponent* MeshComp) const;
};
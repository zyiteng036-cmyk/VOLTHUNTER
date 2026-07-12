//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー表示切り替えNotifyState
//
//NotifyState中だけプレイヤー本体と装備中の剣を非表示にする
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Player_SetVisibility.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayer_SetVisibility : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//プレイヤーと剣を非表示にする
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//プレイヤーと剣を再表示する
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	//プレイヤー本体と装備中の剣の表示状態を切り替える
	void SetPlayerVisibility(USkeletalMeshComponent* MeshComp, bool bVisible) const;
};
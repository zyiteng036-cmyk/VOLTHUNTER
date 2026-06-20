//担当
//伊藤直樹
 
//アニメーションの特定の区間でポストプロセス効果を有効化するためのアニメーション通知(NotifyState)クラス
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PostProsess_NotifyState.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UPostProsess_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//Notify開始時（バーの左端）
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//Notify終了時（バーの右端）
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
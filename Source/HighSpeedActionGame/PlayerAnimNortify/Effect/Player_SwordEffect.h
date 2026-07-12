//担当
//伊藤直樹

//-----------------------------------------------------
//剣トレイルNotifyState
//
//指定した剣Actorの攻撃エフェクトをNotifyState中だけ有効にする
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Player_SwordEffect.generated.h"

class APlayerSword;

UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayer_SwordEffect : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//剣の攻撃エフェクトを有効化
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//剣の攻撃エフェクトを無効化
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	//対象の剣Actorへ攻撃状態を設定
	void SetSwordAttackActive(USkeletalMeshComponent* MeshComp, bool bActive) const;

protected:
	//エフェクトを切り替える剣クラス
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sword")
	TSubclassOf<APlayerSword> TargetSwordClass = nullptr;
};
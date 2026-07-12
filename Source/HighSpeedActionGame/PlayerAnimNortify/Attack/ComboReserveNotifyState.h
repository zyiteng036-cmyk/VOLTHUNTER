//担当
//伊藤直樹

//-----------------------------------------------------
//コンボ予約受付NotifyState
//
//通常コンボまたはジャスト回避反撃の入力予約期間を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "../../PlayerComponent/Types/AttackInputWindowTypes.h"
#include "ComboReserveNotifyState.generated.h"

class UPlayerAttackComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API UComboReserveNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//攻撃入力予約を開始
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//攻撃入力予約を終了
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	//Mesh所有者から攻撃コンポーネントを取得
	UPlayerAttackComponent* FindAttackComponent(USkeletalMeshComponent* MeshComp) const;

private:
	//入力受付モード
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	EAttackInputWindowMode WindowMode = EAttackInputWindowMode::Combo;
};
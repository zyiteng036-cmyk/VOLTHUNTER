//担当
//伊藤直樹

//-----------------------------------------------------
//スキルエフェクトNotifyState
//
//スキルエフェクトActorを生成し、終了時にフェードまたは破棄する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SkillEffect_NotifyState.generated.h"

class AActor;

UCLASS()
class HIGHSPEEDACTIONGAME_API USkillEffect_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//スキルエフェクトActorを生成
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//生成したスキルエフェクトを終了
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	//生成するエフェクトActorクラス
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillEffect")
	TSubclassOf<AActor> EffectClass = nullptr;

	//エフェクトを取り付けるソケット
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillEffect")
	FName SocketName = NAME_None;

	//生成位置オフセット
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillEffect")
	FVector LocationOffset = FVector::ZeroVector;

	//生成したエフェクトを識別するタグ
	UPROPERTY(EditAnywhere, Category = "SkillEffect")
	FName TraceTag = FName(TEXT("ThunderTrailActive"));
};
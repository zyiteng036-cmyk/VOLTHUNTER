//担当
//伊藤直樹

//スキルのエフェクトクラス

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SkillEffect_NotifyState.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API USkillEffect_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	// エディタ上で設定する、生成したいエフェクト（BP_ThunderTrailなどを指定）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillEffect")
	TSubclassOf<AActor> EffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillEffect")
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillEffect")
	FVector LocationOffset;

	// 検索用のタグ名
	UPROPERTY(EditAnywhere, Category = "AnimNotify")
	FName TraceTag = FName("ThunderTrailActive");

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
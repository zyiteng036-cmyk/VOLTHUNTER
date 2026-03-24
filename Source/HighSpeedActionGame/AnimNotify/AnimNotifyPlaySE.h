//担当：佐々木奏太
//サウンド再生アニメ通知クラス

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotifyPlaySE.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAnimNotifyPlaySE : public UAnimNotify
{
	GENERATED_BODY()

public:
    // 再生するサウンド
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TObjectPtr<USoundBase> m_SoundToPlay;//再生用サウンド

    // Notify発動時に呼ばれる
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};

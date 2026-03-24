// 担当：佐々木奏太
//メッシュ非表示AnimNotifyクラス

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotifyToggleVisibility.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAnimNotifyToggleVisibility : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animtion)override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible")
	bool m_ShowMesh = true;//メッシュを表示するか非表示にするか
};

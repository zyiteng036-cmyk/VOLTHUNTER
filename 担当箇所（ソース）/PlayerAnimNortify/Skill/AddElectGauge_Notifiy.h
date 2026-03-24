//担当
//伊藤直樹

//ヒット時ゲージを加算するクラス

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AddElectGauge_Notifiy.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAddElectGauge_Notifiy : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	//攻撃で増える量
	// この攻撃で増えるゲージ量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElectroGauge")
	float m_AddGaugeAmount;
};

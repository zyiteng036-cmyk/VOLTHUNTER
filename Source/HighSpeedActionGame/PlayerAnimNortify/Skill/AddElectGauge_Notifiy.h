//’S“–
//ˆÉ“¡’¼÷

//-----------------------------------------------------
//“d—ÍƒQ[ƒW‰ÁZNotify
//
//UŒ‚‚ª–½’†‚µ‚Ä‚¢‚éê‡‚¾‚¯“d—ÍƒQ[ƒW‚ğ‰ÁZ‚·‚é
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AddElectGauge_Notifiy.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UAddElectGauge_Notifiy : public UAnimNotify
{
	GENERATED_BODY()

public:
	//UŒ‚–½’†‚É“d—ÍƒQ[ƒW‚ğ‰ÁZ
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	//‚±‚ÌUŒ‚‚Å‰ÁZ‚·‚é“d—ÍƒQ[ƒW—Ê
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElectroGauge")
	float m_AddGaugeAmount = 0.f;
};
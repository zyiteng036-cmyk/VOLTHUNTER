// 佐々木奏太担当
//モンタージュ再生BTT

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_PlayMontage.generated.h"

class UAnimMontage;

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UBTT_PlayMontage : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_PlayMontage();

	/** 再生するモンタージュ */
	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* MontageToPlay;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual  void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds);
protected:
	/** モンタージュ終了時のコールバック */
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp);

	/** Delegate登録のためのハンドルを保持 */
	FDelegateHandle MontageEndedDelegateHandle;

	bool m_DelegateBound = false;
	UBehaviorTreeComponent* CachedOwnerComp = nullptr;
};

//佐々木奏太担当
//ボスのAIコントローラークラス

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossAIController.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API ABossAIController : public AAIController
{
	GENERATED_BODY()
public:
	void OnPossess(APawn* InPawn);

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* BTAsset;
protected:
	virtual void BeginPlay() override;

	// エディタやBPで設定可能なビヘイビアツリー資産
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> m_BehaviorTreeAsset;

};

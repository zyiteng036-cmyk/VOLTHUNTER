//佐々木奏太担当
//雑魚のAIコントローラークラス

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GruntAIController.generated.h"

class UAISenseConfig_Sight;
struct FAIStimulus;

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API AGruntAIController : public AAIController
{
	GENERATED_BODY()

public:
	AGruntAIController();
	void OnPossess(APawn* InPawn);

	void EnablePerceptionUpdates();
	void DisablePerceptionUpdates();

	void SetAIActive(const bool _isActive);
	void RestartBT();
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void OnPlayerFound();
	// プレイヤー見失い関数
	void OnPlayerLost();

	UFUNCTION()
	void HandleLostSightTimerExpired();

	// エディタやBPで設定可能なビヘイビアツリー資産
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> m_BehaviorTreeAsset;

	// AI Perceptionコンポーネントのインスタンス（視覚Senseを管理）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent>AIPerceptionComp;

	// 視覚Senseの設定（視認範囲、視野角など）
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	FTimerHandle LostSightTimerHandle;

	FName HasLineOfSightKey = "IsPlayerInSight";  // 視界内かどうかのboolキー

	bool m_PlayerCurrentlySensed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float m_LostSightDelay = 3.0f; // 追跡猶予秒数

};

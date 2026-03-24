//佐々木奏太担当
//ダッシュ攻撃用GA

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityEnemy_DashAttack.generated.h"

class AEnemyBase;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_ApplyRootMotionConstantForce;
class UAbilityTask_WaitCancel;
/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAbilityEnemy_DashAttack : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UAbilityEnemy_DashAttack();

	//アビリティアクティブ時に呼ばれるメソッド
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttack")
	TObjectPtr<UAnimMontage> AttackMontage;

	
	FVector DashVector;

	/** 突進の強さ（速度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashStrength;

	/** 突進の持続時間（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashDuration;

	UPROPERTY()
	UAbilityTask_ApplyRootMotionConstantForce* RootMotionTask;//アニメモンタージュ終了時の処理

	UPROPERTY()
	UAbilityTask_WaitCancel* CancelTask;

	// 壁判定用タイマー
	FTimerHandle DashWallCheckTimerHandle;

	UFUNCTION()
	void CheckWallDuringDash();

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnAbilityCancelled();
};

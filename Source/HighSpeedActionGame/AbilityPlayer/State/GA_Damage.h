//担当
//伊藤直樹

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "GA_Damage.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;

UCLASS()
class HIGHSPEEDACTIONGAME_API UGA_Damage : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_Damage();

protected:
	//Ability開始時
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	//Ability終了時
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

private:
	//Montage正常終了
	UFUNCTION()
	void OnDamageMontageCompleted();

	//Montage中断
	UFUNCTION()
	void OnDamageMontageInterrupted();

	//Damage終了
	void FinishDamageAbility(bool bWasCancelled);

private:
	//ダメージMontage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> m_DamageMontage = nullptr;

	//再生速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Animation", meta = (AllowPrivateAccess = "true"))
	float m_PlayRate = 1.f;

	//入力を止めるか
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	bool m_bLockInputDuringDamage = true;

	//終了済みか
	bool m_bDamageAbilityEnded = false;
};
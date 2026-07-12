//担当
//伊藤直樹

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "GA_Death.generated.h"

class UAnimMontage;

UCLASS()
class HIGHSPEEDACTIONGAME_API UGA_Death : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_Death();

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
	//Montage終了
	UFUNCTION()
	void OnDeathMontageCompleted();

	//死亡Ability終了
	void FinishDeathAbility();

private:
	//死亡Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> m_DeathMontage = nullptr;

	//再生速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Animation", meta = (AllowPrivateAccess = "true"))
	float m_PlayRate = 1.f;

	//死亡確定をMontage終了後に呼ぶか
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death", meta = (AllowPrivateAccess = "true"))
	bool m_bCallPlayerDiedOnMontageEnd = true;

	//終了済みか
	bool m_bDeathAbilityEnded = false;
};
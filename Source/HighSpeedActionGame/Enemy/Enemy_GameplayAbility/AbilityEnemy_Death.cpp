// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityEnemy_Death.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "../EnemyBase.h"

//アビリティアクティブ時に呼ばれるメソッド
void UAbilityEnemy_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (DamageMontage)
	{

		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DamageMontage);

		MontageTask->OnCompleted.AddDynamic(this, &UAbilityEnemy_Death::OnMontageEnded);

		MontageTask->ReadyForActivation();

	}
}


void UAbilityEnemy_Death::OnMontageEnded() {
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	// キャラクターを取得
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor)
	{
		AEnemyBase* EnemyChar = Cast<AEnemyBase>(AvatarActor);
		if (EnemyChar)
		{
			// キャラクターのメソッドを呼び出す
			EnemyChar->OnDeath();

		}
	}

}


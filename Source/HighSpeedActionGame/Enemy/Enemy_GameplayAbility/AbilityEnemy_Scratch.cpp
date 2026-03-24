// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityEnemy_Scratch.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "../EnemyBase.h"

UAbilityEnemy_Scratch::UAbilityEnemy_Scratch() {

}

void UAbilityEnemy_Scratch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);

		MontageTask->OnCompleted.AddDynamic(this, &UAbilityEnemy_Scratch::OnMontageEnded);

		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}


void UAbilityEnemy_Scratch::OnMontageEnded() {
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	// キャラクターを取得
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor)
	{
		AEnemyBase* EnemyChar = Cast<AEnemyBase>(AvatarActor);
		if (EnemyChar)
		{
			// キャラクターのメソッドを呼び出す
			//EnemyChar->OnAttackEnd();
		}
	}
}


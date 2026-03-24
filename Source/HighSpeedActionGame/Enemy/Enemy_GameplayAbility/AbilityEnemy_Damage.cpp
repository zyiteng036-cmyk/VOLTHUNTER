// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityEnemy_Damage.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "../EnemyBase.h"

void UAbilityEnemy_Damage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 前のタスクがあれば終了
	if (CurrentMontageTask)
	{
		CurrentMontageTask->EndTask();
		CurrentMontageTask = nullptr;
	}

	if (DamageMontage)
	{
		CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DamageMontage);
		CurrentMontageTask->OnCompleted.AddDynamic(this, &UAbilityEnemy_Damage::OnMontageEnded);
		CurrentMontageTask->OnInterrupted.AddDynamic(this, &UAbilityEnemy_Damage::OnMontageInterruptedOrCancelled);
		CurrentMontageTask->OnCancelled.AddDynamic(this, &UAbilityEnemy_Damage::OnMontageInterruptedOrCancelled);
		CurrentMontageTask->ReadyForActivation();
	}
}

void UAbilityEnemy_Damage::OnMontageEnded() {
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	// キャラクターを取得
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor)
	{
		AEnemyBase* EnemyChar = Cast<AEnemyBase>(AvatarActor);
		if (EnemyChar)
		{
			// キャラクターのメソッドを呼び出す
			//EnemyChar->OnDamageEnd();
		}
	}
}

void UAbilityEnemy_Damage::OnMontageInterruptedOrCancelled() {
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
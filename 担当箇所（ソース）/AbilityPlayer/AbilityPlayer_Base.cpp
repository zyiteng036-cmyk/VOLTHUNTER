// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityPlayer_Base.h"
#include "../PlayerComponent/Player_AttackComponent.h"
#include "../PlayerComponent/Player_SkillComponent.h"
#include "../PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"




void UAbilityPlayer_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	//デバッグ
	if (GEngine)
	{
		FString MontageName = AttackMontage ? AttackMontage->GetName() : TEXT("None");
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Yellow,
			FString::Printf(TEXT("Activate Ability: %s"), *MontageName)
		);
	}


	//Montageが無い場合は終了
	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}



	//Montage再生タスク作成
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);

	MontageTask->OnCompleted.AddDynamic(this, &UAbilityPlayer_Base::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UAbilityPlayer_Base::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UAbilityPlayer_Base::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UAbilityPlayer_Base::OnMontageEnded);

	//タスクを有効化
	MontageTask->ReadyForActivation();
}

//Montage終了時
void UAbilityPlayer_Base::OnMontageEnded()
{


	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->SetIsHit(false);
			if (UPlayer_SkillComponent* SkillComp = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>())
			{
				SkillComp->SetCanSkillActive(true);
			}

		}
	}
	//Abilityを終了
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

void UAbilityPlayer_Base::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(ActorInfo->AvatarActor))
	{
		if (UPlayer_AttackComponent* AttackComp =
			Player->FindComponentByClass<UPlayer_AttackComponent>())
		{
			if (AttackComp->GetCanBufferAttack() || AttackComp->GetNextAttackRequested())return;

			AttackComp->SetIsAttack(false);
		}
	}
}

bool UAbilityPlayer_Base::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}


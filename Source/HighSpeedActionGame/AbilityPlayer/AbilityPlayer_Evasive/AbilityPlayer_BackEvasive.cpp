// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityPlayer_BackEvasive.h"
#include "AbilitySystemComponent.h"
#include "../../PlayerCharacter.h"
#include"../../PlayerComponent/Player_EvasiveComponent.h"
#include"../../PlayerComponent/Player_SkillComponent.h"
#include "../../PlayerComponent/Player_MovementComponent.h"



void UAbilityPlayer_BackEvasive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{


	UE_LOG(LogTemp, Error, TEXT("m_AbilityPlayer_EvasiveIn"));
	//Montageが無い場合は終了
	if (!EvasiveBackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		UE_LOG(LogTemp, Warning, TEXT("Evasive: no montage"));
		return;
	}
	UAbilitySystemComponent* AbilitySystemComp = GetAbilitySystemComponentFromActorInfo();



	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());

	if (!PlayerCharacter)return;

	//Montage再生タスク作成
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, EvasiveBackMontage);

	MontageTask->OnCompleted.AddDynamic(this, &UAbilityPlayer_BackEvasive::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UAbilityPlayer_BackEvasive::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UAbilityPlayer_BackEvasive::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UAbilityPlayer_BackEvasive::OnMontageEnded);

	//タスクを有効化
	MontageTask->ReadyForActivation();

}

void UAbilityPlayer_BackEvasive::OnMontageEnded()
{

	//回避終了
	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
		{
			if (UPlayer_EvasiveComponent* EvasiveComp = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>())
			{
				EvasiveComp->SetIsEvasive(false);
			}
			if (UPlayer_SkillComponent* SkillComp = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>())
			{
				SkillComp->SetCanSkillActive(true);
			}
			if (UPlayer_MovementComponent* MovementComp = PlayerCharacter->FindComponentByClass<UPlayer_MovementComponent>())
			{
				MovementComp->SetCanMovement(true);
			}
			PlayerCharacter->RevivalCollision();
		}
	}



	//

	//Abilityを終了
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

bool UAbilityPlayer_BackEvasive::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	UE_LOG(LogTemp, Warning, TEXT("CanActivateAbility結果: %s"), bResult ? TEXT("true") : TEXT("false"));
	if (!bResult && OptionalRelevantTags)
	{
		for (const FGameplayTag& Tag : *OptionalRelevantTags)
		{
			UE_LOG(LogTemp, Warning, TEXT("起動拒否タグ: %s"), *Tag.ToString());
		}
	}
	return bResult;
}

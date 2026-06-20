// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityPlayer_Evasive.h"
#include "AbilitySystemComponent.h"
#include "../../PlayerCharacter.h"
#include"../../PlayerComponent/Player_EvasiveComponent.h"
#include"../../PlayerComponent/Player_SkillComponent.h"
#include "../../PlayerComponent/Player_MovementComponent.h"



void UAbilityPlayer_Evasive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	//コストやクールダウンの適用
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//Montageが無い場合は終了
	if (!EvasiveMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	//アクター情報の検証
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}


	//Montage再生タスク作成
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, EvasiveMontage);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UAbilityPlayer_Evasive::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UAbilityPlayer_Evasive::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UAbilityPlayer_Evasive::OnMontageEnded);
		MontageTask->OnBlendOut.AddDynamic(this, &UAbilityPlayer_Evasive::OnMontageEnded);

		// タスクを有効化
		MontageTask->ReadyForActivation();
	}
}

void UAbilityPlayer_Evasive::OnMontageEnded()
{

	//回避終了
	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
		if (PlayerCharacter)
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

			// 回避中に消していたコリジョンを復帰
			PlayerCharacter->RevivalCollision();
		}
	}

	//Abilityを終了
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

bool UAbilityPlayer_Evasive::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

#if !UE_BUILD_SHIPPING
	if (!bResult && OptionalRelevantTags)
	{
		for (const FGameplayTag& Tag : *OptionalRelevantTags)
		{
			UE_LOG(LogTemp, Warning, TEXT("AbilityPlayer_Evasive Activation Denied by Tag: %s"), *Tag.ToString());
		}
	}
#endif
	return bResult;
}

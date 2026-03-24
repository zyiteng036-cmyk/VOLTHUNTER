// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityPlayer_AirAttackHeavy.h"
#include "../../../PlayerCharacter.h"
#include "../../../PlayerComponent/Player_AttackComponent.h"

void UAbilityPlayer_AirAttackHeavy::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// ホバリング開始 呼び出し
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
			{
				AttackComp->AirAttackStart();
			}

		}
	}

	// 基底クラスの処理を実行しMontage再生等を行う
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

}

void UAbilityPlayer_AirAttackHeavy::OnMontageEnded()
{
	// ホバリング終了 呼び出し
	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
		{
			if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
			{
				if (!AttackComp->GetNextAttackRequested() || !PlayerCharacter->m_BufferedNextAbility)
				{
					AttackComp->AirAttackEnd();
				}
			}
		}
	}

	// 基底クラスのMontage終了処理・Ability終了処理呼び出し
	Super::OnMontageEnded();

}

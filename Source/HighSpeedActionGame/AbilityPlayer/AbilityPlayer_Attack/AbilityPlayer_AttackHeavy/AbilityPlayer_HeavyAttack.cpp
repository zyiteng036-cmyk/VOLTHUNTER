// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityPlayer_HeavyAttack.h"
#include "../../../PlayerComponent/Player_AttackComponent.h"
#include "../../../PlayerComponent/Player_ElectroGaugeComponent.h"
#include "../../../PlayerComponent/Player_EvasiveComponent.h"
#include "../../../PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"



void UAbilityPlayer_HeavyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	//入力の長さによって出るアニメーションが変わる
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		if (UPlayer_ElectroGaugeComponent* ElectroComp = PlayerCharacter->FindComponentByClass<UPlayer_ElectroGaugeComponent>())
		{

			if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
			{

				if (AttackComp->GetJustEvasiveLongCharge())
				{
					SetPlayMontage = HeavyLongkMontage;
					AttackComp->SetJustEvasiveLongCharge(false);
				}
				else
				{
					bool bIsOverCharge = ElectroComp->IsOvercharge();
					bool bIsLongCharge = (AttackComp->GetCurrentHeavyChargeTime() >= PlayerParam.HeavyChargeLong);

					if (bIsLongCharge || bIsOverCharge)
					{
						SetPlayMontage = HeavyLongkMontage;
						PlayerCharacter->SetIsEnhancedAttack(true);
						PlayerCharacter->SetInvincible(true);
					}
					else
					{
						SetPlayMontage = HeavyShortMontage;
					}
				}
			}
		}
	}



	//Montageが無い場合は終了
	if (!SetPlayMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}



	//Montage再生タスク作成
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SetPlayMontage);

	MontageTask->OnCompleted.AddDynamic(this, &UAbilityPlayer_HeavyAttack::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UAbilityPlayer_HeavyAttack::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UAbilityPlayer_HeavyAttack::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UAbilityPlayer_HeavyAttack::OnMontageEnded);

	//タスクを有効化
	MontageTask->ReadyForActivation();

}

//Montage終了時
void UAbilityPlayer_HeavyAttack::OnMontageEnded()
{

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
		{
			AttackComp->SetHeavyAttackStart(false);
		}
		if (UPlayer_EvasiveComponent* EvasiveComp = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>())
		{
			EvasiveComp->SetCanEvasive(true);

		}
		PlayerCharacter->SetInvincible(false);
		PlayerCharacter->SetIsEnhancedAttack(false);
	}
	//Abilityを終了
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}



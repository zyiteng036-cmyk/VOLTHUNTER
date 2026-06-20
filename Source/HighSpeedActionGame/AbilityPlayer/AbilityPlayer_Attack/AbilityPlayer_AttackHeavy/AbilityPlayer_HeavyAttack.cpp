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
	//安全確認と早期リターン
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>();
	UPlayer_ElectroGaugeComponent* ElectroComp = PlayerCharacter->FindComponentByClass<UPlayer_ElectroGaugeComponent>();

	if (AttackComp && ElectroComp)
	{
		//再生するモンタージュの決定
		if (AttackComp->GetJustEvasiveLongCharge())
		{
			//親クラスの変数(AttackMontage)にセットするだけ！
			AttackMontage = HeavyLongMontage;
			AttackComp->SetJustEvasiveLongCharge(false);
		}
		else
		{
			const bool bIsOverCharge = ElectroComp->IsOvercharge();
			const bool bIsLongCharge = (AttackComp->GetCurrentHeavyChargeTime() >= PlayerParam.HeavyChargeLong);

			if (bIsLongCharge || bIsOverCharge)
			{
				AttackMontage = HeavyLongMontage;
				PlayerCharacter->SetIsEnhancedAttack(true);
				PlayerCharacter->SetInvincible(true);
			}
			else
			{
				AttackMontage = HeavyShortMontage;
			}
		}
	}

	//ベースクラスの処理を呼ぶ
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}


//Montage終了時
void UAbilityPlayer_HeavyAttack::OnMontageEnded()
{

	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
		if (PlayerCharacter)
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
	}

	//Abilityを終了
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}



// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityPlayer_Base.h"
#include "../PlayerComponent/Player_AttackComponent.h"
#include "../PlayerComponent/Player_SkillComponent.h"
#include "../PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"




UAbilityPlayer_Base::UAbilityPlayer_Base()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAbilityPlayer_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	//コストやクールダウンの適用
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//Montageが無い場合は終了
	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}



	//Montage再生タスク作成
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UAbilityPlayer_Base::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UAbilityPlayer_Base::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UAbilityPlayer_Base::OnMontageEnded);
		MontageTask->OnBlendOut.AddDynamic(this, &UAbilityPlayer_Base::OnMontageEnded);

		//タスクを有効化
		MontageTask->ReadyForActivation();
	}

	else
	{
		//タスク生成失敗時のフェイルセーフ
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

//Montage終了時
void UAbilityPlayer_Base::OnMontageEnded()
{
	//安全確保と早期リターン
	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());

		if (PlayerCharacter)
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

	//早期リターンでネストを浅くする
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!Player) return;

	UPlayer_AttackComponent* AttackComp = Player->FindComponentByClass<UPlayer_AttackComponent>();
	if (!AttackComp) return;

	//コンボが予約されている、または先行入力受付中の場合は攻撃状態を解除しない
	if (AttackComp->GetCanBufferAttack() || AttackComp->GetNextAttackRequested()) return;

	//完全に攻撃が終わったのでフラグをオフにする
	AttackComp->SetIsAttack(false);
}

bool UAbilityPlayer_Base::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}


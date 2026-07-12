#include "GA_AttackInput_Heavy.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerAttackComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

//初期化
UGA_AttackInput_Heavy::UGA_AttackInput_Heavy()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	m_HeavyPressedInputTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Attack.Heavy"), false);
	m_HeavyReleasedInputTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Attack.Heavy.Release"), false);
}

//Ability開始時
void UGA_AttackInput_Heavy::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	m_bReleased = false;
	m_AttackComponent = nullptr;

	if (!m_PlayerCharacter)
	{
		EndPlayerAbility(true);
		return;
	}

	m_AttackComponent = m_PlayerCharacter->FindComponentByClass<UPlayerAttackComponent>();

	if (!m_AttackComponent)
	{
		EndPlayerAbility(true);
		return;
	}

	//押下入力を攻撃コンポーネントへ渡す
	const bool bAccepted = m_AttackComponent->HandleAttackInputByTag(m_HeavyPressedInputTag);

	if (!bAccepted)
	{
		EndPlayerAbility(true);
		return;
	}

	//入力を離した瞬間を待つ
	UAbilityTask_WaitInputRelease* WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);

	if (!WaitInputReleaseTask)
	{
		EndPlayerAbility(true);
		return;
	}

	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_AttackInput_Heavy::OnHeavyInputReleased);
	WaitInputReleaseTask->ReadyForActivation();
}

//入力を離した時
void UGA_AttackInput_Heavy::OnHeavyInputReleased(float TimeHeld)
{
	if (m_bReleased) return;

	m_bReleased = true;

	UE_LOG(LogTemp, Warning, TEXT("[HeavyInput] Released TimeHeld:%f"), TimeHeld);

	if (m_AttackComponent)
	{
		//ShortまたはFullへ派生させる離し入力を送る
		const bool bAccepted = m_AttackComponent->HandleAttackInputByTag(m_HeavyReleasedInputTag);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[HeavyInput] Send Release Result:%s Tag:%s"),
			bAccepted ? TEXT("true") : TEXT("false"),
			*m_HeavyReleasedInputTag.ToString()
		);
	}

	EndPlayerAbility(false);
}
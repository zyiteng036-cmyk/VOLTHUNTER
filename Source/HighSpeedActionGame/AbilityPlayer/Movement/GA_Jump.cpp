#include "GA_Jump.h"
#include "../../PlayerComponent/CustomCharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

//初期化
UGA_Jump::UGA_Jump()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

//ジャンプを開始
void UGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!m_CustomMovementComponent)
	{
		EndPlayerAbility(true);
		return;
	}

	if (!m_CustomMovementComponent->StartJump())
	{
		EndPlayerAbility(true);
		return;
	}

	UAbilityTask_WaitInputRelease* WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);

	if (!WaitInputReleaseTask)
	{
		EndPlayerAbility(false);
		return;
	}

	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_Jump::OnJumpInputReleased);
	WaitInputReleaseTask->ReadyForActivation();
}

//入力を離した時にジャンプ処理を終了
void UGA_Jump::OnJumpInputReleased(float TimeHeld)
{
	if (m_CustomMovementComponent)
	{
		m_CustomMovementComponent->ReleaseJump();
	}

	EndPlayerAbility(false);
}
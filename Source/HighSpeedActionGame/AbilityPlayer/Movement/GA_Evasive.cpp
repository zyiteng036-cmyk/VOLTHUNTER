#include "GA_Evasive.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/CustomCharacterMovementComponent.h"
#include "../../PlayerComponent/PlayerAttackComponent.h"
#include "../../PlayerComponent/PlayerEvasiveComponent.h"
#include "../../DataAssets/Player/PlayerMovementParameter.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"

//初期化
UGA_Evasive::UGA_Evasive()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

//回避を開始
void UGA_Evasive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	m_EvasiveComponent = nullptr;
	m_bEvasiveEnded = false;

	if (!m_PlayerCharacter || !m_CustomMovementComponent)
	{
		EndPlayerAbility(true);
		return;
	}

	//スキル中は回避を開始しない
	const FGameplayTag SkillActiveTag = FGameplayTag::RequestGameplayTag(TEXT("State.Skill.Active"), false);

	if (SkillActiveTag.IsValid() && m_AbilitySystemComponent && m_AbilitySystemComponent->HasMatchingGameplayTag(SkillActiveTag))
	{
		EndPlayerAbility(true);
		return;
	}

	//地上にいる場合だけ回避を開始する
	const UCharacterMovementComponent* CharacterMovement = m_PlayerCharacter->GetCharacterMovement();

	if (!CharacterMovement || !CharacterMovement->IsMovingOnGround())
	{
		EndPlayerAbility(true);
		return;
	}

	//回避を優先するため現在の攻撃を終了する
	if (UPlayerAttackComponent* AttackComponent = m_PlayerCharacter->FindComponentByClass<UPlayerAttackComponent>())
	{
		AttackComponent->CancelAttackForEvasive();
	}

	if (!m_CustomMovementComponent->StartEvasive())
	{
		EndPlayerAbility(true);
		return;
	}

	m_EvasiveComponent = m_PlayerCharacter->FindComponentByClass<UPlayerEvasiveComponent>();

	if (m_EvasiveComponent)
	{
		m_EvasiveComponent->BeginEvasive();
	}

	PlayEvasiveMontage();

	const UPlayerMovementParameter* MovementParameter = m_CustomMovementComponent->GetMovementParameter();
	const float EvasiveDuration = MovementParameter ? MovementParameter->EvasiveDuration : DefaultEvasiveDuration;

	UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, EvasiveDuration);

	if (!WaitDelayTask)
	{
		OnEvasiveFinished();
		return;
	}

	WaitDelayTask->OnFinish.AddDynamic(this, &UGA_Evasive::OnEvasiveFinished);
	WaitDelayTask->ReadyForActivation();
}

//回避Abilityを終了
void UGA_Evasive::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!m_bEvasiveEnded)
	{
		FinishEvasiveState();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//方向に応じた回避Montageを再生
void UGA_Evasive::PlayEvasiveMontage()
{
	if (!m_PlayerCharacter || !m_CustomMovementComponent) return;

	const EPlayerEvasiveDirectionType DirectionType = m_CustomMovementComponent->GetLastEvasiveDirectionType();
	UAnimMontage* SelectedMontage = DirectionType == EPlayerEvasiveDirectionType::InputDirection ? m_EvasiveMontage.Get() : m_BackEvasiveMontage.Get();

	if (!SelectedMontage) return;

	m_PlayerCharacter->PlayAnimMontage(SelectedMontage);
}

//回避状態を終了
void UGA_Evasive::FinishEvasiveState()
{
	if (m_bEvasiveEnded) return;

	m_bEvasiveEnded = true;

	if (m_EvasiveComponent)
	{
		m_EvasiveComponent->EndEvasive();
	}

	if (m_CustomMovementComponent)
	{
		m_CustomMovementComponent->EndEvasive();
	}
}

//回避時間終了時にAbilityを終了
void UGA_Evasive::OnEvasiveFinished()
{
	if (m_bEvasiveEnded) return;

	FinishEvasiveState();
	EndPlayerAbility(false);
}
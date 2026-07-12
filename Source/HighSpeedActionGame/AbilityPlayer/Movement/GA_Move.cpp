#include "GA_Move.h"
#include "../../PlayerComponent/CustomCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

//初期化
UGA_Move::UGA_Move()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

//常駐移動Abilityを開始
void UGA_Move::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!m_CustomMovementComponent)
	{
		EndPlayerAbility(true);
		return;
	}

	//移動入力を受け続けるためAbilityを終了しない
}

//Ability終了時に移動入力をクリア
void UGA_Move::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (m_CustomMovementComponent)
	{
		m_CustomMovementComponent->ClearMovementInput();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//移動入力を更新
void UGA_Move::UpdateMoveInput(const FVector2D& InputVector)
{
	if (!m_CustomMovementComponent) return;

	if (IsMoveInputBlocked())
	{
		m_CustomMovementComponent->ClearMovementInput();
		return;
	}

	m_CustomMovementComponent->UpdateMovementInput(InputVector);
}

//移動入力をクリア
void UGA_Move::ClearMoveInput()
{
	if (!m_CustomMovementComponent) return;

	m_CustomMovementComponent->ClearMovementInput();
}

//移動入力を停止する状態か判定
bool UGA_Move::IsMoveInputBlocked() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	if (!AbilitySystemComponent)
	{
		AbilitySystemComponent = m_AbilitySystemComponent;
	}

	if (!AbilitySystemComponent) return false;

	const FGameplayTag AttackActiveTag = FGameplayTag::RequestGameplayTag(TEXT("State.Attack.Active"), false);

	if (AttackActiveTag.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(AttackActiveTag))
	{
		return true;
	}

	const FGameplayTag SkillActiveTag = FGameplayTag::RequestGameplayTag(TEXT("State.Skill.Active"), false);

	if (SkillActiveTag.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(SkillActiveTag))
	{
		return true;
	}

	const FGameplayTag MoveBlockedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Input.MoveBlocked"), false);

	if (MoveBlockedTag.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(MoveBlockedTag))
	{
		return true;
	}

	return false;
}
#include "GA_AttackBase.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerAttackComponent.h"
#include "../../PostProcess/PostProsess.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"

//初期化
UGA_AttackBase::UGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag AttackAbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack"), false);

	if (AttackAbilityTag.IsValid())
	{
		FGameplayTagContainer AssetTags = GetAssetTags();
		AssetTags.AddTag(AttackAbilityTag);
		SetAssetTags(AssetTags);
	}

	const FGameplayTag AttackActiveTag = FGameplayTag::RequestGameplayTag(TEXT("State.Attack.Active"), false);

	if (AttackActiveTag.IsValid())
	{
		ActivationOwnedTags.AddTag(AttackActiveTag);
	}
}

//攻撃Abilityを開始
void UGA_AttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	m_AttackComponent = nullptr;
	m_bAttackAbilityEnded = false;
	m_bIsJustEvasiveCounterFirstAttack = false;

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

	if (!m_AttackTag.IsValid())
	{
		EndPlayerAbility(true);
		return;
	}

	if (m_ActionType == EPlayerActionType::None)
	{
		EndPlayerAbility(true);
		return;
	}

	if (m_AttackType == EPlayerAttackType::None)
	{
		EndPlayerAbility(true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndPlayerAbility(true);
		return;
	}

	m_bIsJustEvasiveCounterFirstAttack = m_AttackComponent->GetIsJustEvasiveCounterReady();

	m_AttackComponent->BeginAttackByTag(m_AttackTag, m_ActionType, m_AttackType);

	if (m_bIsJustEvasiveCounterFirstAttack)
	{
		SetJustEvasivePostProcessActive(true);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[JustCounterPostProcess] Begin Attack:%s"),
			*m_AttackTag.ToString()
		);
	}

	BeginAttackMotionByType();

	if (!m_AttackMontage)
	{
		FinishAttackAbility(false);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, m_AttackMontage, m_PlayRate, m_StartSectionName);

	if (!MontageTask)
	{
		FinishAttackAbility(true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_AttackBase::OnAttackMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_AttackBase::OnAttackMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_AttackBase::OnAttackMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_AttackBase::OnAttackMontageCancelled);
	MontageTask->ReadyForActivation();
}

//攻撃Abilityを終了
void UGA_AttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (m_bIsJustEvasiveCounterFirstAttack)
	{
		SetJustEvasivePostProcessActive(false);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[JustCounterPostProcess] End Attack:%s Cancelled:%s"),
			*m_AttackTag.ToString(),
			bWasCancelled ? TEXT("true") : TEXT("false")
		);

		m_bIsJustEvasiveCounterFirstAttack = false;
	}

	EndAttackMotionByType();

	if (m_AttackComponent)
	{
		m_AttackComponent->EndAttackByTag(m_AttackTag, bWasCancelled);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//Montage正常終了時にAbilityを終了
void UGA_AttackBase::OnAttackMontageCompleted()
{
	FinishAttackAbility(false);
}

//Montageブレンドアウト開始を受け取る
void UGA_AttackBase::OnAttackMontageBlendOut()
{
	//Montage完全終了時のOnCompletedを待つ
}

//Montage中断時にAbilityを終了
void UGA_AttackBase::OnAttackMontageInterrupted()
{
	FinishAttackAbility(true);
}

//Montageキャンセル時にAbilityを終了
void UGA_AttackBase::OnAttackMontageCancelled()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AttackAbility][MontageCancelled] AttackTag:%s Montage:%s"),
		*m_AttackTag.ToString(),
		m_AttackMontage ? *m_AttackMontage->GetName() : TEXT("None")
	);

	FinishAttackAbility(true);
}

//攻撃Abilityの終了を要求
void UGA_AttackBase::FinishAttackAbility(bool bWasCancelled)
{
	if (m_bAttackAbilityEnded)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[AttackAbility][FinishSkipped] AlreadyEnded AttackTag:%s"),
			*m_AttackTag.ToString()
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AttackAbility][Finish] AttackTag:%s Cancelled:%s"),
		*m_AttackTag.ToString(),
		bWasCancelled ? TEXT("true") : TEXT("false")
	);

	m_bAttackAbilityEnded = true;

	EndPlayerAbility(bWasCancelled);
}

//攻撃種別に応じた移動処理を開始
void UGA_AttackBase::BeginAttackMotionByType()
{
	if (!m_AttackComponent) return;

	switch (m_AttackType)
	{
	case EPlayerAttackType::AirLight:
	case EPlayerAttackType::AirDash:
		m_AttackComponent->BeginAirAttack();
		m_AttackComponent->BeginAirDashAttack();
		break;

	case EPlayerAttackType::AirHeavy:
	case EPlayerAttackType::AirFall:
		m_AttackComponent->BeginAirFallAttack();
		break;

	default:
		break;
	}
}

//攻撃種別に応じた移動処理を終了
void UGA_AttackBase::EndAttackMotionByType()
{
	if (!m_AttackComponent) return;

	switch (m_AttackType)
	{
	case EPlayerAttackType::AirLight:
	case EPlayerAttackType::AirDash:
		m_AttackComponent->EndAirDashAttack();
		m_AttackComponent->EndAirAttack();
		break;

	case EPlayerAttackType::AirHeavy:
	case EPlayerAttackType::AirFall:
		m_AttackComponent->EndAirFallAttack();
		break;

	default:
		break;
	}
}

//ジャスト回避カウンター用ポストプロセスを切り替える
void UGA_AttackBase::SetJustEvasivePostProcessActive(bool bActive)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UPostProsess* PostProsess = World->GetSubsystem<UPostProsess>();
	if (!PostProsess) return;

	PostProsess->SetPostProsessActive(bActive);
}
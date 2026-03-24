// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_GruntGuard.h"
#include "../PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySpec.h"
#include "../HitStopComponent/HitStopComponent.h"
#include "Kismet//KismetMathLibrary.h"
#include "EnemyAIController/GruntAIController.h"

namespace {
	constexpr float AttackableLength = 1000.f;//攻撃可能長さ
	constexpr float AddAttackScals = 300.f;//攻撃可能長さ
}

AEnemy_GruntGuard::AEnemy_GruntGuard()
	: m_Timer(0.f)
	, m_AttackTimer(0.f)
	, m_PreparationForAttackTime(.7f)
	, m_UntilTheAttack(3.f)
	, m_AttackSpeed(1500)
	, m_AttackVector(FVector::ZeroVector)
	, m_IsPreparingAttack(false)
{
	// AddUniqueでタグが重複しないようになる//Editorから生成した敵もプールに入れるためここでタグセット
	Tags.AddUnique(FName(TEXT("Enemy_2")));


	m_DistanceToTarget = 500.f;

	m_DamageTime = 1.5f;
}



void AEnemy_GruntGuard::BeginPlay() {
	Super::BeginPlay();

	//if (m_AbilitySystemComponent)
	//{
	//	if (GuardAbilityClass) {
	//		m_AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GuardAbilityClass, 1, INDEX_NONE, this));
	//	}
	//}

}

void AEnemy_GruntGuard::Tick(float DeltaTime) {
	if (!m_IsActive)return;

	Super::Tick(DeltaTime);


}

void AEnemy_GruntGuard::TakeDamage(const FDamageInfo& _damageInfo) {
	if (!m_IsTakeDamage)return;

	FDamageInfo damageInfo = _damageInfo;

	//ヒットストップ
	m_HitStopComponent->StartHitStop(_damageInfo.HitStopTime);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)return;

	if (_damageInfo.IsEnhancedAttack || (!m_EnemyTypeFlags.bIsEventEnemy && !m_CanSeeTarget)) {
		m_EnemyParam.m_Hp -= CalculateEnemyDamage(_damageInfo);

		if (m_DamageMontage)
		{
			HandleDamageTakenAI();

			AnimInstance->Montage_Play(m_DamageMontage);

			m_IsTakingDamage = true;
			m_DamageTimeTimer = 0.f;
		}
	}
	//ガード成功時の処理
	else {
		if (!m_IsPreparingAttack) {

			AnimInstance->Montage_Play(m_GuardMontage);

			damageInfo.KnockbackDirection.Z = 0.f;

			m_IsTakingDamage = true;
			m_DamageTimeTimer = m_DamageTime - 0.1f;
			HandleDamageTakenAI();
		}

		//ガード成功時はダメージ0.25倍
		m_EnemyParam.m_Hp -= CalculateEnemyDamage(_damageInfo) * 0.5;
	}

	if (m_EnemyParam.m_Hp <= 0) {
		OnDying();//瀕死処理呼び出す
		return;
	}

	FacePlayerDirection();

	//吹っ飛ばし
	BlowEnemy(damageInfo.KnockbackDirection, damageInfo.KnockbackScale);


	if (!m_EnemyTypeFlags.bIsEventEnemy && !m_CanSeeTarget) {
		OnSeeTarget();
	}
}

void AEnemy_GruntGuard::SetAIControllerIsActive(const bool _isActive) {
	Super::SetAIControllerIsActive(_isActive);

	AGruntAIController* AI = Cast<AGruntAIController>(GetController());
	if (AI)
	{
		AI->SetAIActive(_isActive);
	}
}
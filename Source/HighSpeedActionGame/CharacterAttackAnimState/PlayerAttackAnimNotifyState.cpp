// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttackAnimNotifyState.h"
#include "../HitJudgmentComponent/HitJudgmentComponent.h"
#include "../PlayerCharacter.h"

void UPlayerAttackAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) {
	if (!MeshComp) return;

	//再生しているActor取得
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	//Actorから目的のコンポーネント取得（
	UHitJudgmentComponent* HitJudgmentComponen = OwnerActor->FindComponentByClass<UHitJudgmentComponent>();
	if (!HitJudgmentComponen) return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner());

	if (!Player) {
		GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Red, TEXT("No Player"));
		return;
	}

	if (!m_AttackData) {
		GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Red, TEXT("No attack dataset!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
		return;
	}

	//アクターの方向からのベクトルに変更
	FRotator CharacterRotation = OwnerActor->GetActorRotation();
	FVector KnockbackVector = CharacterRotation.RotateVector(m_AttackData->KnockbackDirection);


	//攻撃情報からダメージ情報をセット
	FDamageInfo DamageInfo;
	DamageInfo.KnockbackDirection = KnockbackVector.GetSafeNormal();
	DamageInfo.Damage = m_AttackData->Damage;
	DamageInfo.KnockbackScale = m_AttackData->KnockbackScale;
	DamageInfo.AttackActor = OwnerActor;
	DamageInfo.HitStopTime = m_AttackData->HitStopTime;
	DamageInfo.IsEnhancedAttack = Player->GetIsEnhancedAttack();

	HitJudgmentComponen->BeginHitDetection(DamageInfo, m_AttackCollisionParam.Radius, m_AttackCollisionParam.RelativeLocation, *OwnerActor, m_AttackCollisionParam.HitTag, TotalDuration);
	if (m_AttackCollisionParam.Visible) {
		HitJudgmentComponen->SetAttackCollisionDetectionVisible(true);
	}

}



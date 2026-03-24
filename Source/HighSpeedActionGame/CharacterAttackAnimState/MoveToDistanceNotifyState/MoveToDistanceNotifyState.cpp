// Fill out your copyright notice in the Description page of Project Settings.


#include "MoveToDistanceNotifyState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../GameUtility/GameUtility.h"

void UMoveToDistanceNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	m_CachedCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!m_CachedCharacter.IsValid())return;

	m_OffsetVelocity = m_CachedCharacter->GetCharacterMovement()->Velocity;
	m_ForwardVector = m_CachedCharacter->GetActorForwardVector();

	m_Speed = m_Distance / TotalDuration;

}

void UMoveToDistanceNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float DeltaTime)
{
	if (!m_CachedCharacter.IsValid())return;
	m_CachedCharacter->GetCharacterMovement()->Velocity = m_Speed * m_ForwardVector* CGameUtility::GetFpsCorrection(DeltaTime);
}

void UMoveToDistanceNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!m_CachedCharacter.IsValid())return;

	m_CachedCharacter->GetCharacterMovement()->Velocity = m_OffsetVelocity;
	m_CachedCharacter = nullptr;
}

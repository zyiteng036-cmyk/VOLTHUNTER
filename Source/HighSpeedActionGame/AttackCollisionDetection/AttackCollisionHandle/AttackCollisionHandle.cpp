#include "AttackCollisionHandle.h"
#include "../AttackCollisionPoolSubsystem/AttackCollisionPoolSubsystem.h"
#include "../AttackCollisionDetection.h"

FAttackCollisionHandle::FAttackCollisionHandle(UAttackCollisionPoolSubsystem* InSubsystem, AAttackCollisionDetection* InInstance)
	: m_AttackCollisionPoolSubsystem(InSubsystem)
	, m_AttackCollisionDetectionInstance(InInstance)
{
	if (InInstance)
	{
		m_CachedWorld = InInstance->GetWorld();
	}
}




bool FAttackCollisionHandle::IsValid() const
{
	return m_AttackCollisionPoolSubsystem.IsValid() && m_AttackCollisionDetectionInstance.IsValid();
}


void FAttackCollisionHandle::Release()
{
	ReturnToPool();
}

void FAttackCollisionHandle::ReturnToPool()
{
	if (!IsValid())return;

	if (m_CachedWorld)
	{
		m_CachedWorld->GetTimerManager().ClearTimer(m_LifeTimerHandle);
	}

	m_AttackCollisionPoolSubsystem->Release(m_AttackCollisionDetectionInstance.Get());


	m_AttackCollisionPoolSubsystem = nullptr;
	m_AttackCollisionDetectionInstance = nullptr;
}

void FAttackCollisionHandle::OnLifeTimeExpired()
{
	Release();
}

void FAttackCollisionHandle::SetCollisionWorldLocation(const FVector _location) {
	if (!m_AttackCollisionDetectionInstance.IsValid())return;
	m_AttackCollisionDetectionInstance->SetActorLocation(_location);
}

void FAttackCollisionHandle::Initialize(const UHitJudgmentComponent& _hitJudgmentComponent, const FDamageInfo& _damage, const FVector& _location, const TArray<FString> _tags, const float _radius, const float _activeTime) {
	m_AttackCollisionDetectionInstance->SetHitJudgmentComponent(&_hitJudgmentComponent);
	m_AttackCollisionDetectionInstance->SetDamageInfo(_damage);
	m_AttackCollisionDetectionInstance->SetActorLocation(_location);
	m_AttackCollisionDetectionInstance->SetHitActorTags(_tags);
	m_AttackCollisionDetectionInstance->SetRadius(_radius);

	m_AttackCollisionDetectionInstance->SetActive(true);

	if (m_CachedWorld)
	{
		m_CachedWorld->GetTimerManager().SetTimer(m_LifeTimerHandle, FTimerDelegate::CreateRaw(this, &FAttackCollisionHandle::OnLifeTimeExpired), _activeTime, false);
	}
}

void FAttackCollisionHandle::SetVisible(const bool _isVisble) {
	if (!m_AttackCollisionDetectionInstance.IsValid())return;
	m_AttackCollisionDetectionInstance->SetVisible(_isVisble);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackCollisionPoolSubsystem.h"
#include "../../WorldSettings/MyWorldSettings.h"
#include "../AttackCollisionFactory/AttackCollisionFactory.h"
#include "../AttackCollisionDetection.h"
#include "../AttackCollisionHandle/AttackCollisionHandle.h"

bool UAttackCollisionPoolSubsystem::ShouldCreateSubsystem(UObject* Outer) const {
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	if (UWorld* WorldOuter = Cast<UWorld>(Outer))
	{
		if (AMyWorldSettings* MyWorldSettings = Cast<AMyWorldSettings>(WorldOuter->GetWorldSettings()))
		{
			return MyWorldSettings->m_UseEnemyManager;
		}
	}

	return false;
}

void UAttackCollisionPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("UAttackCollisionPoolSubsystem::Initialize!!!"));
	}
}

void UAttackCollisionPoolSubsystem::Deinitialize() {
	Super::Deinitialize();
	AttackCollisionPool.Empty();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("UAttackCollisionPoolSubsystem::Deinitialize!!!"));
	}
}


void UAttackCollisionPoolSubsystem::CreatePool(const AAttackCollisionFactory* _factory) {
	if (!_factory)return;
	FVector GenerateLocation = FVector::ZeroVector;
	FRotator GenerateRotation = FRotator::ZeroRotator;
	UWorld* GenerateWorld = GetWorld();


	for (int i = 0; i < PoolSize; ++i) {
		AAttackCollisionDetection* Collision = _factory->CreateAAttackCollisionDetection(GenerateWorld, GenerateLocation, GenerateRotation);

		if (Collision) {
			AttackCollisionPool.Add(Collision);
		}
	}
}

FAttackCollisionHandle UAttackCollisionPoolSubsystem::AcquireHandle() {
	//インスタンスが取れれば、インスタンスをハンドルにセットして渡す
	AAttackCollisionDetection* Instance = GetInactiveAttackCollision();

	if (!Instance)
	{
		return FAttackCollisionHandle();
	}

	return FAttackCollisionHandle(this, Instance);
}


void UAttackCollisionPoolSubsystem::Release(AAttackCollisionDetection* Instance) {
	if (!Instance) return;

	// プールに属していないなら早期リターン
	if (!AttackCollisionPool.Contains(Instance))return;

	Instance->SetActive(false);
	Instance->SetHitJudgmentComponent(nullptr);
}

AAttackCollisionDetection* UAttackCollisionPoolSubsystem::GetInactiveAttackCollision()const {
	AAttackCollisionDetection* Result = nullptr;

	for (int i = 0; i < AttackCollisionPool.Num(); ++i) {
		if (AttackCollisionPool[i]) {
			if (!AttackCollisionPool[i]->GetIsActive()) {
				Result = AttackCollisionPool[i];
				break;
			}
		}
	}

	return Result;
}
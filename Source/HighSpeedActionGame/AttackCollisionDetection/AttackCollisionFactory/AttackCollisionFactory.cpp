// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackCollisionFactory.h"
#include "../AttackCollisionPoolSubsystem/AttackCollisionPoolSubsystem.h"
#include "../AttackCollisionDetection.h"

// Sets default values
AAttackCollisionFactory::AAttackCollisionFactory()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AAttackCollisionFactory::BeginPlay()
{
	Super::BeginPlay();

	//ゲームインスタンス取得
	UWorld* World = GetWorld();

	if (!World) return;

	//EnemyManager取得
	UAttackCollisionPoolSubsystem* PoolSubSystem = World->GetSubsystem<UAttackCollisionPoolSubsystem>();

	if (PoolSubSystem) {
		//プールの初期化
		PoolSubSystem->CreatePool(this);
	}

	this->Destroy();
}

// Called every frame
void AAttackCollisionFactory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
TObjectPtr<AAttackCollisionDetection> AAttackCollisionFactory::CreateAAttackCollisionDetection(UWorld* World, const FVector& Location, const FRotator& Rotation) const
{

	AAttackCollisionDetection* Result = nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Result = World->SpawnActor<AAttackCollisionDetection>(AttackCollisionClass, Location, Rotation, SpawnParams);

	return TObjectPtr<AAttackCollisionDetection>(Result);

}
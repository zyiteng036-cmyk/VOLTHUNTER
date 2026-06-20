//敵生成クラス


#include "EnemyFactory.h"
#include "../EnemyBase.h"
#include "../EnemyManager/EnemyManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnemyFactory::AEnemyFactory()
{
	//Tick切る
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AEnemyFactory::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();

	if (!World) return;

	//EnemyManager取得
	UEnemyManager* EnemyManager = World->GetSubsystem<UEnemyManager>();

	if (EnemyManager) {
		//プールの初期化
		EnemyManager->InitializePool(this);
	}

	Destroy();
}

// Called every frame
void AEnemyFactory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TObjectPtr<AEnemyBase> AEnemyFactory::CreateEnemy(UWorld* _world, const FString& _enemyName, const FVector& _location, const FRotator& _rotation)const {
	AEnemyBase* Result = nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (_enemyName == "Enemy_1") {
		Result = _world->SpawnActor<AEnemyBase>(Enemy_GruntClass, _location, _rotation, SpawnParams);
	}
	else if (_enemyName == "Enemy_2") {
		Result = _world->SpawnActor<AEnemyBase>(Enemy_GruntGuardClass, _location, _rotation, SpawnParams);
	}
	else if (_enemyName == "TrojanHorse") {
		Result = _world->SpawnActor<AEnemyBase>(Enemy_TrojanHorseClass, _location, _rotation, SpawnParams);
	}

	if (!Result)
	{
		UE_LOG(LogTemp, Error, TEXT("BPEnemy Could not generate"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("BPEnemy generated  !!!"));
	}

	return TObjectPtr<AEnemyBase>(Result);
}

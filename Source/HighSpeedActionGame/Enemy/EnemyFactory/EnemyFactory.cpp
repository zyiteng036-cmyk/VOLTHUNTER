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


	//ゲームインスタンス取得
	UWorld* World = GetWorld();

	if (!World) return;

	//EnemyManager取得
	UEnemyManager* EnemyManager = World->GetSubsystem<UEnemyManager>();

	if (EnemyManager) {
		//プールの初期化
		EnemyManager->InitializePool();
	}

	this->Destroy();
}

// Called every frame
void AEnemyFactory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

TObjectPtr<AEnemyBase> AEnemyFactory::CreateEnemy(UWorld* World, FString EnemyName, const FVector& Location, const FRotator& Rotation) {
	AEnemyBase* Result = nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (EnemyName == "Enemy_1") {
		Result = World->SpawnActor<AEnemyBase>(Enemy_GruntClass, Location, Rotation, SpawnParams);
	}
	else if (EnemyName == "Enemy_2") {
		Result = World->SpawnActor<AEnemyBase>(Enemy_GruntGuardClass, Location, Rotation, SpawnParams);
	}
	else if (EnemyName == "TrojanHorse") {
		Result = World->SpawnActor<AEnemyBase>(Enemy_TrojanHouseClass, Location, Rotation, SpawnParams);
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

UDataTable* AEnemyFactory::GetEnemyDataToGenerate() {
	return m_EnemyDataToGenerate;
}
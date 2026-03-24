// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyManager.h"
#include "../../WorldSettings/MyWorldSettings.h"
#include "../../GameUtility/GameUtility.h"
#include "../EnemyFactory/EnemyFactory.h"
#include "../EnemyPool/EnemyPool.h"
#include "../EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "../EnemyDataBase/EnemyDataBase.h"
#include "../EnemyNavigationManager/EnemyNavigationManager.h"

bool UEnemyManager::ShouldCreateSubsystem(UObject* Outer) const {
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

void UEnemyManager::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("UEnemyManager::Initialize!!!"));
	}

	m_IsPoolCreated = false;
}

void UEnemyManager::Deinitialize() {
	Super::Deinitialize();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("UEnemyManager::Deinitialize!!!"));
	}

	//プールを空に
	m_EnemyPools.Empty();
	m_ActiveEnemies.Empty();
}

void UEnemyManager::InitializePool() {

	if (m_IsPoolCreated) {
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("The pool is created!!!"));
		return;
	}

	//プール生成Onに
	m_IsPoolCreated = true;

	//敵生成クラス取得
	AEnemyFactory* EnemyFactory = Cast<AEnemyFactory>(CGameUtility::GetActorFromTag(GetWorld(), "EnemyFactory"));

	if (!EnemyFactory) {
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("EnemyFactory:Not Found"));
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("EnemyFactory:Found!!!"));
	}

	UDataTable* LocalEnemyDataTable = EnemyFactory->GetEnemyDataToGenerate();

	if (!LocalEnemyDataTable)
	{
		GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Red, TEXT("EnemyDataNotFound"));
		return;
	}

	static const FString ContextString(TEXT("InitializePool"));
	TArray<FEnemyDataBase*> ParsedRows;
	LocalEnemyDataTable->GetAllRows(ContextString, ParsedRows);

	TArray<AActor*> EnemyActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Enemy"), EnemyActors);

	//コンテナの敵のパラメーターを順番に取り出す。
	for (const FEnemyDataBase* Row : ParsedRows)
	{
		//プール減算量//エディタから配置した敵もプールに含むため
		int32 SubPoolSize = 0;

		TObjectPtr<UEnemyPool> EnemyPool = NewObject<UEnemyPool>(this);

		//Enemyのタグを持つ全ての敵をセット
		for (AActor* EnemyActor : EnemyActors)
		{
			if (EnemyActor->ActorHasTag(*Row->EnemyName)) {
				++SubPoolSize;//減算量加算
				AEnemyBase* Enemy = Cast<AEnemyBase>(EnemyActor);
				m_ActiveEnemies.Add(Enemy);
				EnemyPool->AddEnemyPool(Enemy);
			}
		}

		//プール生成&敵にパラメーターセット
		EnemyPool->GeneratePool(EnemyFactory, *Row->EnemyName, Row->PoolSize - SubPoolSize);
		EnemyPool->SetEnemyParam(*Row);

		m_EnemyPools.Add(*Row->EnemyName, EnemyPool);

		UE_LOG(LogTemp, Warning, TEXT("Name=%s, PoolSize=%d, RotationSpeed=%f"), *Row->EnemyName, Row->PoolSize, Row->RotationSpeed);
	}

}

void UEnemyManager::ActivateEnemy(const FString _name, const FVector _location, const FRotator _rotation, const FEnemyTypeFlags _enemyFlags) {
	//名前のプールから
	TObjectPtr<AEnemyBase> NewEnemy = m_EnemyPools[_name].Get()->GetAndActivateEnemy();


	NewEnemy->SetActorLocation(_location);
	NewEnemy->SetActorRotation(_rotation);

	if (_enemyFlags.bIsEventEnemy) {
		NewEnemy->SetIsEventEnemy(true);

		UEnemyNavigationManager* Subsystem = GetWorld()->GetSubsystem<UEnemyNavigationManager>();
		if (!Subsystem)return;

		Subsystem->AddChasingEnemy(NewEnemy);
	}

	if (_enemyFlags.bIsBoss) {
		NewEnemy->SetIsBoss(true);
		NewEnemy->HandleBossCase();
	}

	m_ActiveEnemies.Add(NewEnemy);
}

void UEnemyManager::ActivateEnemy(const EnemyType _type, const FVector _location, const FRotator _rotation, const FEnemyTypeFlags _enemyFlags) {
	FString EnemyNama = ConvertEnemyTypeToEnmeyName(_type);

	//名前のプールから
	TObjectPtr<AEnemyBase> NewEnemy = m_EnemyPools[EnemyNama].Get()->GetAndActivateEnemy();

	if (!IsValid(NewEnemy))return;

	NewEnemy->SetActorLocation(_location);
	NewEnemy->SetActorRotation(_rotation);

	//イベントの敵ならばフラグを上げる
	if (_enemyFlags.bIsEventEnemy) {
		NewEnemy->SetIsEventEnemy(true);

		UEnemyNavigationManager* Subsystem = GetWorld()->GetSubsystem<UEnemyNavigationManager>();
		if (!Subsystem)return;

		Subsystem->AddChasingEnemy(NewEnemy);
	}

	if (_enemyFlags.bIsBoss) {
		NewEnemy->SetIsBoss(true);
		NewEnemy->HandleBossCase();
		NewEnemy->SetActorLocation(_location);
	}

	m_ActiveEnemies.Add(NewEnemy);

}

const AEnemyBase* UEnemyManager::GetClosestActiveEnemyFromCoordinates(const FVector _location) {
	float ClosestLentgh = -1.f;
	int8 ClosestEnemyIndex = -1;

	//座標と一番近い敵を探索
	for (int i = 0; i < m_ActiveEnemies.Num(); ++i) {
		float LentghToEnemy = (_location - m_ActiveEnemies[i]->GetActorLocation()).Size();
		if (ClosestLentgh < 0.f) {
			ClosestLentgh = LentghToEnemy;
			ClosestEnemyIndex = i;
		}

		if (LentghToEnemy < ClosestLentgh) {
			ClosestLentgh = LentghToEnemy;
			ClosestEnemyIndex = i;
		}

	}

	if (ClosestEnemyIndex < 0)return nullptr;

	return m_ActiveEnemies[ClosestEnemyIndex];
}


void UEnemyManager::RemoveFromActiveEnemies(const AEnemyBase* _enemy) {
	for (int i = 0; i < m_ActiveEnemies.Num(); ++i) {

		if (!m_ActiveEnemies[i])continue;

		if (m_ActiveEnemies[i] == _enemy) {
			m_ActiveEnemies.RemoveAtSwap(i);
			break;
		}
	}
}

FString	UEnemyManager::ConvertEnemyTypeToEnmeyName(const EnemyType _type) {
	FString Result = "";

	switch (_type)
	{
	case EnemyType::Grunt:
		Result = "Enemy_1";
		break;
	case EnemyType::GruntGuard:
		Result = "Enemy_2";
		break;

	case EnemyType::TrojanHouse:
		Result = "TrojanHorse";
		break;
	default:

		break;
	}

	return Result;
}

void UEnemyManager::PauseAllActiveEnemies(const bool _isPause) {
	for (int i = 0; i < m_ActiveEnemies.Num(); ++i) {
		m_ActiveEnemies[i]->PauseEnemy(_isPause);
	}
}

void UEnemyManager::DeactivateAllEventEnemies() {
	//配列削除するので逆から回す
	for (int i = m_ActiveEnemies.Num() - 1; i >= 0; --i) {

		if (m_ActiveEnemies[i]->GetIsEventEnemy()) {
			m_ActiveEnemies[i]->SetActive(false);
			//アクティブな敵から消去
			RemoveFromActiveEnemies(m_ActiveEnemies[i]);
		}
	}
}

const AEnemyBase* UEnemyManager::GetActiveBossEnemy()const {
	AEnemyBase* BossEnemy = nullptr;

	for (int i = 0; i < m_ActiveEnemies.Num(); ++i) {
		if (m_ActiveEnemies[i]->GetIsBossEnemy()) {
			BossEnemy = m_ActiveEnemies[i];
			break;
		}
	}

	return BossEnemy;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPool.h"
#include "../EnemyBase.h"
#include "../EnemyFactory/EnemyFactory.h"


void UEnemyPool::GeneratePool(AEnemyFactory* _enemyFactiry, const FString _name, const int _poolSize) {
	if (!_enemyFactiry) {
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Not Generate Pool!!!!!"));
		return;
	}

	FVector GenerateLocation = FVector::ZeroVector;
	FRotator GenerateRotation = FRotator::ZeroRotator;
	UWorld* GenerateWorld = GetWorld();

	//名前を設定しておく
	m_EnemyType = _name;


	for (int i = 0; i < _poolSize; ++i) {
		//敵生成
		TObjectPtr<AEnemyBase> NewEnemy = _enemyFactiry->CreateEnemy(GenerateWorld, _name, GenerateLocation, GenerateRotation);

		if (!NewEnemy) {
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, _name + TEXT("Can not Create!!!"));
			break;
		}

		//タグをセットしておく(CSVでの名前変更の影響を受けるためここでセット)
		NewEnemy->Tags.AddUnique((FName)m_EnemyType);

		NewEnemy->SetActive(false);

		//プールに追加
		m_EnemyPool.Add(NewEnemy);
	}
}

void UEnemyPool::BeginDestroy() {
	Super::BeginDestroy();

	//プールを空に
	m_EnemyPool.Empty();
}


TObjectPtr<AEnemyBase> UEnemyPool::GetAndActivateEnemy() {
	AEnemyBase* ResultEnemy = nullptr;

	for (int i = 0; i < m_EnemyPool.Num(); ++i) {
		//非アクティブな敵を見つけたらそれを入れる
		if (!m_EnemyPool[i]->GetIsActive()) {
			ResultEnemy = m_EnemyPool[i];
			//敵をアクティブに
			ResultEnemy->SetActive(true);
			break;
		}
	}

	if (ResultEnemy == nullptr) {
		GEngine->AddOnScreenDebugMessage(-1, 300.f, FColor::Red,  TEXT("GetAndActivateEnemy Enemy nullptr!!!"));
	}

	return TObjectPtr<AEnemyBase>(ResultEnemy);
}

void UEnemyPool::AddEnemyPool(TObjectPtr<AEnemyBase> _enemy = nullptr) {
	if (!_enemy) {
		GEngine->AddOnScreenDebugMessage(-1, 300.f, FColor::Red, TEXT("AddEnemyPool Enemy nullptr!!!"));
		return;
	}
	m_EnemyPool.Add(_enemy);
}

void UEnemyPool::SetEnemyParam(const FEnemyDataBase _enemydata) {
	for (int i = 0; i < m_EnemyPool.Num(); ++i) {
		m_EnemyPool[i]->SetEnemyParam(_enemydata);
	}
}
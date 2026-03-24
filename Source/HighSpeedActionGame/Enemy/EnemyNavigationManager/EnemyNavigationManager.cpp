// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyNavigationManager.h"
#include "../../WorldSettings/MyWorldSettings.h"
#include "../../GameUtility/GameUtility.h"
#include "../EnemyBase.h"


namespace {
	constexpr float TentativeLength = 50.f;
}

bool UEnemyNavigationManager::ShouldCreateSubsystem(UObject* Outer) const {
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

void UEnemyNavigationManager::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("UEnemyNavigationManager::Initialize!!!"));
	}

}

void UEnemyNavigationManager::Deinitialize() {
	Super::Deinitialize();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("UEnemyNavigationManager::Deinitialize!!!"));
	}

	m_ChasingEnemies.Empty();
}

void UEnemyNavigationManager::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (bTickEnabled) {
		SetChasingEnemiesTargetLocations();
	}
}

void UEnemyNavigationManager::AddChasingEnemy(AEnemyBase* _chasingEnemy) {
	if (_chasingEnemy) {
		m_ChasingEnemies.AddUnique(_chasingEnemy);

		if (!m_ChasingEnemies.IsEmpty()) {
			bTickEnabled = true;
		}

		SetChasingEnemiesTargetLocations();
	}
}

void UEnemyNavigationManager::RemoveChasingEnemy(AEnemyBase* _chasingEnemy) {
	if (_chasingEnemy)
	{
		m_ChasingEnemies.RemoveSwap(_chasingEnemy);

		if (m_ChasingEnemies.IsEmpty()) {
			bTickEnabled = false;
		}

		SetChasingEnemiesTargetLocations();
	}
}
TStatId UEnemyNavigationManager::GetStatId() const {
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMyTickableWorldSubsystem, STATGROUP_Tickables);
}

void UEnemyNavigationManager::SetChasingEnemiesTargetLocations() {
	if (m_ChasingEnemies.IsEmpty())return;

	//プレイヤーがnullptrなら検索
	if (!PlayerChara) {
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC)return;

		PlayerChara = PC->GetPawn();
		if (!PlayerChara)return;
	}

	int32 ChasingEnemiesNum = m_ChasingEnemies.Num();
	FVector PlayerPos = PlayerChara->GetActorLocation();

	FVector PlayerForwardVec = (m_ChasingEnemies[0]->GetActorLocation() - PlayerPos).GetSafeNormal();

	TArray<FVector> TargetPoss;
	TargetPoss.Reserve(ChasingEnemiesNum);

	//敵の数分目的地を生成
	for (int i = 0; i < ChasingEnemiesNum; ++i) {
		float angle = 360.f / ChasingEnemiesNum * i;
		FVector Offset = PlayerForwardVec.RotateAngleAxis(angle, FVector::UpVector) * TentativeLength;


		TargetPoss.Add(PlayerPos + Offset);
	}

	//貪欲方で位置割り当て
	for (int i = 0; i < ChasingEnemiesNum; ++i) {
		if (!m_ChasingEnemies[i] || TargetPoss.IsEmpty())continue;

		float MinDistance = FLT_MAX;//floatのMax
		int32 BestIndex = INDEX_NONE;

		FVector EnemyPos = m_ChasingEnemies[i]->GetActorLocation();

		for (int n = 0; n < TargetPoss.Num(); ++n) {
			float Distance = FVector::DistSquared(EnemyPos, TargetPoss[n]);

			//長さが最小値より小さかったら更新
			if (Distance < MinDistance) {
				BestIndex = n;
				MinDistance = Distance;
			}
		}

		if (BestIndex != INDEX_NONE) {
			//位置を指定
			FVector TargetLocation = PlayerPos + (TargetPoss[BestIndex] - PlayerPos).GetSafeNormal() * m_ChasingEnemies[i]->GetDistanceToTarget();
			/*m_ChasingEnemies[i]->SetBBMoveToLocation(TargetLocation, "MoveToLocation");*/
			m_ChasingEnemies[i]->SetMoveTargetLocation(TargetLocation);

			//UE_LOG(LogTemp, Warning, TEXT("Num:%s ,X: %f, Y: %f, Z: %f"), *FString::FromInt(i), TargetLocation.X, TargetLocation.Y, TargetLocation.Z);

			//位置を配列から外す
			TargetPoss.RemoveAtSwap(BestIndex);
		}
	}
}
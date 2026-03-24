// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode_Ingame.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "../PlayerCharacter.h"
#include "../PlayerComponent/Player_CameraComponent.h"
#include "../Event/GameplayAreaEventManager/GameplayAreaEventManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "../Enemy/EnemyManager/EnemyManager.h"
#include "../PlayerController/MainPlayerController.h"


AGameMode_Ingame::AGameMode_Ingame()
{
	BGMManager = CreateDefaultSubobject<UGameBGMComponent>(TEXT("BGMManager"));
}

void AGameMode_Ingame::BeginPlay()
{
	Super::BeginPlay();

	if (BGMManager)
	{
		BGMManager->PlayNormalBGM();
	}

	//初期復帰地点 = PlayerStart
	for (TActorIterator<APlayerStart>IT(GetWorld()); IT; ++IT)
	{
		RespawnTransform = IT->GetActorTransform();
		break;
	}

	if (UGameplayAreaEventManager* Manager = GetWorld()->GetSubsystem<UGameplayAreaEventManager>())
	{
		Manager->OnBossDead.AddDynamic(this, &AGameMode_Ingame::OnGameClear);
		Manager->OnBossActive.AddDynamic(this, &AGameMode_Ingame::OnBossBattleStart);
		Manager->OnBossActive.AddDynamic(this, &AGameMode_Ingame::PlayBossSequenceIfAvailable);
		Manager->OnBeginPlay();
	}


}


void AGameMode_Ingame::RespawnPlayer(APlayerCharacter* Player)
{
	if (!Player)return;

	if (BGMManager)
	{
		BGMManager->PlayNormalBGM();
	}

	Player->SetActorTransform(
		RespawnTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	Player->OnRespawn();
}

void AGameMode_Ingame::SetRespawnTransform(const FTransform& NewTransform)
{
	RespawnTransform = NewTransform;
}

void AGameMode_Ingame::RequestRespawn(APlayerCharacter* Player)
{
	if (!Player) return;

	FTimerHandle RespawnTimerHandle;

	// 3秒後に復帰
	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		[this, Player]()
		{
			RespawnPlayer(Player);
		},
		5.0f,
		false
	);


}


void AGameMode_Ingame::OnBossBattleStart()
{
	if (BGMManager)
	{
		BGMManager->PlayBossBGM();
	}
}

void AGameMode_Ingame::OnGameClear() {

	// Widgetが存在するかチェック
	if (BGMManager)
	{
		BGMManager->PlayClearBGM();
	}

	if (m_CurrentWidgetInstance)
	{
		//プレイヤーUIを画面から消す
		m_CurrentWidgetInstance->RemoveFromParent();

		// 念のため空にしておく（再利用しない場合）
		m_CurrentWidgetInstance = nullptr;
	}

	if (m_PostProcessActor)
	{
		FVector SpawnLocation(0.f, 0.f, 0.f); // 生成位置例：任意設定可
		FRotator SpawnRotation(0.f, 0.f, 0.f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(m_PostProcessActor, SpawnLocation, SpawnRotation, SpawnParams);
		if (SpawnedActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("OnGameClear: Actor spawned at %s"), *SpawnLocation.ToString());
			// 追加で初期設定や配置ロジックがあればここに
		}
	}

	UEnemyManager* Manager = GetWorld()->GetSubsystem<UEnemyManager>();
	if (Manager) {
		Manager->PauseAllActiveEnemies(true);
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APlayerCharacter* Player;
	if (PC) {
		Player = Cast<APlayerCharacter>(PC->GetPawn());

		if (Player) {
			Player->PausePlayer(true);
		}

		if (AMainPlayerController* MainPC = Cast<AMainPlayerController>(PC))
		{
			MainPC->EnterResultMode();
		}
	}
}

void AGameMode_Ingame::PlayBossSequenceIfAvailable() {

}
// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode_Ingame.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../Event/GameplayAreaEventManager/GameplayAreaEventManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "../Enemy/EnemyManager/EnemyManager.h"
#include "../PlayerController/MainPlayerController.h"

//初期化
AGameMode_Ingame::AGameMode_Ingame()
{
	BGMManager = CreateDefaultSubobject<UGameBGMComponent>(TEXT("BGMManager"));
}

//開始時
void AGameMode_Ingame::BeginPlay()
{
	Super::BeginPlay();

	if (BGMManager)
	{
		BGMManager->PlayNormalBGM();
	}

	//初期復帰地点をPlayerStartに設定
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		RespawnTransform = It->GetActorTransform();
		break;
	}

	//ゲームエリアイベントを登録
	if (UGameplayAreaEventManager* Manager = GetWorld()->GetSubsystem<UGameplayAreaEventManager>())
	{
		Manager->OnBossDead.AddDynamic(this, &AGameMode_Ingame::OnGameClear);
		Manager->OnBossActive.AddDynamic(this, &AGameMode_Ingame::OnBossBattleStart);
		Manager->OnBossActive.AddDynamic(this, &AGameMode_Ingame::PlayBossSequenceIfAvailable);
		Manager->OnBeginPlay();
	}
}

//プレイヤーをリスポーン
void AGameMode_Ingame::RespawnPlayer(APlayerCharacter* Player)
{
	if (!Player) return;

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

//リスポーン地点を設定
void AGameMode_Ingame::SetRespawnTransform(const FTransform& NewTransform)
{
	RespawnTransform = NewTransform;
}

//リスポーンを要求
void AGameMode_Ingame::RequestRespawn(APlayerCharacter* Player)
{
	if (!Player) return;

	FTimerHandle RespawnTimerHandle;

	//タイマーでリスポーンを遅延実行
	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		[this, Player]()
		{
			RespawnPlayer(Player);
		},
		RespawnWaitTime,
		false
	);
}

//Boss戦開始時の処理
void AGameMode_Ingame::OnBossBattleStart()
{
	if (BGMManager)
	{
		BGMManager->PlayBossBGM();
	}
}

//ゲームクリア時の処理
void AGameMode_Ingame::OnGameClear()
{
	if (BGMManager)
	{
		BGMManager->PlayClearBGM();
	}

	if (m_CurrentWidgetInstance)
	{
		//プレイヤーUIを画面から消す
		m_CurrentWidgetInstance->RemoveFromParent();
		m_CurrentWidgetInstance = nullptr;
	}

	if (m_PostProcessActor)
	{
		const FVector SpawnLocation = FVector::ZeroVector;
		const FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* SpawnedActor =
			GetWorld()->SpawnActor<AActor>(
				m_PostProcessActor,
				SpawnLocation,
				SpawnRotation,
				SpawnParams
			);

		if (SpawnedActor)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("OnGameClear: Actor spawned at %s"),
				*SpawnLocation.ToString()
			);
		}
	}

	if (UEnemyManager* Manager = GetWorld()->GetSubsystem<UEnemyManager>())
	{
		Manager->PauseAllActiveEnemies(true);
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(PC->GetPawn());
	if (Player)
	{
		Player->PausePlayer(true);
	}

	if (AMainPlayerController* MainPC = Cast<AMainPlayerController>(PC))
	{
		MainPC->EnterResultMode();
	}
}

//Boss開始シーケンス再生
void AGameMode_Ingame::PlayBossSequenceIfAvailable()
{
}
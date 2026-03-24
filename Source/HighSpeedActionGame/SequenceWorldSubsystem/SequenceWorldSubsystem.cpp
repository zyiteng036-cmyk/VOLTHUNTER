// Fill out your copyright notice in the Description page of Project Settings.


#include "SequenceWorldSubsystem.h"
#include "LevelSequenceActor.h"     
#include "LevelSequencePlayer.h"
#include "../WorldSettings/MyWorldSettings.h"
#include "../Event/GameplayAreaEventManager/GameplayAreaEventManager.h"

bool USequenceWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	if (UWorld* WorldOuter = Cast<UWorld>(Outer))
	{
		if (AMyWorldSettings* MyWorldSettings = Cast<AMyWorldSettings>(WorldOuter->GetWorldSettings()))
		{
			return MyWorldSettings->m_SequenceWorldSubsystem;
		}
	}

	return false;
}

void USequenceWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);


	FTimerHandle TempHandle;
	GetWorld()->GetTimerManager().SetTimer(TempHandle, this, &USequenceWorldSubsystem::HandleOnBossActive, 0.1f, false);
	m_HasPlayedBossCutscene = false;
}

void USequenceWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USequenceWorldSubsystem::HandleOnBossActive()
{
	if (UGameplayAreaEventManager* Manager = GetWorld()->GetSubsystem<UGameplayAreaEventManager>())
	{
		Manager->OnBossActive.AddDynamic(this, &USequenceWorldSubsystem::PlayBossSeqence);
	}
}


void USequenceWorldSubsystem::PlayBossSeqence()
{
	if (m_HasPlayedBossCutscene)return;

	UWorld* WorldOuter = GetWorld();
	if (!WorldOuter)return;

	AMyWorldSettings* MyWorldSettings = Cast<AMyWorldSettings>(WorldOuter->GetWorldSettings());
	if (!MyWorldSettings)return;

	ALevelSequenceActor* BossSeqence = MyWorldSettings->BossSequenceActor;

	if (!BossSeqence)return;

	ULevelSequencePlayer* Player = BossSeqence->GetSequencePlayer();

	if (!Player || Player->IsPlaying()) return;

	Player->OnFinished.AddDynamic(this, &USequenceWorldSubsystem::HandleBossSequenceFinished);

	Player->Play();

	//m_HasPlayedBossCutscene = true;
}

void USequenceWorldSubsystem::HandleBossSequenceFinished() {
	if (OnBossSequenceFinished.IsBound()) {
		OnBossSequenceFinished.Broadcast();
	}
}
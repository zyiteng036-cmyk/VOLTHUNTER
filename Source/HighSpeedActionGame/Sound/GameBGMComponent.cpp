// Fill out your copyright notice in the Description page of Project Settings.


#include "GameBGMComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UGameBGMComponent::UGameBGMComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// ...
}

void UGameBGMComponent::PlayNormalBGM()
{
	PlayBGM(NormalBGM);
}

void UGameBGMComponent::PlayBossBGM()
{
	PlayBGM(BossBGM);
}

void UGameBGMComponent::PlayClearBGM()
{
	PlayBGM(ClearBGM);
}

void UGameBGMComponent::StopBGM()
{
	if (CurrentAudioComponent)
	{
		CurrentAudioComponent->Stop(); // 即停止
		CurrentAudioComponent = nullptr;
	}
}

void UGameBGMComponent::PlayBGM(USoundBase* NewSound)
{
	// 音が設定されていないなら何もしない
	if (!NewSound) return;

	// 今鳴っている音と同じなら無視（連打対策）
	if (CurrentAudioComponent && CurrentAudioComponent->GetSound() == NewSound && CurrentAudioComponent->IsPlaying())
	{
		return;
	}

	//前の曲を即停止
	if (CurrentAudioComponent)
	{
		CurrentAudioComponent->Stop();
	}

	//新しい曲を即再生
	CurrentAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), NewSound);
}



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
		CurrentAudioComponent->Stop(); // ‘¦’â~
		CurrentAudioComponent = nullptr;
	}
}

void UGameBGMComponent::PlayBGM(USoundBase* NewSound)
{
	//‰¹‚ªİ’è‚³‚ê‚Ä‚¢‚È‚¢‚È‚ç‰½‚à‚µ‚È‚¢
	if (!NewSound) return;

	//¡–Â‚Á‚Ä‚¢‚é‰¹‚Æ“¯‚¶‚È‚ç–³‹
	if (CurrentAudioComponent && CurrentAudioComponent->GetSound() == NewSound && CurrentAudioComponent->IsPlaying())
	{
		return;
	}

	//‘O‚Ì‹È‚ğ‘¦’â~
	if (CurrentAudioComponent)
	{
		CurrentAudioComponent->Stop();
	}

	//V‚µ‚¢‹È‚ğ‘¦Ä¶
	CurrentAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), NewSound);
}



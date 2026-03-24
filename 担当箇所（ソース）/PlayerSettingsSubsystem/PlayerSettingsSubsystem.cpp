// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSettingsSubsystem.h"

void UPlayerSettingsSubsystem::Deinitialize()
{
	if (GEngine)
	{
		GEngine->DisplayGamma = 2.2f;
	}

	Super::Deinitialize();
}

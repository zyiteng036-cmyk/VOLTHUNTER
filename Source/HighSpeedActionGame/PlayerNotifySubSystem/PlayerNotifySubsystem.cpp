// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerNotifySubsystem.h"

void UPlayerNotifySubsystem::NotifyJustEvasive(const AActor* Attacker)
{
	UE_LOG(LogTemp, Log, TEXT("Subsystem: Just Evasive Notification Received!"));

	if (OnJustEvasiveOccurred.IsBound())
	{
		OnJustEvasiveOccurred.Broadcast(Attacker);
	}
}

void UPlayerNotifySubsystem::NotifyPlayerDying(AActor* DyingActor)
{

	if (OnPlayerDyingOccurred.IsBound())
	{
		OnPlayerDyingOccurred.Broadcast(DyingActor);
	}
}

void UPlayerNotifySubsystem::NotifyPlayerDied(AActor* DeadActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Subsystem: Player Died Notification Received!"));

	if (OnPlayerDiedOccurred.IsBound())
	{
		OnPlayerDiedOccurred.Broadcast(DeadActor);
	}
}

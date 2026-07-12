// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerNotifySubsystem.h"

void UPlayerNotifySubsystem::NotifyJustEvasive(const AActor* Attacker)
{

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

	if (OnPlayerDiedOccurred.IsBound())
	{
		OnPlayerDiedOccurred.Broadcast(DeadActor);
	}
}

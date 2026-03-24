// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerAttackSound_NotifyState.h"
#include "Kismet/GameplayStatics.h"
#include "../../PlayerCharacter.h"
#include "../../PlayerComponent/Player_EvasiveComponent.h"

void UPlayerAttackSound_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	bHasHitSomething = false;
}

void UPlayerAttackSound_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	if (!MeshComp) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return;

	// ƒqƒbƒg”»’è‚ª‚ ‚éê‡
	if (PlayerCharacter->GetIsHit() && !bHasHitSomething)
	{
		bHasHitSomething = true;

		if (HitSoundToPlay)
		{
			UGameplayStatics::PlaySoundAtLocation(PlayerCharacter, HitSoundToPlay, PlayerCharacter->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("SoundHit"));
		}

		// ‰ñ”ð‰Â”\‚É‚·‚é
		if (UPlayer_EvasiveComponent* EvasiveComp = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>())
		{
			EvasiveComp->SetCanEvasive(true);
		}
	}
}

void UPlayerAttackSound_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!bHasHitSomething)
	{
		if (!MeshComp) return;

		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
		if (!PlayerCharacter) return;

		if (NoHitSoundToPlay)
		{
			UGameplayStatics::PlaySoundAtLocation(PlayerCharacter, NoHitSoundToPlay, PlayerCharacter->GetActorLocation());
		}
	}
}

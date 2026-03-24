// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttackLightSoundNotify.h"
#include "Kismet/GameplayStatics.h"
#include "../../PlayerCharacter.h"
#include "../../PlayerComponent/Player_EvasiveComponent.h"
#include "../../PlayerComponent/Player_ElectroGaugeComponent.h"

void UPlayerAttackLightSoundNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp || !HitSoundToPlay || !NoHitSoundToPlay) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		//ヒットしたとき
		if (PlayerCharacter->GetIsHit())
		{
			// キャラクターの位置で再生
			UGameplayStatics::PlaySoundAtLocation(Owner, HitSoundToPlay, Owner->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("SoundHit"));
		}
		//ヒットしなかったとき
		else if (!PlayerCharacter->GetIsHit())
		{

			// キャラクターの位置で再生
			UGameplayStatics::PlaySoundAtLocation(Owner, NoHitSoundToPlay, Owner->GetActorLocation());
		}


		if (UPlayer_EvasiveComponent* EvasiveComp = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>())
		{
			EvasiveComp->SetCanEvasive(true);
		}

	}


}

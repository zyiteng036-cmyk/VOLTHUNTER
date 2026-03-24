// Fill out your copyright notice in the Description page of Project Settings.


#include "AddElectGauge_Notifiy.h"
#include "../../PlayerCharacter.h"
#include "../../PlayerComponent/Player_ElectroGaugeComponent.h"

void UAddElectGauge_Notifiy::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	APlayerCharacter* Player =
		Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!Player) return;

	UPlayer_ElectroGaugeComponent* ElectroComp =
		Player->FindComponentByClass<UPlayer_ElectroGaugeComponent>();
	if (!ElectroComp) return;
	//ƒqƒbƒg‚µ‚½‚Æ‚«
	if (Player->GetIsHit())
	{
		ElectroComp->AddElectroGauge(m_AddGaugeAmount);
	}
}

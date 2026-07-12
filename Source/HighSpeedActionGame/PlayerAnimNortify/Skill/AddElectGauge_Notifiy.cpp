#include "AddElectGauge_Notifiy.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerElectroGaugeComponent.h"
#include "Components/SkeletalMeshComponent.h"

//UŒ‚–½’†‚É“d—ÍƒQ[ƒW‚ğ‰ÁZ
void UAddElectGauge_Notifiy::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return;

	//UŒ‚‚ª–½’†‚µ‚Ä‚¢‚È‚¢ê‡‚ÍƒQ[ƒW‚ğ‰ÁZ‚µ‚È‚¢
	if (!PlayerCharacter->GetIsHit()) return;

	UPlayerElectroGaugeComponent* ElectroGaugeComponent = PlayerCharacter->FindComponentByClass<UPlayerElectroGaugeComponent>();
	if (!ElectroGaugeComponent) return;

	//Notify‚Éİ’è‚³‚ê‚½—Ê‚ğŒ»İ‚Ì“d—ÍƒQ[ƒW‚Ö‰ÁZ
	ElectroGaugeComponent->AddElectroGauge(m_AddGaugeAmount);
}
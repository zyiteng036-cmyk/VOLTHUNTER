// Fill out your copyright notice in the Description page of Project Settings.


#include "EventTriggerVolume.h"
#include "Components/BoxComponent.h"
#include "../EventActor/EventActorBase.h"
#include "../GameplayAreaEventManager/GameplayAreaEventManager.h"

// Sets default values
AEventTriggerVolume::AEventTriggerVolume()
	:m_IsEventExecution(false)
	, m_EventActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = m_TriggerBox;
	m_TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	m_TriggerBox->SetGenerateOverlapEvents(true);

}

// Called when the game starts or when spawned
void AEventTriggerVolume::BeginPlay()
{
	Super::BeginPlay();

	if (!m_TriggerBox)return;
	m_TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &AEventTriggerVolume::OnOverlapBegin);

}

// Called every frame
void AEventTriggerVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEventTriggerVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if (!OtherActor->ActorHasTag("Player"))return;

	if (!m_IsEventExecution) {

		UWorld* World = GetWorld();

		if (!World)return;

		UGameplayAreaEventManager* Subsystem = World->GetSubsystem<UGameplayAreaEventManager>();

		if (!Subsystem)return;

		if (!Subsystem->GetIsDuringTheEvent()) {
			m_IsEventExecution = true;

			Subsystem->StartEvent(*this);

			if (!m_EventActor.IsEmpty()) {
				for (int i = 0; i < m_EventActor.Num(); ++i) {
					if (!m_EventActor[i])continue;

					m_EventActor[i]->SetActive(true);
				}
			}
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Event Start"));
		}

	}

}
UEventDataBase* AEventTriggerVolume::GetEventData()const {
	return m_EventData;
}

void AEventTriggerVolume::EventEnd() {
	if (!m_EventActor.IsEmpty()) {
		for (int i = 0; i < m_EventActor.Num(); ++i) {
			if (!m_EventActor[i])continue;

			m_EventActor[i]->SetActive(false);
		}
	}
}


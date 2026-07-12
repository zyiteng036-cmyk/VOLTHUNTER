// Fill out your copyright notice in the Description page of Project Settings.

#include "TutorialGuidanceTrigger.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "../PlayerCharacter/PlayerCharacter.h"

// Sets default values
ATutorialGuidanceTrigger::ATutorialGuidanceTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	//コリジョン設定
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

// Called when the game starts or when spawned
void ATutorialGuidanceTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (!TriggerBox) return;

	//イベント登録
	TriggerBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&ATutorialGuidanceTrigger::OnOverlapBegin
	);

	TriggerBox->OnComponentEndOverlap.AddDynamic(
		this,
		&ATutorialGuidanceTrigger::OnOverlapEnd
	);
}

//プレイヤーが範囲に入った時
void ATutorialGuidanceTrigger::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	if (!TutorialWidgetClass) return;

	if (!m_TutorialWidgetInstance)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (!PC) return;

		m_TutorialWidgetInstance =
			CreateWidget<UUserWidget>(
				PC,
				TutorialWidgetClass
			);
	}

	if (m_TutorialWidgetInstance && !m_TutorialWidgetInstance->IsInViewport())
	{
		m_TutorialWidgetInstance->AddToViewport();
	}
}

//プレイヤーが範囲から出た時
void ATutorialGuidanceTrigger::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	if (m_TutorialWidgetInstance)
	{
		m_TutorialWidgetInstance->RemoveFromParent();
	}
}
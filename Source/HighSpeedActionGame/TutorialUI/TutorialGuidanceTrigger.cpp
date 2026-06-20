// Fill out your copyright notice in the Description page of Project Settings.


#include "TutorialGuidanceTrigger.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "../PlayerCharacter.h"
#include "../PlayerComponent/Player_CameraComponent.h"


// Sets default values
ATutorialGuidanceTrigger::ATutorialGuidanceTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
	
	//イベント
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATutorialGuidanceTrigger::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ATutorialGuidanceTrigger::OnOverlapEnd);

}

void ATutorialGuidanceTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//プレイヤーかどうか判定
	if (OtherActor && OtherActor->IsA(APlayerCharacter::StaticClass()))
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
		{
			if (UPlayer_CameraComponent* CameraComp = Player->FindComponentByClass<UPlayer_CameraComponent>())
			{
				// カメラを右に150ずらす（プレイヤーが左に見える）
				CameraComp->SetCameraSideOffset(true, 150.f);
			}
		}
		if (TutorialWidgetClass)
		{
			if (!m_TutorialWidgetInstance)
			{
				APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
				m_TutorialWidgetInstance = CreateWidget<UUserWidget>(PC, TutorialWidgetClass);
			}

			//画面に追加
			if (m_TutorialWidgetInstance && !m_TutorialWidgetInstance->IsInViewport())
			{
				m_TutorialWidgetInstance->AddToViewport();
			}
		}
	}
}

void ATutorialGuidanceTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//プレイヤーが離れたか判定
	if (OtherActor && OtherActor->IsA(APlayerCharacter::StaticClass()))
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
		{
			if (UPlayer_CameraComponent* CameraComp = Player->FindComponentByClass<UPlayer_CameraComponent>())
			{
				CameraComp->SetCameraSideOffset(false);
			}
		}
		if (m_TutorialWidgetInstance)
		{
			m_TutorialWidgetInstance->RemoveFromParent();
		}
	}
}
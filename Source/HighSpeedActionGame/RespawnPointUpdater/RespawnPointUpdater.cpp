// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnPointUpdater.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h" 
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../GameMode/GameMode_Ingame.h"
#include "../Scanner/ScannerActor.h"
#include "../PlayerCharacter.h"

// Sets default values
ARespawnPointUpdater::ARespawnPointUpdater()
	:m_HPAdditionAmount(50)
	,m_IsPlayerOverlapping(false)
	,m_OverlappingPlayer(nullptr)
	,m_RespawnRotation(FRotator::ZeroRotator)

{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// スタティックメッシュを作成してルートコンポーネントに設定
	m_StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = m_StaticMeshComponent;

	m_PlayerDitectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerDitectionBox"));
	m_PlayerDitectionBox->SetupAttachment(RootComponent);
	m_PlayerDitectionBox->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	m_PlayerDitectionBox->SetGenerateOverlapEvents(true);
	m_PlayerDitectionBox->SetCollisionProfileName(TEXT("Trigger"));

	m_PlayerDitectionBox->OnComponentBeginOverlap.AddDynamic(this, &ARespawnPointUpdater::OnOverlapBegin);
	m_PlayerDitectionBox->OnComponentEndOverlap.AddDynamic(this, &ARespawnPointUpdater::OnOverlapEnd);
}

// Called when the game starts or when spawned
void ARespawnPointUpdater::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ARespawnPointUpdater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (m_IsPlayerOverlapping) {
		if (m_OverlappingPlayer.IsValid()) {
			//ここにHP加算処理書いて

			float HealAmount = m_HPAdditionAmount * DeltaTime;

			m_OverlappingPlayer->AddHealth(HealAmount);
		}
	}
}

void ARespawnPointUpdater::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (!OtherActor->ActorHasTag("Player"))return;
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);

	if (!PlayerCharacter)return;

	m_IsPlayerOverlapping = true;

	m_OverlappingPlayer = PlayerCharacter;

	if (ActivationSound)
	{
		//音を再生
		UGameplayStatics::PlaySoundAtLocation(this, ActivationSound, GetActorLocation());
	}

	if (ScannerEffectClass)
	{
		// 生成パラメータ
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 自分（リスポーン地点）と同じ場所に生成
		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = FRotator::ZeroRotator; // 回転は0でOK（あるいは GetActorRotation()）

		// 生成
		AScannerActor* SpawnedEffect = GetWorld()->SpawnActor<AScannerActor>(ScannerEffectClass, SpawnLocation, SpawnRotation, SpawnParams);

		//拡大開始の命令
		if (SpawnedEffect)
		{
			SpawnedEffect->StartScannerEffect();
		}
	}

	//現在のワールドのゲームモードを取得
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);

	AGameMode_Ingame* InGameMode = Cast<AGameMode_Ingame>(GameMode);

	if (InGameMode)
	{
		FTransform NewRespawnTransform = GetActorTransform();
		NewRespawnTransform.SetRotation(FQuat(m_RespawnRotation));

		InGameMode->SetRespawnTransform(NewRespawnTransform);
	}
}


void ARespawnPointUpdater::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex){
	if (!OtherActor->ActorHasTag("Player"))return;

	m_IsPlayerOverlapping = false;
	m_OverlappingPlayer = nullptr;
}
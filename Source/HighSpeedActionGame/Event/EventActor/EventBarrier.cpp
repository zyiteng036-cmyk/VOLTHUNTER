// Fill out your copyright notice in the Description page of Project Settings.


#include "EventBarrier.h"
#include "Components/BoxComponent.h"

// Sets default values for this actor's properties
AEventBarrier::AEventBarrier() {
	// Blocking Volume用Boxコンポーネント作成・アタッチ
	m_BlockingVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockingVolume"));
	if (m_BlockingVolume) {
		m_BlockingVolume->SetupAttachment(RootComponent);
		// コリジョン設定（物理ブロック）
		m_BlockingVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		m_BlockingVolume->SetCollisionObjectType(ECC_WorldStatic);
		m_BlockingVolume->SetCollisionResponseToAllChannels(ECR_Block);
	}
}

// Called when the game starts or when spawned
void AEventBarrier::BeginPlay() {
	Super::BeginPlay();

	SetActive(false);
}

// Called every frame
void AEventBarrier::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

}

void AEventBarrier::SetActive(const bool _isActive) {
	m_IsActive = _isActive;

	if (!m_BlockingVolume)return;
	SetActorHiddenInGame(!_isActive);//隠すかのフラグなのでフラグ反転
	SetActorEnableCollision(_isActive);

	if (_isActive) { m_BlockingVolume->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); }//コリジョンの反発はしない設定（overrapはする）
	else { m_BlockingVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision); }

	SetVisible(_isActive);
}


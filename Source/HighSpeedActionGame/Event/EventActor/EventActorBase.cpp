// Fill out your copyright notice in the Description page of Project Settings.


#include "EventActorBase.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AEventActorBase::AEventActorBase()
	:m_IsActive(false)
	, m_VisualMesh(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// ルート用SceneComponent（スケール1固定）
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	// StaticMesh生成
	m_VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("m_VisualMesh"));
	if (m_VisualMesh) {
		m_VisualMesh->SetupAttachment(RootComponent);
	}

}

// Called when the game starts or when spawned
void AEventActorBase::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void AEventActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEventActorBase::SetActive(const bool _isActive) {

}

void AEventActorBase::SetVisible(const bool _isVisible) {
	if (m_VisualMesh) {
		m_VisualMesh->SetVisibility(_isVisible, true);
	}
}
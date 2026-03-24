// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSword.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"


// Sets default values
APlayerSword::APlayerSword()
	:m_TrailInitialized(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	SetRootComponent(m_SwordMesh);
	m_SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_SwordMesh->SetMobility(EComponentMobility::Movable);

	m_TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComponent"));
	m_TrailComponent->SetupAttachment(m_SwordMesh);
	m_TrailComponent->SetAutoActivate(false);

}

void APlayerSword::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerSword has no Character owner"));
		return;
	}

	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	if (!OwnerMesh)
	{
		return;
	}

	AttachToComponent(
		OwnerMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		AttachSocketName
	);
}

void APlayerSword::SetAttackActive(bool _Active)
{
	if (!m_TrailComponent || !m_TrailNiagara)
	{
		return;
	}
	// === ‰‰ñ‚Ì‚Ý‰Šú‰» ===
	if (!m_TrailInitialized)
	{
		m_TrailComponent->SetAsset(m_TrailNiagara);
		m_TrailComponent->AttachToComponent(
			m_SwordMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale
		);
		m_TrailComponent->Deactivate(); // ”O‚Ì‚½‚ß
		m_TrailInitialized = true;
		UE_LOG(LogTemp, Error, TEXT("What?!SecondInitialized??"));
	}

	if (_Active)
	{
		if (!m_TrailComponent->IsActive())
		{
			m_TrailComponent->Activate();
		}
	}
	else
	{
		m_TrailComponent->Deactivate();
	}
}

void APlayerSword::SetSwordMeshVisibility(bool bVisible)
{
	if (m_SwordMesh)
	{
		m_SwordMesh->SetVisibility(bVisible, false);
	}
}

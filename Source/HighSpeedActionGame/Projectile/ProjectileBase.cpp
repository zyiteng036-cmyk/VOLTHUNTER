// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "../HitJudgmentComponent/HitJudgmentComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "../PlayerNotifySubSystem/PlayerNotifySubsystem.h"
#include "Kismet/GameplayStatics.h"

namespace {
	constexpr float MaxLifeTime = 10.f;//弾系の最大の寿命
}

// Sets default values
AProjectileBase::AProjectileBase()
	: m_TimeUntilDestroyed(1.5f)
	, m_Timer(0.f)
{
	PrimaryActorTick.bCanEverTick = true;

	// RootComponentが無ければ作成する（例: SceneComponent）
	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	}

	// 移動の基準となるコンポーネントをUpdatedComponentに設定
	m_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	m_ProjectileMovementComponent->SetUpdatedComponent(RootComponent);
	m_ProjectileMovementComponent->InitialSpeed = 3000.f;
	m_ProjectileMovementComponent->MaxSpeed = 3000.f;

	// NiagaraComponentはRootComponentにアタッチ
	m_NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	m_NiagaraComp->SetupAttachment(RootComponent);
	m_NiagaraComp->SetAutoActivate(true);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraSystemAsset(TEXT("/Game/Path/To/YourNiagaraSystem.YourNiagaraSystem"));
	if (NiagaraSystemAsset.Succeeded())
	{
		m_NiagaraComp->SetAsset(NiagaraSystemAsset.Object);
	}

	m_HitJudgmentComponent = CreateDefaultSubobject<UHitJudgmentComponent>(TEXT("HitJudgmentComponent"));


}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)return;

	UPlayerNotifySubsystem* Subsystem = LP->GetSubsystem<UPlayerNotifySubsystem>();
	if (!Subsystem)return;

	Subsystem->OnPlayerDiedOccurred.AddDynamic(this, &AProjectileBase::HandlePlayerDeath);
}



// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	OnCollisionTick();
	m_Timer += DeltaTime;


	if (m_Timer > m_TimeUntilDestroyed) {
		OnCollisionEnd();
		Destroy();
	}
}

void AProjectileBase::SetAttackCollisionParam(const FAttackCollisionParam& _param) {
	m_AttackCollisionParam = _param;
}

void AProjectileBase::SetDamageInfo(const FDamageInfo& _damageInfo) {
	m_DamageInfo = _damageInfo;
}
void AProjectileBase::OnCollisionBegin() {
	m_HitJudgmentComponent->BeginHitDetection(m_DamageInfo, m_AttackCollisionParam.Radius, m_AttackCollisionParam.RelativeLocation, *this, m_AttackCollisionParam.HitTag, MaxLifeTime);
	m_HitJudgmentComponent->SetAttackCollisionDetectionVisible(m_AttackCollisionParam.Visible);
}
void AProjectileBase::OnCollisionTick() {
	m_HitJudgmentComponent->PossibleHitTime(m_AttackCollisionParam.RelativeLocation);
}
void AProjectileBase::OnCollisionEnd() {
	m_HitJudgmentComponent->EndHitDetection();
}

void AProjectileBase::HandlePlayerDeath(AActor* _actor)
{
	Destroy();
}

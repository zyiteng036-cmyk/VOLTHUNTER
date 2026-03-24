// Fill out your copyright notice in the Description page of Project Settings.


#include "RailgunProjectile.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"


ARailgunProjectile::ARailgunProjectile() {
    // NiagaraComponentはRootComponentにアタッチ
    m_RailgunBottom = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RailgunBottom"));
    m_RailgunBottom->SetupAttachment(RootComponent);
    m_RailgunBottom->SetAutoActivate(true);

    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraSystemAsset(TEXT("/Game/Path/To/YourNiagaraSystem.YourNiagaraSystem"));
    if (NiagaraSystemAsset.Succeeded())
    {
        m_RailgunBottom->SetAsset(NiagaraSystemAsset.Object);
    }

}



void ARailgunProjectile::BeginPlay() {
    Super::BeginPlay();

    m_StartLocation = GetActorLocation();
}


void ARailgunProjectile::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    if (m_RailgunBottom)
    {
        FVector FixedLocation = m_StartLocation;
        FScopedPreventAttachedComponentMove PreventMoveScope(m_RailgunBottom);
        m_RailgunBottom->SetWorldLocation(FixedLocation);

        //現在の移動ベクトル取得
        FVector MoveVector = GetActorLocation() - m_StartLocation;

        //伸びる向き
        FVector Forward = GetActorForwardVector();

        //前方向にどれだけ進んだかを算出
        float DeltaLength = FVector::DotProduct(MoveVector, Forward);

        const float BottomInitialLength = 100.f;

        float YScale = DeltaLength / BottomInitialLength;

        m_RailgunBottom->SetNiagaraVariableFloat("User.Y_Scale Factor", YScale);
    }
}


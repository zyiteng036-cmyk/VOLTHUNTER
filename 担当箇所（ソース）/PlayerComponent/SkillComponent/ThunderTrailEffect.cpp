// Fill out your copyright notice in the Description page of Project Settings.


#include "ThunderTrailEffect.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AThunderTrailEffect::AThunderTrailEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	// プレイヤーの移動が終わった後に位置を更新するため、Tickグループを後ろにする
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	RootComponent = NiagaraComp;
	NiagaraComp->SetAutoActivate(true);
}

// Called when the game starts or when spawned
void AThunderTrailEffect::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();

	FixedRotation = GetActorRotation();

	FixedRotation.Pitch = 0.0f;
	FixedRotation.Roll = 0.0f;
}

// Called every frame
void AThunderTrailEffect::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);

	//フェードアウト処理
	if (bIsFinished)
	{
		//現在のスケール
		FVector CurrentScale = GetActorScale3D();
		
		float NewThickness = FMath::FInterpTo(CurrentScale.Y, 0.0f, DeltaTime, 10.0f);

		SetActorScale3D(FVector(CurrentScale.X, NewThickness, NewThickness));

		return;
	}

	AActor* Parent = GetAttachParentActor();

	if (Parent)
	{
		//開始地点開始アングルに固定
		SetActorLocation(StartLocation);
		SetActorRotation(FixedRotation);

		//距離計算
		FVector CurrentPlayerPos = Parent->GetActorLocation();

		//進行方向へ
		FVector MoveVector = CurrentPlayerPos - StartLocation;

		FVector ForwardVector = FRotationMatrix(FixedRotation).GetUnitAxis(EAxis::X);
		float ForwardDistance = FVector::DotProduct(MoveVector, ForwardVector);

		if (ForwardDistance < 0.0f) ForwardDistance = 0.0f;

		//スケール適用
		FVector NewScale = FVector(ForwardDistance / BaseEffectLength, ThicknessScale, ThicknessScale);
		SetActorScale3D(NewScale);
	}
}

void AThunderTrailEffect::BeginFadeOut()
{
	bIsFinished = true;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetLifeSpan(0.5f);
}


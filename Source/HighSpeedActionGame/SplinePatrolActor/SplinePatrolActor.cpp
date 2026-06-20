// Fill out your copyright notice in the Description page of Project Settings.


#include "SplinePatrolActor.h"
#include "Components/SplineComponent.h"

// Sets default values
ASplinePatrolActor::ASplinePatrolActor()
{
	PrimaryActorTick.bCanEverTick = false;

	m_Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = m_Spline;
}

// Called when the game starts or when spawned
void ASplinePatrolActor::BeginPlay()
{
	Super::BeginPlay();


}


FVector ASplinePatrolActor::GetSplinePointLocation(int32 _index)const {

	if (!m_Spline)return FVector::ZeroVector;

	return m_Spline->GetLocationAtSplinePoint(_index, ESplineCoordinateSpace::World);
}

int32 ASplinePatrolActor::GetSplinePointCount()const {
	return m_Spline->GetNumberOfSplinePoints();
}
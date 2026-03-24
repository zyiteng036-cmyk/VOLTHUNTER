// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttackData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class HIGHSPEEDACTIONGAME_API UAttackData : public UDataAsset
{
	GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float KnockbackScale = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector KnockbackDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HitStopTime = -1.f;
	
};

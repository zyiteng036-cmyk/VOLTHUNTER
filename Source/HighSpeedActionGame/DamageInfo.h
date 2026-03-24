//佐々木奏太担当
//ダメージインターフェースクラス

#pragma once

#include "CoreMinimal.h"
#include "DamageInfo.generated.h"

USTRUCT(BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float KnockbackScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector KnockbackDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	const AActor* AttackActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStopTime = -1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsEnhancedAttack = false;
};
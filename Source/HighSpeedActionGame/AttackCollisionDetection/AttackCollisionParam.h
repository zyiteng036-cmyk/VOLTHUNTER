//担当佐々木奏太
//攻撃判定用のパラメーター

#pragma once

#include "CoreMinimal.h"  
#include "AttackCollisionParam.generated.h"  

USTRUCT(BlueprintType)
struct FAttackCollisionParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackCollisionInfo")
	float Radius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackCollisionInfo")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackCollisionInfo")
	bool Visible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackCollisionInfo")
	TArray<FString> HitTag;

};
//佐々木奏太担当
//敵のデータベース構造体

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EnemyDataBase.generated.h"

USTRUCT(BlueprintType)
struct FEnemyDataBase : public FTableRowBase

{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EnemyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PoolSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPower = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DashSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationSpeed = 0.f;
};
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
    int32 PoolSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WalkSpeed;


    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DashSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RotationSpeed;
};
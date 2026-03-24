//佐々木奏太担当
//敵のフラグタイプ
#pragma once

#include "CoreMinimal.h"
#include "EnemyTypeFlags.generated.h"

USTRUCT(BlueprintType)
struct FEnemyTypeFlags
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bIsBoss = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bIsEventEnemy = false;
};
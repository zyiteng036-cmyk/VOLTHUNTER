//担当：佐々木奏太
//敵を全て倒すイベントで使用するデータクラス

#pragma once

#include "CoreMinimal.h"
#include "EventDataBase.h"
#include "../../Enemy/EnemyType.h"
#include "EventEnemyClearConditionData.generated.h"


USTRUCT(BlueprintType)
struct FEventEnemyData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	EnemyType m_EnmeyType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	bool m_IsBoss;
};
/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UEventEnemyClearConditionData : public UEventDataBase
{
	GENERATED_BODY()
public:
	UEventEnemyClearConditionData();
	virtual EventMode GetDataEventMode()const { return EventMode::EnemyClearCondition; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEventEnemyData> EventEnemyTypes;


	//生存できる敵の最大数
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MaximumNumberOfEnemiesPresent = 1;

	//敵が呼び出される場所
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> EnemySpawnPosition;

private:
	EventMode DataEventMode = EventMode::EnemyClearCondition;


};

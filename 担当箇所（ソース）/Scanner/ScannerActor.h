//担当
//伊藤直樹

//リスポーンに触れた際の見た目を生成するクラス

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScannerActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API AScannerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AScannerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//外部からこの関数を呼ぶと開始
	void StartScannerEffect();

private:
	// 自壊処理
	void DestroySelf();


protected:
	//表示用コンポーネント

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SphereMesh;

	// 拡大スピード (1秒あたりどれくらい大きくなるか)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Settings")
	float ExpansionSpeed;

	// 消えるまでの時間 (秒)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Settings")
	float LifeDuration;

private:
	// 動作中かどうかを管理するフラグ
	bool bIsActive;

	// 自壊用のタイマーハンドル
	FTimerHandle DestroyTimerHandle;

};
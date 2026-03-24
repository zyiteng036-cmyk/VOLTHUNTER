//担当佐々木
// リスポーン地点更新用のActor
//担当
//伊藤直樹

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnPointUpdater.generated.h"

class UBoxComponent;
class AScannerActor;
class APlayerCharacter;
class USoundBase;

UCLASS()
class HIGHSPEEDACTIONGAME_API ARespawnPointUpdater : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARespawnPointUpdater();

	virtual void Tick(float DeltaTime) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//HP加算量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	float m_HPAdditionAmount;

	//プレイヤーが重なってるかどうか
	bool m_IsPlayerOverlapping = false;

	//重なったプレイヤー
	TWeakObjectPtr<APlayerCharacter> m_OverlappingPlayer;

	UPROPERTY(VisibleAnywhere, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> m_StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<UBoxComponent> m_PlayerDitectionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	FRotator m_RespawnRotation;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSubclassOf<AScannerActor> ScannerEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	USoundBase* ActivationSound;
};

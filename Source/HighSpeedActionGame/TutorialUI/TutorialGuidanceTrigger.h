// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialGuidanceTrigger.generated.h"

class UBoxComponent;
class UUserWidget;

UCLASS()
class HIGHSPEEDACTIONGAME_API ATutorialGuidanceTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATutorialGuidanceTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	//衝突判定用ボックス
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	UBoxComponent* TriggerBox;

	//表示するWidgetクラス
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	TSubclassOf<UUserWidget> TutorialWidgetClass;

private:
	//生成したWidgetのインスタンスを保持
	UPROPERTY()
	UUserWidget* m_TutorialWidgetInstance;
};

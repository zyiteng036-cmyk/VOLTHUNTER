// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../../Event/EventMode.h"
#include "EventDataBase.generated.h"

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UEventDataBase : public UDataAsset
{
	GENERATED_BODY()
public:
	UEventDataBase();

	virtual EventMode GetDataEventMode()const { return EventMode::None; }
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timer = 0.f;

public:
	EventMode DataEventMode;
};

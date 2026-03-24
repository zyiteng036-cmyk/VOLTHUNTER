// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "../DamageInfo.h"
#include "../DataAssets/AttackData.h"
#include "../AttackCollisionDetection/AttackCollisionParam.h"
#include "CharacterAttackAnimNotifyState.generated.h"


//衝突判定の動きのモード
UENUM(BlueprintType)
enum class AttackAnimNotifyStateMode : uint8
{
	LocalCoordinateReference,//ローカル座標参照
};

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UCharacterAttackAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackCollisionInfo")
	FAttackCollisionParam m_AttackCollisionParam;//攻撃判定パラメーター


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackData")
	TObjectPtr<UAttackData> m_AttackData;//攻撃のデータ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackCollisionInfo")
	AttackAnimNotifyStateMode AnimNotifyStateMode = AttackAnimNotifyStateMode::LocalCoordinateReference;
};

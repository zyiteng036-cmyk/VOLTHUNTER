// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillEffect_NotifyState.h"
#include "../../PlayerComponent/SkillComponent/ThunderTrailEffect.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void USkillEffect_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!EffectClass || !MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = MeshComp->GetOwner();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector SpawnLoc = FVector::ZeroVector;
		FRotator SpawnRot = FRotator::ZeroRotator;

		if (SocketName != NAME_None)
		{
			SpawnLoc = MeshComp->GetSocketLocation(SocketName);
			SpawnRot = MeshComp->GetSocketRotation(SocketName);
		}
		else
		{
			// 指定がなければ足元（コンポーネント原点）
			SpawnLoc = MeshComp->GetComponentLocation();
			SpawnRot = MeshComp->GetComponentRotation();
		}

		// オフセット（微調整）を加算
		SpawnLoc += LocationOffset;


		// 生成
		AActor* SpawnedActor = World->SpawnActor<AActor>(EffectClass, SpawnLoc, SpawnRot, SpawnParams);

		if (SpawnedActor)
		{
			SpawnedActor->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

			SpawnedActor->Tags.Add(TraceTag);
		}
	}
}

void USkillEffect_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;

	// アタッチされている子アクターから、自分が生成した雷を探す
	TArray<AActor*> AttachedActors;

	if (AActor* Owner = MeshComp->GetOwner())
	{
		Owner->GetAttachedActors(AttachedActors);
	}

	for (AActor* Child : AttachedActors)
	{
		if (Child && Child->ActorHasTag(TraceTag))
		{
			// 専用クラスにキャストして終了処理を呼ぶ
			if (AThunderTrailEffect* Thunder = Cast<AThunderTrailEffect>(Child))
			{
				Thunder->BeginFadeOut();
			}
			else
			{
				// 万が一違うクラスなら強制削除
				Child->Destroy();
			}
		}
	}
}

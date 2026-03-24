// Fill out your copyright notice in the Description page of Project Settings.


#include "PostProsess_NotifyState.h"
#include "../PostProsess.h"
#include "Engine/World.h"

void UPostProsess_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp)
	{
		if (UWorld* World = MeshComp->GetWorld())
		{
			//サブシステムを取得して有効化 
			if (UPostProsess* PostProsess = World->GetSubsystem<UPostProsess>())
			{
				PostProsess->SetPostProsessActive(true);
			}
		}
	}
}

void UPostProsess_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		if (UWorld* World = MeshComp->GetWorld())
		{
			//サブシステムを取得して無効化
			if (UPostProsess* PostProsess = World->GetSubsystem<UPostProsess>())
			{
				PostProsess->SetPostProsessActive(false);
			}
		}
	}

}

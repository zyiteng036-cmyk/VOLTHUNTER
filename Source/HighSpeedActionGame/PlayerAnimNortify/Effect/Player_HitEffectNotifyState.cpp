// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_HitEffectNotifyState.h"
#include "NiagaraFunctionLibrary.h"
#include "../../PlayerCharacter.h"

void UPlayer_HitEffectNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
    m_PrevHit = false;
}

void UPlayer_HitEffectNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp)return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return;

	// ƒqƒbƒg”»’è‚ª‚ ‚éê‡
	if (PlayerCharacter->GetIsHit() && !m_PrevHit)
	{
		m_PrevHit = true;

        if (m_AttachToMesh)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                HitNiagara,
                MeshComp,
                AttachSocketName,
                LocationOffset,
                RotationOffset,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }
        else
        {
            const FTransform SpawnTransform(
                RotationOffset,
                MeshComp->GetComponentLocation() + LocationOffset
            );

            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                MeshComp->GetWorld(),
                HitNiagara,
                MeshComp->GetComponentLocation() + LocationOffset,
                RotationOffset
            );
        }
    }

}
void UPlayer_HitEffectNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

	m_PrevHit = false;
}


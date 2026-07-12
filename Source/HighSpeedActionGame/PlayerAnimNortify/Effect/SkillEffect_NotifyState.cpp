#include "SkillEffect_NotifyState.h"
#include "../../PlayerComponent/Skill/ThunderTrailEffect.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

//スキルエフェクトActorを生成
void USkillEffect_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !EffectClass) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	AActor* OwnerActor = MeshComp->GetOwner();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerActor;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	//ソケット指定がある場合はソケットTransformを使用
	if (SocketName != NAME_None)
	{
		SpawnLocation = MeshComp->GetSocketLocation(SocketName);
		SpawnRotation = MeshComp->GetSocketRotation(SocketName);
	}
	else
	{
		//ソケット未指定時はMeshComponentのTransformを使用
		SpawnLocation = MeshComp->GetComponentLocation();
		SpawnRotation = MeshComp->GetComponentRotation();
	}

	SpawnLocation += LocationOffset;

	//指定位置へエフェクトActorを生成
	AActor* SpawnedActor = World->SpawnActor<AActor>(EffectClass, SpawnLocation, SpawnRotation, SpawnParameters);
	if (!SpawnedActor) return;

	//NotifyState中はMeshまたは指定ソケットへ追従させる
	SpawnedActor->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

	//NotifyEndで対象を識別できるようタグを設定
	SpawnedActor->Tags.Add(TraceTag);
}

//生成したスキルエフェクトを終了
void USkillEffect_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	TArray<AActor*> AttachedActors;
	OwnerActor->GetAttachedActors(AttachedActors);

	for (AActor* ChildActor : AttachedActors)
	{
		if (!ChildActor || !ChildActor->ActorHasTag(TraceTag)) continue;

		//ThunderTrailは即破棄せず専用フェード処理へ移行
		if (AThunderTrailEffect* ThunderTrailEffect = Cast<AThunderTrailEffect>(ChildActor))
		{
			ThunderTrailEffect->BeginFadeOut();
			continue;
		}

		//専用終了処理を持たないActorはその場で破棄
		ChildActor->Destroy();
	}
}
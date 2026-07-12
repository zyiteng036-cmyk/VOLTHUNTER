#include "HitNotifyState.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerEvasiveComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

//攻撃ヒット受付を開始
void UHitNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	bHitTriggered = false;
	ActiveCameraShake = nullptr;

	APlayerCharacter* PlayerCharacter = FindPlayerCharacter(MeshComp);
	if (!PlayerCharacter) return;

	UE_LOG(LogTemp, Warning, TEXT("[HitNotify] Begin HitWindow"));
}

//攻撃命中を確認して演出を再生
void UHitNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	//同じ判定区間ではヒット演出を一度だけ再生
	if (bHitTriggered) return;

	APlayerCharacter* PlayerCharacter = FindPlayerCharacter(MeshComp);
	if (!PlayerCharacter) return;

	//攻撃Collisionが命中するまで演出を待機
	if (!PlayerCharacter->GetIsHit()) return;

	bHitTriggered = true;

	UE_LOG(LogTemp, Warning, TEXT("[HitNotify] Hit Triggered"));

	//プレイヤー位置で命中サウンドを再生
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(PlayerCharacter, HitSound, PlayerCharacter->GetActorLocation());
	}

	SpawnHitEffect(MeshComp);
	StartHitCameraShake(PlayerCharacter);

	//攻撃命中後は回避入力を許可
	if (UPlayerEvasiveComponent* EvasiveComponent = PlayerCharacter->FindComponentByClass<UPlayerEvasiveComponent>())
	{
		EvasiveComponent->SetCanEvasive(true);
	}
}

//ヒット受付と再生中の演出を終了
void UHitNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	APlayerCharacter* PlayerCharacter = FindPlayerCharacter(MeshComp);
	if (!PlayerCharacter) return;

	//攻撃が命中しなかった場合は空振りサウンドを再生
	if (!bHitTriggered && NoHitSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitNotify] No Hit"));

		UGameplayStatics::PlaySoundAtLocation(PlayerCharacter, NoHitSound, PlayerCharacter->GetActorLocation());
	}

	StopHitCameraShake(PlayerCharacter);

	bHitTriggered = false;
	ActiveCameraShake = nullptr;

	//次の攻撃判定区間に備えてヒット状態を初期化
	PlayerCharacter->ClearAttackHitState();
}

//Mesh所有者からプレイヤーを取得
APlayerCharacter* UHitNotifyState::FindPlayerCharacter(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp) return nullptr;

	return Cast<APlayerCharacter>(MeshComp->GetOwner());
}

//ヒット時のNiagaraを生成
void UHitNotifyState::SpawnHitEffect(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp || !HitEffect) return;

	//Meshへ追従させる場合は指定ソケットへ取り付ける
	if (bAttachToMesh)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			HitEffect,
			MeshComp,
			AttachSocketName,
			EffectLocationOffset,
			EffectRotationOffset,
			EAttachLocation::KeepRelativeOffset,
			true
		);

		return;
	}

	//追従不要の場合は現在のMesh位置へ生成
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		MeshComp->GetWorld(),
		HitEffect,
		MeshComp->GetComponentLocation() + EffectLocationOffset,
		EffectRotationOffset
	);
}

//ヒット時のカメラシェイクを開始
void UHitNotifyState::StartHitCameraShake(APlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter || !CameraShakeClass) return;

	APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController());
	if (!PlayerController || !PlayerController->PlayerCameraManager) return;

	ActiveCameraShake = PlayerController->PlayerCameraManager->StartCameraShake(CameraShakeClass, CameraShakeScale);
}

//再生中のカメラシェイクを停止
void UHitNotifyState::StopHitCameraShake(APlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter || !ActiveCameraShake) return;

	APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController());
	if (!PlayerController || !PlayerController->PlayerCameraManager) return;

	PlayerController->PlayerCameraManager->StopCameraShake(ActiveCameraShake, false);
}
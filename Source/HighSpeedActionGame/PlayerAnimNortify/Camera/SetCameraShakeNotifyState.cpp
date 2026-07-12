#include "SetCameraShakeNotifyState.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../PlayerComponent/PlayerAttackComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

//演出状態を初期化
void USetCameraShakeNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	//NotifyStateごとの実行状態を初期化
	bTriggered = false;
	SpawnedEffect = nullptr;
	ActiveCameraShake = nullptr;
}

//溜め時間を確認して演出を開始
void USetCameraShakeNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	//演出はNotifyState中に一度だけ発生させる
	if (bTriggered || !MeshComp) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return;

	UPlayerAttackComponent* AttackComponent = PlayerCharacter->FindComponentByClass<UPlayerAttackComponent>();
	if (!AttackComponent) return;

	//必要な溜め時間に達するまで待機
	if (AttackComponent->GetCurrentHeavyChargeTime() < RequiredChargeTime)
	{
		return;
	}

	//プレイヤーカメラへシェイクを開始
	if (APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController()))
	{
		if (PlayerController->PlayerCameraManager && CameraShakeClass)
		{
			ActiveCameraShake = PlayerController->PlayerCameraManager->StartCameraShake(CameraShakeClass, ShakeScale);
		}
	}

	//ソケット指定がある場合はソケット位置へ生成
	if (Effect)
	{
		const FVector SpawnLocation = EffectSocketName != NAME_None ? MeshComp->GetSocketLocation(EffectSocketName) : PlayerCharacter->GetActorLocation();

		SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MeshComp->GetWorld(), Effect, SpawnLocation + EffectOffset);
	}

	bTriggered = true;
}

//再生中の演出を終了
void USetCameraShakeNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	//生成したNiagaraを停止して破棄
	if (SpawnedEffect)
	{
		SpawnedEffect->Deactivate();
		SpawnedEffect->DestroyComponent();
		SpawnedEffect = nullptr;
	}

	//NotifyState中に開始したカメラシェイクを停止
	if (ActiveCameraShake && MeshComp)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController()))
			{
				if (PlayerController->PlayerCameraManager)
				{
					PlayerController->PlayerCameraManager->StopCameraShake(ActiveCameraShake, false);
				}
			}
		}
	}

	bTriggered = false;
	ActiveCameraShake = nullptr;
}
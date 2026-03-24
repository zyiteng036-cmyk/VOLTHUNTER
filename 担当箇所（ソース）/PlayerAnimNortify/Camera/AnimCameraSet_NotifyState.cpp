// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimCameraSet_NotifyState.h"
#include "../../PlayerCharacter.h"
#include "../../PlayerComponent/Player_CameraComponent.h"

void UAnimCameraSet_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (!MeshComp || !MeshComp->GetOwner()) return;

	//プレイヤーを取得カメラコンポーネントを探す
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		if (UPlayer_CameraComponent* CameraComp = PlayerCharacter->FindComponentByClass<UPlayer_CameraComponent>())
		{
			//値を渡してオーバーライド
			//ArmLength,FOVを使わない場合無視
			float SendArmLength = bEnableArmLength ? TargetArmLength : -1.0f;
			float SendFOV = bEnableFOV ? TargetFOV : -1.0f;

			CameraComp->StartAnimCameraOverride(SendArmLength, SendFOV, InterpSpeed);
		}
	}
}

void UAnimCameraSet_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp || !MeshComp->GetOwner()) return;

	//プレイヤーを取得カメラコンポーネントを探す
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		if (UPlayer_CameraComponent* CameraComp = PlayerCharacter->FindComponentByClass<UPlayer_CameraComponent>())
		{
			CameraComp->EndAnimCameraOverride();
		}
	}

}

//担当
//伊藤直樹

//-----------------------------------------------------
//強攻撃溜め演出NotifyState
//
//必要な溜め時間を満たした時にカメラシェイクとNiagaraを再生する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SetCameraShakeNotifyState.generated.h"

class UCameraShakeBase;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class HIGHSPEEDACTIONGAME_API USetCameraShakeNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//演出状態を初期化
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//溜め時間を確認して演出を開始
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	//再生中の演出を終了
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	//-----------------------------------------------------
	//カメラシェイク
	//-----------------------------------------------------

	//発生させるカメラシェイク
	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;

	//カメラシェイク倍率
	UPROPERTY(EditAnywhere, Category = "CameraShake", meta = (ClampMin = "0.0"))
	float ShakeScale = 1.f;

	//演出を開始する溜め時間
	UPROPERTY(EditAnywhere, Category = "CameraShake", meta = (ClampMin = "0.0", Units = "s"))
	float RequiredChargeTime = 1.f;

	//-----------------------------------------------------
	//エフェクト
	//-----------------------------------------------------

	//発生させるNiagaraエフェクト
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UNiagaraSystem> Effect = nullptr;

	//エフェクト発生ソケット
	UPROPERTY(EditAnywhere, Category = "Effect")
	FName EffectSocketName = NAME_None;

	//エフェクト位置オフセット
	UPROPERTY(EditAnywhere, Category = "Effect")
	FVector EffectOffset = FVector::ZeroVector;

private:
	//演出を開始済みか
	bool bTriggered = false;

	//生成したNiagaraComponent
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SpawnedEffect = nullptr;

	//再生中のカメラシェイク
	UPROPERTY()
	TObjectPtr<UCameraShakeBase> ActiveCameraShake = nullptr;
};
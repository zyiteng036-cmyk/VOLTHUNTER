//担当
//伊藤直樹

//-----------------------------------------------------
//攻撃ヒット演出NotifyState
//
//攻撃命中時のサウンド、Niagara、カメラシェイクを管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "HitNotifyState.generated.h"

class APlayerCharacter;
class UCameraShakeBase;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class HIGHSPEEDACTIONGAME_API UHitNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//攻撃ヒット受付を開始
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	//攻撃命中を確認して演出を再生
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	//ヒット受付と再生中の演出を終了
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	//Mesh所有者からプレイヤーを取得
	APlayerCharacter* FindPlayerCharacter(USkeletalMeshComponent* MeshComp) const;

	//ヒット時のNiagaraを生成
	void SpawnHitEffect(USkeletalMeshComponent* MeshComp) const;

	//ヒット時のカメラシェイクを開始
	void StartHitCameraShake(APlayerCharacter* PlayerCharacter);

	//再生中のカメラシェイクを停止
	void StopHitCameraShake(APlayerCharacter* PlayerCharacter);

protected:
	//-----------------------------------------------------
	//サウンド
	//-----------------------------------------------------

	//攻撃命中時のサウンド
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> HitSound = nullptr;

	//攻撃が命中しなかった時のサウンド
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> NoHitSound = nullptr;

	//-----------------------------------------------------
	//エフェクト
	//-----------------------------------------------------

	//攻撃命中時のNiagara
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UNiagaraSystem> HitEffect = nullptr;

	//NiagaraをMeshへ取り付けるか
	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bAttachToMesh = true;

	//Niagaraを取り付けるソケット
	UPROPERTY(EditAnywhere, Category = "Effect")
	FName AttachSocketName = NAME_None;

	//Niagaraの位置オフセット
	UPROPERTY(EditAnywhere, Category = "Effect")
	FVector EffectLocationOffset = FVector::ZeroVector;

	//Niagaraの回転オフセット
	UPROPERTY(EditAnywhere, Category = "Effect")
	FRotator EffectRotationOffset = FRotator::ZeroRotator;

	//-----------------------------------------------------
	//カメラ
	//-----------------------------------------------------

	//攻撃命中時のカメラシェイク
	UPROPERTY(EditAnywhere, Category = "Camera")
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;

	//カメラシェイク倍率
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "0.0"))
	float CameraShakeScale = 1.f;

private:
	//このNotifyState中にヒット演出を再生済みか
	bool bHitTriggered = false;

	//再生中のカメラシェイク
	UPROPERTY()
	TObjectPtr<UCameraShakeBase> ActiveCameraShake = nullptr;
};
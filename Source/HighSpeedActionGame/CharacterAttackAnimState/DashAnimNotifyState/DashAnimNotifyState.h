// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "DashAnimNotifyState.generated.h"

class UPlayerNotifySubsystem;
/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UDashAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float DeltaTime)override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)override;

private:
	//前方が壁か調べるメソッド
	bool IsWallInFront()const;
	//パラメーターリセット
	void ResetParam();

	UFUNCTION()
	void OnJustEvasiveOccurred(const AActor* Attacker);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashSpeed = 1200.f;//ダッシュ速度

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float m_WallJudgmentLength = 300.f;//壁判定の長さ

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	bool m_IsJustEvasion = false;

private:
	//使用しているキャラクター
	TWeakObjectPtr<ACharacter> CachedCharacter = nullptr;

	float m_SpeedBeforeSprint = 0.f;

	bool m_HasStopped = false;

	FVector m_OriginalVelocity = FVector::ZeroVector;

	const float m_StopTime = .3f;//ジャスト回避後に何秒で止まるか

	float m_StopTimer = 0.f;//ヒットストップ時間
};
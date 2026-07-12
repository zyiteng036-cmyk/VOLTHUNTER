//担当
//伊藤直樹

//-----------------------------------------------------
//ジャンプAbility
//
//ジャンプ開始と入力を離した時の終了処理を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "GA_Jump.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UGA_Jump : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_Jump();

protected:
	//ジャンプを開始
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	//入力を離した時にジャンプ処理を終了
	UFUNCTION()
	void OnJumpInputReleased(float TimeHeld);
};
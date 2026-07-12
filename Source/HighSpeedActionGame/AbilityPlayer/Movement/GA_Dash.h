//担当
//伊藤直樹

//-----------------------------------------------------
//ダッシュAbility
//
//ダッシュ開始、再入力停止、移動状態に連動した終了を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "../../PlayerComponent/Types/PlayerMovementTypes.h"
#include "GA_Dash.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UGA_Dash : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_Dash();

protected:
	//ダッシュを開始
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//再入力時にダッシュ停止を要求
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	//ダッシュAbilityを終了
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	//移動状態変更イベントを登録
	void BindMovementStateEvent();

	//移動状態変更イベントを解除
	void UnbindMovementStateEvent();

	//移動状態変更時にAbility終了を判定
	UFUNCTION()
	void OnMovementStateChanged(EPlayerMovementState PreviousState, EPlayerMovementState NewState);

private:
	//終了処理中か
	bool m_bIsEndingDashAbility = false;
};
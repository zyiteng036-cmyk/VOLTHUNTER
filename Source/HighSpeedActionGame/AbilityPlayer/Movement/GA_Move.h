//担当
//伊藤直樹

//-----------------------------------------------------
//移動Ability
//
//移動入力をCustomCharacterMovementComponentへ渡す常駐Ability
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "GA_Move.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UGA_Move : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_Move();

	//移動入力を更新
	void UpdateMoveInput(const FVector2D& InputVector);

	//移動入力をクリア
	void ClearMoveInput();

protected:
	//常駐移動Abilityを開始
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//Ability終了時に移動入力をクリア
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	//移動入力を停止する状態か判定
	bool IsMoveInputBlocked() const;
};
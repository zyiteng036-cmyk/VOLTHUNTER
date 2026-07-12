//担当
//伊藤直樹

//-----------------------------------------------------
//弱攻撃入力Ability
//
//弱攻撃入力をAttackComponentへGameplayTagとして渡す
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "GameplayTagContainer.h"
#include "GA_AttackInput_Light.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UGA_AttackInput_Light : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_AttackInput_Light();

protected:
	//弱攻撃入力を処理
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	//弱攻撃入力タグ
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Input")
	FGameplayTag m_LightInputTag = FGameplayTag();
};
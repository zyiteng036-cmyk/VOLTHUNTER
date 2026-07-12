//担当
//伊藤直樹

//-----------------------------------------------------
//強攻撃入力Ability
//
//押下と離し入力をAttackComponentへGameplayTagとして渡す
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "GameplayTagContainer.h"
#include "GA_AttackInput_Heavy.generated.h"

class UPlayerAttackComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API UGA_AttackInput_Heavy : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_AttackInput_Heavy();

protected:
	//強攻撃押下入力を処理
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	//強攻撃離し入力を処理
	UFUNCTION()
	void OnHeavyInputReleased(float TimeHeld);

private:
	//攻撃コンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerAttackComponent> m_AttackComponent = nullptr;

	//強攻撃押下タグ
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Input")
	FGameplayTag m_HeavyPressedInputTag = FGameplayTag();

	//強攻撃離しタグ
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Input")
	FGameplayTag m_HeavyReleasedInputTag = FGameplayTag();

	//離し入力を処理済みか
	bool m_bReleased = false;
};
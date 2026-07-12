//担当
//伊藤直樹

//-----------------------------------------------------
//スキル入力Ability
//
//選択中スキルの発動要求をPlayerSkillComponentへ渡す
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "GA_SkillInput.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UGA_SkillInput : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_SkillInput();

protected:
	//選択中スキルの発動を要求
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	//スキル入力処理を実行
	bool ExecuteSkillInput();
};
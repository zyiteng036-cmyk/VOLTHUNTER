//担当
//伊藤直樹

//-----------------------------------------------------
//スキルAbility基底クラス
//
//スキル開始、Montage再生、終了通知の共通処理を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "GameplayTagContainer.h"
#include "../../PlayerComponent/Types/PlayerSkillTypes.h"
#include "GA_SkillBase.generated.h"

class UAnimMontage;
class UPlayerSkillComponent;

UCLASS(Abstract)
class HIGHSPEEDACTIONGAME_API UGA_SkillBase : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_SkillBase();

protected:
	//スキルAbilityを開始
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//スキルAbilityを終了
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	//Montage正常終了時にAbilityを終了
	UFUNCTION()
	void OnSkillMontageCompleted();

	//Montageブレンドアウト時にAbilityを終了
	UFUNCTION()
	void OnSkillMontageBlendOut();

	//Montage中断時にAbilityを終了
	UFUNCTION()
	void OnSkillMontageInterrupted();

	//Montageキャンセル時にAbilityを終了
	UFUNCTION()
	void OnSkillMontageCancelled();

	//スキルAbilityの終了を要求
	void FinishSkillAbility(bool bWasCancelled);

protected:
	//スキルタグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTag m_SkillTag = FGameplayTag();

	//スキル種別
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	EPlayerSkillType m_SkillType = EPlayerSkillType::None;

	//スキルMontage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Animation")
	TObjectPtr<UAnimMontage> m_SkillMontage = nullptr;

	//Montage再生速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Animation")
	float m_PlayRate = 1.f;

	//Montage開始セクション
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Animation")
	FName m_StartSectionName = NAME_None;

private:
	//スキルコンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerSkillComponent> m_SkillComponent = nullptr;

	//Ability終了処理済みか
	bool m_bSkillAbilityEnded = false;
};
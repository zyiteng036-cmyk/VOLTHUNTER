//担当
//伊藤直樹

//-----------------------------------------------------
//回避Ability
//
//回避開始、終了、方向に応じたMontage再生を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "../../PlayerComponent/Types/PlayerMovementTypes.h"
#include "GA_Evasive.generated.h"

class UAnimMontage;
class UPlayerEvasiveComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API UGA_Evasive : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_Evasive();

protected:
	//回避を開始
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//回避Abilityを終了
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	//方向に応じた回避Montageを再生
	void PlayEvasiveMontage();

	//回避状態を終了
	void FinishEvasiveState();

	//回避時間終了時にAbilityを終了
	UFUNCTION()
	void OnEvasiveFinished();

private:
	//移動パラメータ未設定時の回避時間
	static constexpr float DefaultEvasiveDuration = 0.4f;

	//入力あり回避Montage
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Evasive")
	TObjectPtr<UAnimMontage> m_EvasiveMontage = nullptr;

	//入力なしバック回避Montage
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Evasive")
	TObjectPtr<UAnimMontage> m_BackEvasiveMontage = nullptr;

	//回避コンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerEvasiveComponent> m_EvasiveComponent = nullptr;

	//回避終了済みか
	bool m_bEvasiveEnded = false;
};
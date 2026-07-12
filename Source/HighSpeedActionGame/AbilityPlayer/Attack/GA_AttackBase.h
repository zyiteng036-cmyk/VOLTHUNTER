//担当
//伊藤直樹

//-----------------------------------------------------
//攻撃Ability基底クラス
//
//攻撃状態、Montage、空中攻撃移動、終了処理を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../GA_PlayerBase.h"
#include "GameplayTagContainer.h"
#include "../../PlayerComponent/Types/PlayerActionTypes.h"
#include "../../PlayerComponent/Types/PlayerAttackTypes.h"
#include "GA_AttackBase.generated.h"

class UAnimMontage;
class UPlayerAttackComponent;

UCLASS(Abstract)
class HIGHSPEEDACTIONGAME_API UGA_AttackBase : public UGA_PlayerBase
{
	GENERATED_BODY()

public:
	//初期化
	UGA_AttackBase();

protected:
	//攻撃Abilityを開始
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//攻撃Abilityを終了
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	//Montage正常終了時にAbilityを終了
	UFUNCTION()
	void OnAttackMontageCompleted();

	//Montageブレンドアウト開始を受け取る
	UFUNCTION()
	void OnAttackMontageBlendOut();

	//Montage中断時にAbilityを終了
	UFUNCTION()
	void OnAttackMontageInterrupted();

	//Montageキャンセル時にAbilityを終了
	UFUNCTION()
	void OnAttackMontageCancelled();

	//攻撃Abilityの終了を要求
	void FinishAttackAbility(bool bWasCancelled);

	//攻撃種別に応じた移動処理を開始
	void BeginAttackMotionByType();

	//攻撃種別に応じた移動処理を終了
	void EndAttackMotionByType();

	//ジャスト回避カウンター用ポストプロセスを切り替える
	void SetJustEvasivePostProcessActive(bool bActive);

protected:
	//このAbilityが表す攻撃タグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FGameplayTag m_AttackTag = FGameplayTag();

	//プレイヤー全体のアクション種別
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	EPlayerActionType m_ActionType = EPlayerActionType::Attack;

	//攻撃の細かい種別
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	EPlayerAttackType m_AttackType = EPlayerAttackType::None;

	//攻撃Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Animation")
	TObjectPtr<UAnimMontage> m_AttackMontage = nullptr;

	//Montage再生速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Animation")
	float m_PlayRate = 1.f;

	//Montage開始セクション
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Animation")
	FName m_StartSectionName = NAME_None;

private:
	//攻撃コンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerAttackComponent> m_AttackComponent = nullptr;

	//Ability終了処理済みか
	bool m_bAttackAbilityEnded = false;

	//ジャスト回避カウンターの一発目か
	bool m_bIsJustEvasiveCounterFirstAttack = false;
};
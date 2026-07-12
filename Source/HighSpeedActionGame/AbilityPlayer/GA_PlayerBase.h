//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー用GameplayAbility基底クラス
//
//PlayerCharacter、ASC、移動Componentの共通参照を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlayerBase.generated.h"

class APlayerCharacter;
class UAbilitySystemComponent;
class UCustomCharacterMovementComponent;

UCLASS(Abstract)
class HIGHSPEEDACTIONGAME_API UGA_PlayerBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	//初期化
	UGA_PlayerBase();

protected:
	//Ability開始時に共通参照を取得
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//共通参照を取得
	bool CachePlayerReferences(const FGameplayAbilityActorInfo* ActorInfo);

	//現在のAbilityを終了
	void EndPlayerAbility(bool bWasCancelled = false);

protected:
	//プレイヤー
	UPROPERTY()
	TObjectPtr<APlayerCharacter> m_PlayerCharacter = nullptr;

	//AbilitySystemComponent
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> m_AbilitySystemComponent = nullptr;

	//カスタム移動コンポーネント
	UPROPERTY()
	TObjectPtr<UCustomCharacterMovementComponent> m_CustomMovementComponent = nullptr;
};
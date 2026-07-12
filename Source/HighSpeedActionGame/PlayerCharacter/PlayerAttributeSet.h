//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤーAttributeSet
//
//GASで使用するHPと電力ゲージのAttributeを管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PlayerAttributeSet.generated.h"

struct FGameplayEffectModCallbackData;

#define PLAYER_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class HIGHSPEEDACTIONGAME_API UPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	//Attributeの初期値を設定
	UPlayerAttributeSet();

	//Attributeへ値が設定される前に範囲を補正
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//GameplayEffect適用後に現在値を有効範囲へ補正
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

public:
	//現在HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute|Health")
	FGameplayAttributeData Health;
	PLAYER_ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Health)

		//最大HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute|Health")
	FGameplayAttributeData MaxHealth;
	PLAYER_ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxHealth)

		//現在の電力ゲージ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute|Gauge")
	FGameplayAttributeData ElectroGauge;
	PLAYER_ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, ElectroGauge)

		//最大電力ゲージ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute|Gauge")
	FGameplayAttributeData MaxElectroGauge;
	PLAYER_ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxElectroGauge)

private:
	//初期最大HP
	static constexpr float DefaultMaxHealth = 100.f;

	//初期最大電力ゲージ
	static constexpr float DefaultMaxElectroGauge = 100.f;

	//最大値として許可する最低値
	static constexpr float MinimumMaximumAttributeValue = 1.f;
};

#undef PLAYER_ATTRIBUTE_ACCESSORS
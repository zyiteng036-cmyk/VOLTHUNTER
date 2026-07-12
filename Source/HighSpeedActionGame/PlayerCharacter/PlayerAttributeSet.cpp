#include "PlayerAttributeSet.h"
#include "GameplayEffectExtension.h"

//Attributeの初期値を設定
UPlayerAttributeSet::UPlayerAttributeSet()
{
	//HPは最大値まで回復した状態で初期化
	InitMaxHealth(DefaultMaxHealth);
	InitHealth(DefaultMaxHealth);

	//電力ゲージは空の状態で初期化
	InitMaxElectroGauge(DefaultMaxElectroGauge);
	InitElectroGauge(0.f);
}

//Attributeへ値が設定される前に範囲を補正
void UPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	//現在HPを0から最大HPの範囲へ制限
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
		return;
	}

	//最大HPが0以下にならないように制限
	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, MinimumMaximumAttributeValue);
		return;
	}

	//現在ゲージを0から最大ゲージの範囲へ制限
	if (Attribute == GetElectroGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxElectroGauge());
		return;
	}

	//最大ゲージが0以下にならないように制限
	if (Attribute == GetMaxElectroGaugeAttribute())
	{
		NewValue = FMath::Max(NewValue, MinimumMaximumAttributeValue);
	}
}

//GameplayEffect適用後に現在値を有効範囲へ補正
void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute ModifiedAttribute = Data.EvaluatedData.Attribute;

	//GameplayEffect適用後のHPを有効範囲へ戻す
	if (ModifiedAttribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		return;
	}

	//GameplayEffect適用後の電力ゲージを有効範囲へ戻す
	if (ModifiedAttribute == GetElectroGaugeAttribute())
	{
		SetElectroGauge(FMath::Clamp(GetElectroGauge(), 0.f, GetMaxElectroGauge()));
	}
}
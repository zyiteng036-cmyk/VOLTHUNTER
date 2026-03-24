//担当
//伊藤直樹

//プレイヤー強化ゲージクラス

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player_ElectroGaugeComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class APlayerCharacter;

UENUM(BlueprintType)
enum class EElectroState : uint8
{
	Normal			UMETA(DisplayName = "Normal"),
	Overcharge		UMETA(DisplayName = "Overcharge"),
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayer_ElectroGaugeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayer_ElectroGaugeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	//通常時の減少
	void _updateNormalStateDecay(float DeltaTime);
	//超電力の減少
	void _updateOverchargeStateDecay(float DeltaTime);

public:
	//セット
	void DebugOverCharge();

	//===Get関数===
	UFUNCTION(BlueprintPure, Category = "Electro")
	float GetGaugeRate()const;
	float GetCurrentGauge()const;
	UFUNCTION(BlueprintPure, Category = "ElectroGauge")
	bool IsOvercharge()const;
	EElectroState GetElectroState()const;

	//===外部通知===
	void AddElectroGauge(float Value);
	void SubtractionElectoroGauge(float Value);
	void OnJustEvasiveSuccess();

public:
	//リセット関数
	void ResetGauge();

private:
	void EnterOvercharge();
	void ExitOvercharge();

protected:
	APlayerCharacter* m_Player;


	//Overcharge時にエフェクト
	UPROPERTY(EditDefaultsOnly, Category = "Electro|Effect")
	UNiagaraSystem* m_OverchargeEffect;

	UPROPERTY()
	UNiagaraComponent* m_OverchargeEffectComp;

private:
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge")
	float m_MaxGauge;

	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge")
	float m_CurrentGauge;

	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge")
	float m_NormalDecayRate;

	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge")
	float m_OverChargeDecayRate;

	//最後にゲージが増えた時刻
	float m_LastAddGaugeTime;

	//減少開始までの猶予時間
	float m_DecayStartDelay;

	//減少加速
	float m_DecayAcceleration;

	//減少が始まってからの経過時間
	float m_DecayElapsedTime;

	//減少しているか
	bool m_IsDecaying;

	UPROPERTY(VisibleInstanceOnly, Category = "ElectroGauge")
	EElectroState m_ElectroState = EElectroState::Normal;
};

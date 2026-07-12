//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー電力ゲージコンポーネント
//
// AttributeSetのElectroGaugeを操作する窓口
// 通常時の自然減少、Overcharge状態、演出を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerElectroGaugeComponent.generated.h"

class APlayerCharacter;
class UPlayerAttributeSet;
class UNiagaraComponent;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EElectroState : uint8
{
	//通常状態
	Normal UMETA(DisplayName = "Normal"),

	//超帯電状態
	Overcharge UMETA(DisplayName = "Overcharge"),
};

//ゲージ変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnElectroGaugeChangedDelegate,
	float, CurrentGauge,
	float, MaxGauge
);

//状態変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnElectroStateChangedDelegate,
	EElectroState, PreviousState,
	EElectroState, NewState
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayerElectroGaugeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//初期化
	UPlayerElectroGaugeComponent();

protected:
	//開始時
	virtual void BeginPlay() override;

public:
	//毎フレーム更新
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

private:
	//通常時の減少
	void UpdateNormalStateDecay(float DeltaTime);

	//超帯電中の減少
	void UpdateOverchargeStateDecay(float DeltaTime);

public:
	//デバッグ用Overcharge
	UFUNCTION(BlueprintCallable, Category = "Electro")
	void DebugOverCharge();

	//デバッグ用Overcharge解除
	UFUNCTION(BlueprintCallable, Category = "Electro|Debug")
	void DebugExitOverCharge();

	//デバッグ用Overcharge切り替え
	UFUNCTION(BlueprintCallable, Category = "Electro|Debug")
	void DebugToggleOverCharge();

	//ゲージ割合取得
	UFUNCTION(BlueprintPure, Category = "Electro")
	float GetGaugeRate() const;

	//現在ゲージ取得
	UFUNCTION(BlueprintPure, Category = "Electro")
	float GetCurrentGauge() const;

	//最大ゲージ取得
	UFUNCTION(BlueprintPure, Category = "Electro")
	float GetMaxGauge() const;

	//Overcharge中か
	UFUNCTION(BlueprintPure, Category = "Electro")
	bool IsOvercharge() const;

	//現在状態取得
	UFUNCTION(BlueprintPure, Category = "Electro")
	EElectroState GetElectroState() const;

	//ゲージ加算
	UFUNCTION(BlueprintCallable, Category = "Electro")
	void AddElectroGauge(float Value);

	//ゲージ減少
	UFUNCTION(BlueprintCallable, Category = "Electro")
	void SubtractionElectoroGauge(float Value);

	//ゲージ減少
	UFUNCTION(BlueprintCallable, Category = "Electro")
	void SubtractElectroGauge(float Value);

	//ジャスト回避成功時
	UFUNCTION(BlueprintCallable, Category = "Electro")
	void OnJustEvasiveSuccess();

	//リセット
	UFUNCTION(BlueprintCallable, Category = "Electro")
	void ResetGauge();

private:
	//Overchargeへ入る
	void EnterOvercharge();

	//Overchargeを終了
	void ExitOvercharge();

	//状態変更
	void SetElectroState(EElectroState NewState);

	//AttributeSet取得
	UPlayerAttributeSet* GetPlayerAttributeSet() const;

	//ゲージ値設定
	void SetGaugeValue(float NewGauge);

	//最大ゲージ値設定
	void SetMaxGaugeValue(float NewMaxGauge);

	//ゲージ変更通知
	void BroadcastGaugeChanged();

public:
	//ゲージ変更通知
	UPROPERTY(BlueprintAssignable, Category = "Electro|Event")
	FOnElectroGaugeChangedDelegate OnElectroGaugeChanged;

	//状態変更通知
	UPROPERTY(BlueprintAssignable, Category = "Electro|Event")
	FOnElectroStateChangedDelegate OnElectroStateChanged;

protected:
	//プレイヤー
	UPROPERTY()
	TObjectPtr<APlayerCharacter> m_Player = nullptr;

	//Overcharge時のエフェクト
	UPROPERTY(EditDefaultsOnly, Category = "Electro|Effect")
	TObjectPtr<UNiagaraSystem> m_OverchargeEffect = nullptr;

	//Overcharge時のエフェクトインスタンス
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> m_OverchargeEffectComp = nullptr;

private:
	//最大ゲージ
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	float m_MaxGauge = 100.f;

	//通常時のベース減少速度
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	float m_NormalDecayRate = 1.f;

	//Overcharge中の消費倍率
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge|Overcharge", meta = (AllowPrivateAccess = "true"))
	float m_OverChargeConsumeRate = 0.35f;

	//Overcharge中の減少速度
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	float m_OverChargeDecayRate = 6.f;

	//減少開始までの猶予時間
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	float m_DecayStartDelay = 4.f;

	//減少加速
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	float m_DecayAcceleration = 3.f;

	//最後にゲージが増えてからの経過時間
	float m_LastAddGaugeTime = 0.f;

	//減少が始まってからの経過時間
	float m_DecayElapsedTime = 0.f;

	//減少しているか
	bool m_bIsDecaying = false;

	//現在の電力状態
	UPROPERTY(VisibleInstanceOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	EElectroState m_ElectroState = EElectroState::Normal;
};
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
	Normal      UMETA(DisplayName = "Normal"),
	Overcharge  UMETA(DisplayName = "Overcharge"),
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayer_ElectroGaugeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//コンストラクタ
	UPlayer_ElectroGaugeComponent();

protected:
	//ゲーム開始時に呼ばれる処理
	virtual void BeginPlay() override;

public:
	//毎フレーム呼ばれる処理
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	//通常時の減少
	void _updateNormalStateDecay(float DeltaTime);

	//超電力の減少
	void _updateOverchargeStateDecay(float DeltaTime);

public:
	//===Get関数===
	//ゲージの割合を取得
	UFUNCTION(BlueprintPure, Category = "ElectroGauge")
	float GetGaugeRate() const;

	//現在のゲージ量を取得
	UFUNCTION(BlueprintPure, Category = "ElectroGauge")
	float GetCurrentGauge() const;

	//オーバーチャージ中か判定
	UFUNCTION(BlueprintPure, Category = "ElectroGauge")
	bool IsOvercharge() const;

	//現在の電力量ステートを取得
	UFUNCTION(BlueprintPure, Category = "ElectroGauge")
	EElectroState GetElectroState() const;

	//===外部通知===
	//ゲージを加算する
	void AddElectroGauge(float Value);

	//ゲージを減算する
	void SubtractionElectoroGauge(float Value);

	//ジャスト回避成功時の処理
	void OnJustEvasiveSuccess();

	//リセット関数
	void ResetGauge();

private:
	//オーバーチャージ状態へ突入
	void EnterOvercharge();

	//オーバーチャージ状態から退出
	void ExitOvercharge();

protected:
	//プレイヤーの参照
	UPROPERTY()
	APlayerCharacter* m_Player = nullptr;

	//オーバーチャージ時に再生するエフェクト
	UPROPERTY(EditDefaultsOnly, Category = "Electro|Effect")
	UNiagaraSystem* m_OverchargeEffect = nullptr;

	//生成されたNiagaraComponent
	UPROPERTY()
	UNiagaraComponent* m_OverchargeEffectComp = nullptr;

private:
	//===パラメータ===

	//ゲージの最大値
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	float m_MaxGauge = 100.0f;

	//現在のゲージ量
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	float m_CurrentGauge = 0.0f;

	//通常時のベース減少速度
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	float m_NormalDecayRate = 1.0f;

	//オーバーチャージ中の減少速度
	UPROPERTY(EditDefaultsOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	float m_OverChargeDecayRate = 6.0f;

	//最後にゲージが増えた時刻
	float m_LastAddGaugeTime = 0.0f;

	//減少開始までの猶予時間
	float m_DecayStartDelay = 4.0f;

	//減少加速
	float m_DecayAcceleration = 3.0f;

	//減少が始まってからの経過時間
	float m_DecayElapsedTime = 0.0f;

	//減少しているか
	bool m_IsDecaying = false;

	//現在の電力量ステート
	UPROPERTY(VisibleInstanceOnly, Category = "ElectroGauge", meta = (AllowPrivateAccess = "true"))
	EElectroState m_ElectroState = EElectroState::Normal;
};
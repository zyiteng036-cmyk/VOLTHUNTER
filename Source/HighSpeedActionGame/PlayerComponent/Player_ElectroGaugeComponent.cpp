#include "Player_ElectroGaugeComponent.h"
#include "../PlayerCharacter.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

//===マジックナンバー排除用の定数===
namespace ElectroConstants
{
	//超帯電中のダメージ等によるゲージ減少カット率(30%カット)
	constexpr float OverchargeDamageReductionMultiplier = 0.7f;
	//エフェクト生成時のZ軸オフセット位置
	constexpr float OverchargeEffectOffsetZ = -100.0f;
}

UPlayer_ElectroGaugeComponent::UPlayer_ElectroGaugeComponent()
{
	//毎フレームのTick処理を有効化する設定
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayer_ElectroGaugeComponent::BeginPlay()
{
	//親クラスのBeginPlayを呼び出し
	Super::BeginPlay();

	//現在のゲージ量を初期値の0にリセット
	m_CurrentGauge = 0.0f;
	//ステートを通常状態に初期化
	m_ElectroState = EElectroState::Normal;

	//オーナーアクターをプレイヤーキャラクターとしてキャストして取得
	m_Player = Cast<APlayerCharacter>(GetOwner());
	//プレイヤーが正しく取得できなかった場合は以降の処理を中断
	if (!m_Player)
	{
		return;
	}
}

void UPlayer_ElectroGaugeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//親クラスのTickComponentを呼び出し
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//現在の電力量ステートに応じて更新処理を分岐
	switch (m_ElectroState)
	{
		//通常状態の場合
	case EElectroState::Normal:
		//通常時のゲージ減少処理を実行
		_updateNormalStateDecay(DeltaTime);
		break;

		//オーバーチャージ状態の場合
	case EElectroState::Overcharge:
		//オーバーチャージ中のゲージ減少処理を実行
		_updateOverchargeStateDecay(DeltaTime);
		break;
	}
}

void UPlayer_ElectroGaugeComponent::_updateNormalStateDecay(float DeltaTime)
{
	//現在のゲージが0以下か判定
	if (m_CurrentGauge <= 0.0f)
	{
		//ゲージが空なので減少フラグを解除
		m_IsDecaying = false;
		//減少経過時間をリセット
		m_DecayElapsedTime = 0.0f;
		//最後にゲージが追加された時刻をリセット
		m_LastAddGaugeTime = 0.0f;
		//これ以上の処理は不要なため終了
		return;
	}

	//現在ゲージが減少していない状態か判定
	if (!m_IsDecaying)
	{
		//最後にゲージが増加してからの経過時間を加算
		m_LastAddGaugeTime += DeltaTime;

		//経過時間が減少開始の猶予時間を超えたか判定
		if (m_LastAddGaugeTime >= m_DecayStartDelay)
		{
			//猶予時間を超えたため減少フラグを有効化
			m_IsDecaying = true;
		}
	}

	//ゲージ減少フラグが有効な場合のみ実行
	if (m_IsDecaying)
	{
		//減少が開始してからの経過時間を加算
		m_DecayElapsedTime += DeltaTime;
		//基本減少速度に経過時間分の加速値を加えて現在の減少速度を算出
		const float AcceleratedDecayRate = m_NormalDecayRate + (m_DecayElapsedTime * m_DecayAcceleration);
		//算出した減少速度を用いてゲージを減算し0から最大値の範囲に収める
		m_CurrentGauge = FMath::Clamp(m_CurrentGauge - (AcceleratedDecayRate * DeltaTime), 0.0f, m_MaxGauge);
	}
}

void UPlayer_ElectroGaugeComponent::_updateOverchargeStateDecay(float DeltaTime)
{
	//オーバーチャージ専用の減少速度でゲージを減算
	m_CurrentGauge -= m_OverChargeDecayRate * DeltaTime;

	//ゲージが0以下になったか判定
	if (m_CurrentGauge <= 0.0f)
	{
		//ゲージが尽きたためオーバーチャージ状態から退出
		ExitOvercharge();
	}
}

float UPlayer_ElectroGaugeComponent::GetGaugeRate() const
{
	//最大値が0以下の場合は0除算を防ぐため即座に0を返す
	if (m_MaxGauge <= 0.0f)
	{
		return 0.0f;
	}
	//現在のゲージ量を最大値で割り割合を算出して返す
	return m_CurrentGauge / m_MaxGauge;
}

float UPlayer_ElectroGaugeComponent::GetCurrentGauge() const
{
	//現在のゲージ量をそのまま返す
	return m_CurrentGauge;
}

bool UPlayer_ElectroGaugeComponent::IsOvercharge() const
{
	//現在のステートがオーバーチャージ状態と一致するかを判定して返す
	return m_ElectroState == EElectroState::Overcharge;
}

EElectroState UPlayer_ElectroGaugeComponent::GetElectroState() const
{
	//現在の電力量ステートを返す
	return m_ElectroState;
}

void UPlayer_ElectroGaugeComponent::AddElectroGauge(float Value)
{
	//現在がオーバーチャージ状態か判定
	if (m_ElectroState == EElectroState::Overcharge)
	{
		//オーバーチャージ中はゲージの加算を行わないため処理を終了
		return;
	}

	//受け取った値を現在のゲージに加算し0から最大値の範囲に収める
	m_CurrentGauge = FMath::Clamp(m_CurrentGauge + Value, 0.0f, m_MaxGauge);

	//ゲージが増加したため増加後からの経過時間をリセット
	m_LastAddGaugeTime = 0.0f;
	//ゲージ減少中の経過時間をリセット
	m_DecayElapsedTime = 0.0f;
	//ゲージの減少フラグを解除し減少を止める
	m_IsDecaying = false;
}

void UPlayer_ElectroGaugeComponent::SubtractionElectoroGauge(float Value)
{
	//現在がオーバーチャージ状態か判定
	if (m_ElectroState == EElectroState::Overcharge)
	{
		//オーバーチャージ中はダメージ等によるゲージ減少量を軽減率に応じてカットする
		Value *= ElectroConstants::OverchargeDamageReductionMultiplier;
	}

	//計算された減少値を現在のゲージから引き0から最大値の範囲に収める
	m_CurrentGauge = FMath::Clamp(m_CurrentGauge - Value, 0.0f, m_MaxGauge);
}

void UPlayer_ElectroGaugeComponent::OnJustEvasiveSuccess()
{
	//現在が通常状態ではない(オーバーチャージ中など)か判定
	if (m_ElectroState != EElectroState::Normal)
	{
		//通常状態以外では処理を行わないため終了
		return;
	}

	//現在のゲージが最大値に達しているか判定
	if (m_CurrentGauge >= m_MaxGauge)
	{
		//最大値に達しているためオーバーチャージ状態に突入
		EnterOvercharge();
	}
}

void UPlayer_ElectroGaugeComponent::EnterOvercharge()
{
	//既にオーバーチャージ状態か判定
	if (m_ElectroState == EElectroState::Overcharge)
	{
		//既に状態が移行しているため処理を終了
		return;
	}

	//ステートをオーバーチャージ状態に変更
	m_ElectroState = EElectroState::Overcharge;

	//再生するエフェクトが未設定またはオーナーが存在しないか判定
	if (!m_OverchargeEffect || !GetOwner())
	{
		//エフェクトを再生できないため処理を終了
		return;
	}

	//エフェクトコンポーネントがまだ生成されていないか判定
	if (!m_OverchargeEffectComp)
	{
		//オーナーのルートコンポーネントにアタッチする形でNiagaraエフェクトを生成
		m_OverchargeEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			m_OverchargeEffect,
			GetOwner()->GetRootComponent(),
			NAME_None, //ソケットの指定はBP側で行うためなし
			FVector(0.0f, 0.0f, ElectroConstants::OverchargeEffectOffsetZ), //名前空間の定数からZ軸オフセット位置を適用
			FRotator::ZeroRotator, //回転はデフォルトのまま
			EAttachLocation::KeepRelativeOffset, //相対的なオフセットを維持してアタッチ
			false //自動破棄(AutoDestroy)を無効化
		);
	}
	else
	{
		//既に生成済みの場合はエフェクトを再度アクティブにして再生開始
		m_OverchargeEffectComp->Activate(true);
	}
}

void UPlayer_ElectroGaugeComponent::ExitOvercharge()
{
	//現在がオーバーチャージ状態ではないか判定
	if (m_ElectroState != EElectroState::Overcharge)
	{
		//退出する状態ではないため処理を終了
		return;
	}

	//ステートを通常状態に変更
	m_ElectroState = EElectroState::Normal;
	//オーバーチャージが終了したためゲージを0にリセット
	m_CurrentGauge = 0.0f;

	//エフェクトコンポーネントが存在するか判定
	if (m_OverchargeEffectComp)
	{
		//再生中のエフェクトを非アクティブにして停止
		m_OverchargeEffectComp->Deactivate();
	}
}

void UPlayer_ElectroGaugeComponent::ResetGauge()
{
	//現在のゲージ量を0にリセット
	m_CurrentGauge = 0.0f;
	//最後にゲージが追加された時刻をリセット
	m_LastAddGaugeTime = 0.0f;
	//減少経過時間をリセット
	m_DecayElapsedTime = 0.0f;
	//減少フラグを解除
	m_IsDecaying = false;

	//現在がオーバーチャージ状態か判定
	if (m_ElectroState == EElectroState::Overcharge)
	{
		//オーバーチャージ中の場合は適切な退出処理を実行
		ExitOvercharge();
	}
	else
	{
		//それ以外の場合は単純にステートを通常状態に戻す
		m_ElectroState = EElectroState::Normal;
	}
}
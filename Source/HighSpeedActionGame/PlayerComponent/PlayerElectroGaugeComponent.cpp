#include "PlayerElectroGaugeComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../PlayerCharacter/PlayerAttributeSet.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

//初期化
UPlayerElectroGaugeComponent::UPlayerElectroGaugeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

//開始時
void UPlayerElectroGaugeComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player) return;

	SetMaxGaugeValue(m_MaxGauge);
	SetGaugeValue(0.f);

	SetElectroState(EElectroState::Normal);

	m_LastAddGaugeTime = 0.f;
	m_DecayElapsedTime = 0.f;
	m_bIsDecaying = false;
}

//毎フレーム更新
void UPlayerElectroGaugeComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (m_ElectroState)
	{
	case EElectroState::Normal:
		UpdateNormalStateDecay(DeltaTime);
		break;

	case EElectroState::Overcharge:
		UpdateOverchargeStateDecay(DeltaTime);
		break;

	default:
		break;
	}
}

//通常時の減少
void UPlayerElectroGaugeComponent::UpdateNormalStateDecay(float DeltaTime)
{
	const float CurrentGauge = GetCurrentGauge();

	if (CurrentGauge <= 0.f)
	{
		m_bIsDecaying = false;
		m_DecayElapsedTime = 0.f;
		m_LastAddGaugeTime = 0.f;
		return;
	}

	if (!m_bIsDecaying)
	{
		m_LastAddGaugeTime += DeltaTime;

		if (m_LastAddGaugeTime >= m_DecayStartDelay)
		{
			m_bIsDecaying = true;
		}
	}

	if (!m_bIsDecaying) return;

	m_DecayElapsedTime += DeltaTime;

	const float AcceleratedDecayRate =
		m_NormalDecayRate + m_DecayElapsedTime * m_DecayAcceleration;

	SetGaugeValue(CurrentGauge - AcceleratedDecayRate * DeltaTime);
}

//超帯電中の減少
void UPlayerElectroGaugeComponent::UpdateOverchargeStateDecay(float DeltaTime)
{
	const float NewGauge = GetCurrentGauge() - m_OverChargeDecayRate * DeltaTime;

	if (NewGauge <= 0.f)
	{
		ExitOvercharge();
		return;
	}

	SetGaugeValue(NewGauge);
}

//デバッグ用Overcharge
void UPlayerElectroGaugeComponent::DebugOverCharge()
{
	SetGaugeValue(GetMaxGauge());
	EnterOvercharge();
}

//デバッグ用Overcharge解除
void UPlayerElectroGaugeComponent::DebugExitOverCharge()
{
	ExitOvercharge();
}

//デバッグ用Overcharge切り替え
void UPlayerElectroGaugeComponent::DebugToggleOverCharge()
{
	if (IsOvercharge())
	{
		DebugExitOverCharge();
		return;
	}

	DebugOverCharge();
}

//ゲージ割合取得
float UPlayerElectroGaugeComponent::GetGaugeRate() const
{
	const float MaxGauge = GetMaxGauge();
	if (MaxGauge <= 0.f) return 0.f;

	return GetCurrentGauge() / MaxGauge;
}

//現在ゲージ取得
float UPlayerElectroGaugeComponent::GetCurrentGauge() const
{
	const UPlayerAttributeSet* AttributeSet = GetPlayerAttributeSet();
	if (!AttributeSet) return 0.f;

	return AttributeSet->GetElectroGauge();
}

//最大ゲージ取得
float UPlayerElectroGaugeComponent::GetMaxGauge() const
{
	const UPlayerAttributeSet* AttributeSet = GetPlayerAttributeSet();
	if (!AttributeSet) return m_MaxGauge;

	return AttributeSet->GetMaxElectroGauge();
}

//Overcharge中か
bool UPlayerElectroGaugeComponent::IsOvercharge() const
{
	return m_ElectroState == EElectroState::Overcharge;
}

//現在状態取得
EElectroState UPlayerElectroGaugeComponent::GetElectroState() const
{
	return m_ElectroState;
}

//ゲージ加算
void UPlayerElectroGaugeComponent::AddElectroGauge(float Value)
{
	if (m_ElectroState == EElectroState::Overcharge)
	{
		return;
	}

	SetGaugeValue(GetCurrentGauge() + Value);

	m_LastAddGaugeTime = 0.f;
	m_DecayElapsedTime = 0.f;
	m_bIsDecaying = false;
}

//ゲージ減少
void UPlayerElectroGaugeComponent::SubtractionElectoroGauge(float Value)
{
	SubtractElectroGauge(Value);
}

//ゲージ減少
void UPlayerElectroGaugeComponent::SubtractElectroGauge(float Value)
{
	float FinalValue = Value;

	if (m_ElectroState == EElectroState::Overcharge)
	{
		FinalValue *= m_OverChargeConsumeRate;
	}

	SetGaugeValue(GetCurrentGauge() - FinalValue);
}


//ジャスト回避成功時
void UPlayerElectroGaugeComponent::OnJustEvasiveSuccess()
{
	if (m_ElectroState != EElectroState::Normal)
	{
		return;
	}

	if (GetCurrentGauge() >= GetMaxGauge())
	{
		EnterOvercharge();
	}
}

//リセット
void UPlayerElectroGaugeComponent::ResetGauge()
{
	SetGaugeValue(0.f);

	m_LastAddGaugeTime = 0.f;
	m_DecayElapsedTime = 0.f;
	m_bIsDecaying = false;

	if (m_ElectroState == EElectroState::Overcharge)
	{
		ExitOvercharge();
	}
	else
	{
		SetElectroState(EElectroState::Normal);
	}
}

//Overchargeへ入る
void UPlayerElectroGaugeComponent::EnterOvercharge()
{
	if (m_ElectroState == EElectroState::Overcharge)
	{
		return;
	}

	SetElectroState(EElectroState::Overcharge);

	if (!m_OverchargeEffect || !GetOwner()) return;

	if (!m_OverchargeEffectComp)
	{
		m_OverchargeEffectComp =
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				m_OverchargeEffect,
				GetOwner()->GetRootComponent(),
				NAME_None,
				FVector(0.f, 0.f, -100.f),
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				false
			);
	}
	else
	{
		m_OverchargeEffectComp->Activate(true);
	}
}

//Overchargeを終了
void UPlayerElectroGaugeComponent::ExitOvercharge()
{
	if (m_ElectroState != EElectroState::Overcharge)
	{
		return;
	}

	SetElectroState(EElectroState::Normal);
	SetGaugeValue(0.f);

	if (m_OverchargeEffectComp)
	{
		m_OverchargeEffectComp->Deactivate();
	}
}

//状態変更
void UPlayerElectroGaugeComponent::SetElectroState(EElectroState NewState)
{
	if (m_ElectroState == NewState) return;

	const EElectroState PreviousState = m_ElectroState;
	m_ElectroState = NewState;

	OnElectroStateChanged.Broadcast(
		PreviousState,
		m_ElectroState
	);
}

//AttributeSet取得
UPlayerAttributeSet* UPlayerElectroGaugeComponent::GetPlayerAttributeSet() const
{
	if (!m_Player) return nullptr;

	return m_Player->GetPlayerAttributeSet();
}

//ゲージ値設定
void UPlayerElectroGaugeComponent::SetGaugeValue(float NewGauge)
{
	UPlayerAttributeSet* AttributeSet = GetPlayerAttributeSet();
	if (!AttributeSet) return;

	const float ClampedGauge = FMath::Clamp(NewGauge, 0.f, GetMaxGauge());
	AttributeSet->SetElectroGauge(ClampedGauge);

	BroadcastGaugeChanged();
}

//最大ゲージ値設定
void UPlayerElectroGaugeComponent::SetMaxGaugeValue(float NewMaxGauge)
{
	UPlayerAttributeSet* AttributeSet = GetPlayerAttributeSet();
	if (!AttributeSet) return;

	const float SafeMaxGauge = FMath::Max(NewMaxGauge, 1.f);
	AttributeSet->SetMaxElectroGauge(SafeMaxGauge);

	BroadcastGaugeChanged();
}

//ゲージ変更通知
void UPlayerElectroGaugeComponent::BroadcastGaugeChanged()
{
	OnElectroGaugeChanged.Broadcast(GetCurrentGauge(), GetMaxGauge());
}
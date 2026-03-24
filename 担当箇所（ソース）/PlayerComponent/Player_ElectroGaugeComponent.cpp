// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_ElectroGaugeComponent.h"
#include "../PlayerCharacter.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"


// Sets default values for this component's properties
UPlayer_ElectroGaugeComponent::UPlayer_ElectroGaugeComponent()
	:m_MaxGauge(100.f)
	, m_CurrentGauge(0.f)
	, m_NormalDecayRate(1.f)
	, m_OverChargeDecayRate(6.f)
	, m_LastAddGaugeTime(0.f)
	, m_DecayStartDelay(4.f)
	, m_DecayAcceleration(3.f)
	, m_DecayElapsedTime(0.f)
	, m_IsDecaying(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayer_ElectroGaugeComponent::BeginPlay()
{
	Super::BeginPlay();



	m_CurrentGauge = 0.f;
	m_ElectroState = EElectroState::Normal;

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player)		return;

}


// Called every frame
void UPlayer_ElectroGaugeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (m_ElectroState)
	{
		//===通常===
	case EElectroState::Normal:
	{
		_updateNormalStateDecay(DeltaTime);
	}
	break;

	//===超帯電中===
	case EElectroState::Overcharge:
	{
		_updateOverchargeStateDecay(DeltaTime);
	}
	break;
	}
	// ...

}

void UPlayer_ElectroGaugeComponent::_updateNormalStateDecay(float DeltaTime)
{
	//もしゲージが０なら何もしない
	if (m_CurrentGauge <= 0.f)
	{
		m_IsDecaying = false;
		m_DecayElapsedTime = 0.f;
		m_LastAddGaugeTime = 0.f;
		return;
	}

	if (!m_IsDecaying) {
		//最後のゲージ増加からの経過時間
		m_LastAddGaugeTime += DeltaTime;

	}

	//猶予時間超えたら減少開始
	if (m_LastAddGaugeTime >= m_DecayStartDelay)
	{
		m_IsDecaying = true;
	}
	//減少中のみ時間を進める

	if (m_IsDecaying)
	{

		m_DecayElapsedTime += DeltaTime;
		const float AcceleratedDacayRate = m_NormalDecayRate + m_DecayElapsedTime * m_DecayAcceleration;
		m_CurrentGauge = FMath::Clamp(m_CurrentGauge - AcceleratedDacayRate * DeltaTime, 0.f, m_MaxGauge);

	}

}

void UPlayer_ElectroGaugeComponent::_updateOverchargeStateDecay(float DeltaTime)
{
	m_CurrentGauge -= m_OverChargeDecayRate * DeltaTime;

	if (m_CurrentGauge <= 0.f)
	{
		ExitOvercharge();
	}

}

void UPlayer_ElectroGaugeComponent::DebugOverCharge()
{
	// ゲージが空だとTickですぐ終了してしまうため、最大まで埋める
	m_CurrentGauge = m_MaxGauge;

	// 既存のオーバーチャージ突入処理を呼ぶ
	// (ステート変更、Niagara再生などはここで行われる)
	EnterOvercharge();
}

float UPlayer_ElectroGaugeComponent::GetGaugeRate() const
{
	return m_CurrentGauge / m_MaxGauge;
}

float UPlayer_ElectroGaugeComponent::GetCurrentGauge() const
{
	return m_CurrentGauge;
}

bool UPlayer_ElectroGaugeComponent::IsOvercharge() const
{
	return m_ElectroState == EElectroState::Overcharge;
}

EElectroState UPlayer_ElectroGaugeComponent::GetElectroState() const
{
	return m_ElectroState;
}

void UPlayer_ElectroGaugeComponent::AddElectroGauge(float Value)
{
	//超帯電中は加算しない
	if (m_ElectroState == EElectroState::Overcharge)
	{
		return;
	}

	m_CurrentGauge = FMath::Clamp(m_CurrentGauge + Value, 0.f, m_MaxGauge);

	//リセット
	m_LastAddGaugeTime = 0.f;
	m_DecayElapsedTime = 0.f;
	m_IsDecaying = false;
}

void UPlayer_ElectroGaugeComponent::SubtractionElectoroGauge(float Value)
{
	//超帯電中は減算の軽減
	if (m_ElectroState == EElectroState::Overcharge)
	{
		Value *= 0.7f;
	}

	m_CurrentGauge = FMath::Clamp(m_CurrentGauge - Value, 0.f, m_MaxGauge);

}

void UPlayer_ElectroGaugeComponent::OnJustEvasiveSuccess()
{
	if (m_ElectroState != EElectroState::Normal)
	{
		return;
	}

	if (m_CurrentGauge >= m_MaxGauge)
	{
		EnterOvercharge();
	}
}


void UPlayer_ElectroGaugeComponent::EnterOvercharge()
{
	if (m_ElectroState == EElectroState::Overcharge)
	{
		return;
	}
	m_ElectroState = EElectroState::Overcharge;
	//m_Player->SetIsEnhancedAttack(true);

	if (!m_OverchargeEffect || !GetOwner())return;

	// === Niagara ON ===
	if (!m_OverchargeEffectComp)
	{
		m_OverchargeEffectComp =
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				m_OverchargeEffect,
				GetOwner()->GetRootComponent(),
				NAME_None,                // ソケットは BP 側
				FVector(0.f, 0.f, -100.f),
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				false                     // AutoDestroy = false
			);
	}
	else
	{
		m_OverchargeEffectComp->Activate(true);
	}
}

void UPlayer_ElectroGaugeComponent::ExitOvercharge()
{
	if (m_ElectroState != EElectroState::Overcharge)
	{
		return;
	}
	m_ElectroState = EElectroState::Normal;
	m_CurrentGauge = 0.f;

	//m_Player->SetIsEnhancedAttack(false);

	// === Niagara OFF ===
	if (m_OverchargeEffectComp)
	{
		m_OverchargeEffectComp->Deactivate();
	}
}

void UPlayer_ElectroGaugeComponent::ResetGauge()
{
	// 値を初期化
	m_CurrentGauge = 0.f;
	m_LastAddGaugeTime = 0.f;
	m_DecayElapsedTime = 0.f;
	m_IsDecaying = false;

	// ステートを通常に戻す
	if (m_ElectroState == EElectroState::Overcharge)
	{
		ExitOvercharge();
	}
	m_ElectroState = EElectroState::Normal;
}

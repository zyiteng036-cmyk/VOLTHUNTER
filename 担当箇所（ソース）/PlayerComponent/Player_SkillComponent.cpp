// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_SkillComponent.h"
#include "Components/CapsuleComponent.h"
#include "Player_MovementComponent.h"
#include "Player_AttackComponent.h"
#include "Player_CameraComponent.h"
#include "Player_ElectroGaugeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "../PlayerCharacter.h"

//#include "../PostProcess/PostProcess.h"

// Sets default values for this component's properties
UPlayer_SkillComponent::UPlayer_SkillComponent()
	:m_Player(nullptr)
	, m_MovementComponent(nullptr)
	, m_AttackComponent(nullptr)
	, m_ElectroGaugeComponent(nullptr)
	, m_CurrentAbilitySkill(nullptr)
	, m_CanSkillActive(true)
	, m_ThunderFlashCost(70.f)
	, m_Skill01Distance(600.f)
	, m_SelectedSkill(EPlayerSkill::ThunderFlash)
	, m_SelectLocked(false)
	, m_IsSkillActive(false)
	, m_StartLocation(FVector::ZeroVector)
	, m_TargetLocation(FVector::ZeroVector)
	, m_DelayElapsed(0.f)
	, m_DelayTime(0.4f)
	, m_Teleported(false)
	, m_bLastUsableState(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayer_SkillComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player)return;

	m_MovementComponent = m_Player->FindComponentByClass<UPlayer_MovementComponent>();
	if (!m_MovementComponent)return;

	m_AttackComponent = m_Player->FindComponentByClass<UPlayer_AttackComponent>();
	if (!m_AttackComponent)return;

	m_ElectroGaugeComponent = m_Player->FindComponentByClass<UPlayer_ElectroGaugeComponent>();
	if (!m_ElectroGaugeComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: ElectroGaugeComponent not found"));
	}

	if (OnSkillUsableChanged.IsBound())
	{
		OnSkillUsableChanged.Broadcast(m_bLastUsableState);
	}
}


// Called every frame
void UPlayer_SkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_IsSkillActive && m_SelectedSkill == EPlayerSkill::ThunderFlash)
	{
		_updateThunderFlash(DeltaTime);

	}

	if (m_IsSkillActive && !m_bHasShownHitUI && m_Player)
	{
		// プレイヤー側のヒットフラグを確認
		if (m_Player->GetIsHit())
		{
			// UIクラスが設定されているか確認
			if (SkillHitUIClass)
			{
				// UIを作成
				m_ActiveHitUIInstance = CreateWidget<UUserWidget>(GetWorld(), SkillHitUIClass);

				if (m_ActiveHitUIInstance)
				{
					// 画面に追加
					m_ActiveHitUIInstance->AddToViewport();
				}
			}

			// 表示済みフラグを立てる（これで連発を防ぐ）
			m_bHasShownHitUI = true;
		}
	}

	//現在の使用可否を取得
	bool bCurrentUsable = CheckCurrentSkillUsable();

	//前回のフレームと状態が違っていたら通知を送る
	if (bCurrentUsable != m_bLastUsableState)
	{
		m_bLastUsableState = bCurrentUsable;

		//UI等のリスナーへ通知
		if (OnSkillUsableChanged.IsBound())
		{
			OnSkillUsableChanged.Broadcast(m_bLastUsableState);
		}
	}
}

void UPlayer_SkillComponent::Input_SkillSelect(const FInputActionValue& Value)
{
	if (m_SelectLocked)return;
	if (m_IsSkillActive)return;
	if (!m_Player || !m_Player->HasActorBegunPlay())
	{
		return;
	}

	const float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis))return;

	m_SelectLocked = true;

	if (Axis > 0.5)
	{
		SelectNextSkill();
	}
	else if (Axis < -0.5f)
	{
		SelectPrevSkill();
	}
}

void UPlayer_SkillComponent::Input_SkillReleased()
{
	m_SelectLocked = false;

}

void UPlayer_SkillComponent::Input_SkillActive()
{
	if (!m_Player)return;
	if (m_MovementComponent->GetIsJump() || !m_Player->GetCharacterMovement()->IsMovingOnGround())return;
	if (m_IsSkillActive)return;
	if (!m_CanSkillActive)return;
	UE_LOG(LogTemp, Warning, TEXT("Skill!!!!!!!!!!!"));

	m_CurrentAbilitySkill = nullptr;

	switch (m_SelectedSkill)
	{
	case EPlayerSkill::ThunderFlash:
	{
		if (!CanUseThunderFlash())
		{
			return;
		}
		m_CurrentAbilitySkill = m_Player->m_AbilityPlayer_Skill01;
		StartThunderFlash();
		
		//電力ゲージ消費
		m_ElectroGaugeComponent->SubtractionElectoroGauge(m_ThunderFlashCost);
	}
	break;
	default:
		break;
	}

	if (!m_CurrentAbilitySkill)return;

	m_IsSkillActive = true;
	m_Player->SetIsEnhancedAttack(true);
	m_Player->SetInvincible(true);


	m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_CurrentAbilitySkill);

}

void UPlayer_SkillComponent::SelectNextSkill()
{
	int32 SkillIndex = static_cast<int32>(m_SelectedSkill);
	SkillIndex = (SkillIndex + 1) % static_cast<int32>(EPlayerSkill::MAX);
	m_SelectedSkill = static_cast<EPlayerSkill>(SkillIndex);
}

void UPlayer_SkillComponent::SelectPrevSkill()
{
	int32 SkillIndex = static_cast<int32>(m_SelectedSkill);
	SkillIndex = (SkillIndex - 1 + static_cast<int32>(EPlayerSkill::MAX)) % static_cast<int32>(EPlayerSkill::MAX);
	m_SelectedSkill = static_cast<EPlayerSkill>(SkillIndex);

}

void UPlayer_SkillComponent::StartThunderFlash()
{
	if (!m_Player || !m_Player->HasActorBegunPlay())
	{
		return;
	}


	if (UPlayer_CameraComponent* CameraComp =
		m_Player->FindComponentByClass<UPlayer_CameraComponent>())
	{
		CameraComp->StartSkillCamera();
	}

	m_IsSkillActive = true;
	m_Teleported = false;
	m_DelayElapsed = 0.f;

	m_bHasShownHitUI = false;
	m_ActiveHitUIInstance = nullptr;

	m_StartLocation = m_Player->GetActorLocation();
	m_TargetLocation = m_StartLocation + m_Player->GetActorForwardVector() * m_Skill01Distance;

	m_Player->GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Ignore
	);

	//移動開始
	if (m_MovementComponent)
	{
		m_MovementComponent->SetCanMovement(false);
	}
	if (m_AttackComponent)
	{
		m_AttackComponent->SetCanAttack(false);
	}

}

void UPlayer_SkillComponent::_updateThunderFlash(float DeltaTime)
{
	if (!m_Player || !m_Player->HasActorBegunPlay())
	{
		return;
	}
	m_DelayElapsed += DeltaTime;

	// まだ待ち時間中
	if (m_DelayElapsed < m_DelayTime)
	{
		return;
	}

	// すでにテレポート済みなら何もしない
	if (m_Teleported)
	{
		return;
	}
	//見た目を消す
	m_Player->GetMesh()->SetVisibility(false, true);

	//瞬間移動
	m_Player->SetActorLocation(m_TargetLocation, true);

	//表示を戻す
	m_Player->GetMesh()->SetVisibility(true, true);

	m_Teleported = true;

	//EndSkill();
}



void UPlayer_SkillComponent::EndSkill()
{

	m_Player->SetIsEnhancedAttack(false);

	m_IsSkillActive = false;
	m_Player->SetInvincible(false);

	if (m_ActiveHitUIInstance)
	{
		m_ActiveHitUIInstance->RemoveFromParent();
		m_ActiveHitUIInstance = nullptr;
	}

	m_Player->GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Block
	);


	//移動開始
	if (m_MovementComponent)
	{
		m_MovementComponent->SetCanMovement(true);
	}
	if (m_AttackComponent)
	{
		m_AttackComponent->SetCanAttack(true);
	}

}

bool UPlayer_SkillComponent::CanUseThunderFlash() const
{
	if (!m_ElectroGaugeComponent) return false;

	if (m_ElectroGaugeComponent->IsOvercharge())
	{
		return true;
	}

	return m_ElectroGaugeComponent->GetCurrentGauge() >= m_ThunderFlashCost;
}

bool UPlayer_SkillComponent::GetIsSkillActive() const
{
	return m_IsSkillActive;
}


void UPlayer_SkillComponent::SetCanSkillActive(bool _CanSkillActive)
{
	m_CanSkillActive = _CanSkillActive;
}

bool UPlayer_SkillComponent::CheckCurrentSkillUsable() const
{
	//プレイヤーやゲージコンポーネントがない場合
	if (!m_Player || !m_ElectroGaugeComponent)
	{
		return false;
	}

	//選択中のスキルに応じてコスト判定
	switch (m_SelectedSkill)
	{
	case EPlayerSkill::ThunderFlash:
		return CanUseThunderFlash(); //既存の判定関数を利用

	default:
		return false;
	}
}

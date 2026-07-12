#include "PlayerSkillComponent.h"
#include "CameraRigComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "Skill/PlayerThunderFlashComponent.h"
#include "PlayerAttackComponent.h"
#include "PlayerElectroGaugeComponent.h"
#include "../DataAssets/Player/PlayerSkillParameter.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	//スキル選択の最小値
	constexpr int32 MinSkillIndex = 0;

	//スキルAbilityのデフォルト付与レベル
	constexpr int32 DefaultSkillAbilityLevel = 1;
}

//初期化
UPlayerSkillComponent::UPlayerSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

//開始時
void UPlayerSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player) return;

	CacheComponents();
	InitializeSubComponents();

	//初期選択スキルを設定
	if (
		m_RuntimeState.SelectedSkill == EPlayerSkillType::None &&
		m_SkillSelectOrder.Num() > MinSkillIndex
		)
	{
		SetSelectedSkill(m_SkillSelectOrder[MinSkillIndex]);
	}

	m_bLastUsableState = CheckCurrentSkillUsable();
	OnSkillUsableChanged.Broadcast(m_bLastUsableState);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Skill] BeginPlay Selected:%d CanUse:%s SkillParam:%s ThunderFlashAbility:%s GaugeComp:%s"),
		static_cast<int32>(m_RuntimeState.SelectedSkill),
		m_bLastUsableState ? TEXT("true") : TEXT("false"),
		m_SkillParameter ? TEXT("valid") : TEXT("null"),
		m_ThunderFlashAbilityClass ? *m_ThunderFlashAbilityClass->GetName() : TEXT("None"),
		m_ElectroGaugeComponent ? TEXT("valid") : TEXT("null")
	);
}


//参照を取得
void UPlayerSkillComponent::CacheComponents()
{
	if (!m_Player) return;

	m_ThunderFlashComponent = m_Player->FindComponentByClass<UPlayerThunderFlashComponent>();
	m_AttackComponent = m_Player->FindComponentByClass<UPlayerAttackComponent>();
	m_ElectroGaugeComponent = m_Player->FindComponentByClass<UPlayerElectroGaugeComponent>();
	m_CameraRigComponent = m_Player->FindComponentByClass<UCameraRigComponent>();
}

//サブコンポーネントを初期化
void UPlayerSkillComponent::InitializeSubComponents()
{
	if (m_ThunderFlashComponent)
	{
		m_ThunderFlashComponent->InitializeThunderFlashComponent(
			m_Player,
			m_SkillParameter
		);
	}
}

//スキル状態を完全リセット
void UPlayerSkillComponent::ResetSkill()
{
	if (m_ThunderFlashComponent)
	{
		m_ThunderFlashComponent->EndThunderFlash();
	}

	EndSkillCamera();
	HideSkillHitUI();

	m_RuntimeState.bCanSkillActive = true;

	ResetSkillState();

	UE_LOG(LogTemp, Warning, TEXT("[Skill] ResetSkill Complete"));
}
//現在のスキル使用可能状態を取得
bool UPlayerSkillComponent::IsCurrentSkillUsable() const
{
	return CheckCurrentSkillUsable();
}

//現在のスキル使用可能状態を強制通知
void UPlayerSkillComponent::RefreshSkillUsableNotify()
{
	m_bLastUsableState = CheckCurrentSkillUsable();

	OnSkillUsableChanged.Broadcast(m_bLastUsableState);
}

//初期スキルAbilityを付与
void UPlayerSkillComponent::GiveInitialSkillAbilities()
{
	if (!m_Player)
	{
		m_Player = Cast<APlayerCharacter>(GetOwner());
	}

	if (!m_Player) return;

	if (
		m_RuntimeState.SelectedSkill == EPlayerSkillType::None &&
		m_SkillSelectOrder.Num() > MinSkillIndex
		)
	{
		SetSelectedSkill(m_SkillSelectOrder[MinSkillIndex]);
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	TSubclassOf<UGameplayAbility> SkillAbilityClass =
		GetSelectedSkillAbilityClass();

	if (!SkillAbilityClass) return;

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (!Spec.Ability) continue;

		if (Spec.Ability->GetClass()->IsChildOf(SkillAbilityClass))
		{
			return;
		}
	}

	ASC->GiveAbility(
		FGameplayAbilitySpec(
			SkillAbilityClass,
			DefaultSkillAbilityLevel,
			INDEX_NONE,
			m_Player
		)
	);
}


//毎フレーム更新
void UPlayerSkillComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSkillHitUI();
	UpdateSkillUsableNotify();
}

//スキル選択入力
void UPlayerSkillComponent::InputSkillSelect(const FInputActionValue& Value)
{
	if (m_RuntimeState.bSelectLocked) return;
	if (m_RuntimeState.bIsSkillActive) return;
	if (!m_Player) return;
	if (!m_SkillParameter) return;

	const float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis)) return;

	m_RuntimeState.bSelectLocked = true;

	if (Axis > m_SkillParameter->SelectInputThreshold)
	{
		SelectNextSkill();
		return;
	}

	if (Axis < -m_SkillParameter->SelectInputThreshold)
	{
		SelectPrevSkill();
	}
}

//スキル選択入力解除
void UPlayerSkillComponent::InputSkillReleased()
{
	m_RuntimeState.bSelectLocked = false;
}

//スキル発動入力
void UPlayerSkillComponent::InputSkillActive()
{
	RequestActivateSelectedSkill();
}

//選択中スキルの発動を要求
bool UPlayerSkillComponent::RequestActivateSelectedSkill()
{
	if (!CanStartSkill(m_RuntimeState.SelectedSkill)) return false;

	TSubclassOf<UGameplayAbility> SkillAbilityClass =
		GetSelectedSkillAbilityClass();

	if (!SkillAbilityClass) return false;

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return false;

	FGameplayAbilitySpecHandle SkillHandle;

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (!Spec.Ability) continue;

		if (Spec.Ability->GetClass()->IsChildOf(SkillAbilityClass))
		{
			SkillHandle = Spec.Handle;
			break;
		}
	}

	//未付与ならここで付与して、そのHandleで即起動する
	if (!SkillHandle.IsValid())
	{
		SkillHandle = ASC->GiveAbility(
			FGameplayAbilitySpec(
				SkillAbilityClass,
				DefaultSkillAbilityLevel,
				INDEX_NONE,
				m_Player
			)
		);
	}

	if (!SkillHandle.IsValid()) return false;

	return ASC->TryActivateAbility(SkillHandle);
}


//スキル開始可能か
bool UPlayerSkillComponent::CanStartSkill(EPlayerSkillType SkillType) const
{
	if (!m_Player) return false;
	if (!m_SkillParameter) return false;
	if (!m_RuntimeState.bCanSkillActive) return false;
	if (m_RuntimeState.bIsSkillActive) return false;
	if (!CanUseGroundSkill()) return false;
	if (!CanPaySkillCost(SkillType)) return false;

	if (m_AttackComponent && m_AttackComponent->GetIsInAttackState())
	{
		return false;
	}

	switch (SkillType)
	{
	case EPlayerSkillType::ThunderFlash:
		return CanUseThunderFlash();

	default:
		return false;
	}
}


//スキル開始
void UPlayerSkillComponent::BeginSkillByType(
	const FGameplayTag& SkillTag,
	EPlayerSkillType SkillType
)
{
	if (!m_Player) return;
	if (!SkillTag.IsValid()) return;

	if (!ConsumeSkillCost(SkillType))
	{
		return;
	}

	ApplySkillStartState(SkillTag, SkillType);
	StartSkillCamera();

	switch (SkillType)
	{
	case EPlayerSkillType::ThunderFlash:
		if (m_ThunderFlashComponent)
		{
			m_ThunderFlashComponent->BeginThunderFlash();
		}
		break;

	default:
		break;
	}

	OnSkillStarted.Broadcast(SkillTag, SkillType);
}


//スキル終了
void UPlayerSkillComponent::EndSkillByType(
	const FGameplayTag& SkillTag,
	EPlayerSkillType SkillType,
	bool bWasCancelled
)
{
	switch (SkillType)
	{
	case EPlayerSkillType::ThunderFlash:
		if (m_ThunderFlashComponent)
		{
			m_ThunderFlashComponent->EndThunderFlash();
		}
		break;

	default:
		break;
	}

	EndSkillCamera();
	HideSkillHitUI();
	ResetSkillState();

	OnSkillEnded.Broadcast(SkillTag, SkillType);
}


//スキル発動可能状態を設定
void UPlayerSkillComponent::SetCanSkillActive(bool bCanSkillActive)
{
	m_RuntimeState.bCanSkillActive = bCanSkillActive;

	UpdateSkillUsableNotify();
}

//スキルを強制終了
void UPlayerSkillComponent::EndSkill()
{
	if (!m_RuntimeState.bIsSkillActive) return;

	EndSkillByType(
		m_CurrentSkillTag,
		m_CurrentSkillType,
		false
	);
}

//現在選択中のスキルが使用可能か
bool UPlayerSkillComponent::CheckCurrentSkillUsable() const
{
	return CanStartSkill(m_RuntimeState.SelectedSkill);
}

//次のスキルを選択
void UPlayerSkillComponent::SelectNextSkill()
{
	if (m_SkillSelectOrder.Num() <= 1) return;

	const int32 CurrentIndex =
		m_SkillSelectOrder.IndexOfByKey(m_RuntimeState.SelectedSkill);

	if (CurrentIndex == INDEX_NONE)
	{
		SetSelectedSkill(m_SkillSelectOrder[0]);
		return;
	}

	const int32 NextIndex =
		(CurrentIndex + 1) % m_SkillSelectOrder.Num();

	SetSelectedSkill(m_SkillSelectOrder[NextIndex]);
}


//前のスキルを選択
void UPlayerSkillComponent::SelectPrevSkill()
{
	if (m_SkillSelectOrder.Num() <= 1) return;

	const int32 CurrentIndex =
		m_SkillSelectOrder.IndexOfByKey(m_RuntimeState.SelectedSkill);

	if (CurrentIndex == INDEX_NONE)
	{
		SetSelectedSkill(m_SkillSelectOrder[0]);
		return;
	}

	int32 PrevIndex = CurrentIndex - 1;

	if (PrevIndex < 0)
	{
		PrevIndex = m_SkillSelectOrder.Num() - 1;
	}

	SetSelectedSkill(m_SkillSelectOrder[PrevIndex]);
}


//スキル選択を設定
void UPlayerSkillComponent::SetSelectedSkill(EPlayerSkillType SkillType)
{
	if (SkillType == EPlayerSkillType::None) return;
	if (SkillType == EPlayerSkillType::MAX) return;

	m_RuntimeState.SelectedSkill = SkillType;

	UpdateSkillUsableNotify();
}

//ThunderFlashが使用可能か
bool UPlayerSkillComponent::CanUseThunderFlash() const
{
	return CanPaySkillCost(EPlayerSkillType::ThunderFlash);
}

//スキルコストを支払えるか
bool UPlayerSkillComponent::CanPaySkillCost(EPlayerSkillType SkillType) const
{
	if (!m_ElectroGaugeComponent) return false;
	if (!m_SkillParameter) return false;

	switch (SkillType)
	{
	case EPlayerSkillType::ThunderFlash:
		if (m_ElectroGaugeComponent->IsOvercharge())
		{
			return true;
		}

		return m_ElectroGaugeComponent->GetCurrentGauge() >=
			m_SkillParameter->ThunderFlashCost;

	default:
		return false;
	}
}

//スキルコストを消費
bool UPlayerSkillComponent::ConsumeSkillCost(EPlayerSkillType SkillType)
{
	if (!m_ElectroGaugeComponent) return false;
	if (!m_SkillParameter) return false;

	if (!CanPaySkillCost(SkillType))
	{
		return false;
	}

	switch (SkillType)
	{
	case EPlayerSkillType::ThunderFlash:
		m_ElectroGaugeComponent->SubtractElectroGauge(
			m_SkillParameter->ThunderFlashCost
		);
		return true;

	default:
		return false;
	}
}


//地上スキルを使えるか
bool UPlayerSkillComponent::CanUseGroundSkill() const
{
	if (!m_Player) return false;

	const UCharacterMovementComponent* MoveComp =
		m_Player->GetCharacterMovement();

	return MoveComp && MoveComp->IsMovingOnGround();
}

//スキル状態を設定
void UPlayerSkillComponent::SetSkillState(EPlayerSkillState NewState)
{
	if (m_RuntimeState.SkillState == NewState) return;

	const EPlayerSkillState PreviousState =
		m_RuntimeState.SkillState;

	m_RuntimeState.SkillState = NewState;

	OnSkillStateChanged.Broadcast(
		PreviousState,
		NewState
	);
}

//スキル開始時の共通状態を設定
void UPlayerSkillComponent::ApplySkillStartState(
	const FGameplayTag& SkillTag,
	EPlayerSkillType SkillType
)
{
	m_CurrentSkillTag = SkillTag;
	m_CurrentSkillType = SkillType;

	m_RuntimeState.SelectedSkill = SkillType;
	m_RuntimeState.bIsSkillActive = true;
	m_RuntimeState.bHasShownHitUI = false;

	SetSkillState(EPlayerSkillState::Active);

	if (m_Player)
	{
		m_Player->SetIsEnhancedAttack(true);
		m_Player->SetInvincible(true);
	}

	if (m_AttackComponent)
	{
		m_AttackComponent->SetCanAttack(false);
	}
}

//スキル終了時の共通状態を戻す
void UPlayerSkillComponent::ResetSkillState()
{
	m_CurrentSkillTag = FGameplayTag();
	m_CurrentSkillType = EPlayerSkillType::None;

	m_RuntimeState.bIsSkillActive = false;
	m_RuntimeState.bHasShownHitUI = false;
	m_RuntimeState.bSelectLocked = false;

	SetSkillState(EPlayerSkillState::None);

	if (m_Player)
	{
		m_Player->SetIsEnhancedAttack(false);
		m_Player->SetInvincible(false);
	}

	if (m_AttackComponent)
	{
		m_AttackComponent->SetCanAttack(true);
	}
}

//スキル使用可能通知を更新
void UPlayerSkillComponent::UpdateSkillUsableNotify()
{
	const bool bCurrentUsable = CheckCurrentSkillUsable();

	if (bCurrentUsable == m_bLastUsableState)
	{
		return;
	}

	m_bLastUsableState = bCurrentUsable;

	OnSkillUsableChanged.Broadcast(m_bLastUsableState);
}

//選択中スキルのAbilityを取得
TSubclassOf<UGameplayAbility> UPlayerSkillComponent::GetSelectedSkillAbilityClass() const
{
	switch (m_RuntimeState.SelectedSkill)
	{
	case EPlayerSkillType::ThunderFlash:
		return m_ThunderFlashAbilityClass;

	default:
		return nullptr;
	}
}

//ASCを取得
UAbilitySystemComponent* UPlayerSkillComponent::GetASC() const
{
	if (!m_Player) return nullptr;

	return m_Player->GetAbilitySystemComponent();
}

//スキルカメラを開始
void UPlayerSkillComponent::StartSkillCamera()
{
	if (!m_CameraRigComponent) return;

	m_CameraRigComponent->RequestSkillCamera();
}

//スキルカメラを終了
void UPlayerSkillComponent::EndSkillCamera()
{
	if (!m_CameraRigComponent) return;

	m_CameraRigComponent->ClearActionCameraRequest();
}


//ヒットUIを更新
void UPlayerSkillComponent::UpdateSkillHitUI()
{
	if (!m_Player) return;
	if (!m_SkillParameter) return;
	if (!m_RuntimeState.bIsSkillActive) return;
	if (m_RuntimeState.bHasShownHitUI) return;
	if (!m_Player->GetIsHit()) return;

	ShowSkillHitUI();

	m_RuntimeState.bHasShownHitUI = true;
}

//ヒットUIを表示
void UPlayerSkillComponent::ShowSkillHitUI()
{
	if (!m_SkillParameter) return;
	if (!m_SkillParameter->ThunderFlashHitUIClass) return;
	if (m_ActiveHitUIInstance) return;

	m_ActiveHitUIInstance =
		CreateWidget<UUserWidget>(
			GetWorld(),
			m_SkillParameter->ThunderFlashHitUIClass
		);

	if (m_ActiveHitUIInstance)
	{
		m_ActiveHitUIInstance->AddToViewport();
	}
}

//ヒットUIを非表示
void UPlayerSkillComponent::HideSkillHitUI()
{
	if (!m_ActiveHitUIInstance) return;

	m_ActiveHitUIInstance->RemoveFromParent();
	m_ActiveHitUIInstance = nullptr;
}
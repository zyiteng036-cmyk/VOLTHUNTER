#include "PlayerAttackComboComponent.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../PlayerAttackComponent.h"
#include "../../DataAssets/Player/PlayerComboTable.h"
#include "AbilitySystemComponent.h"

namespace
{
	//弱攻撃入力タグ
	const FName Tag_Input_Attack_Light = TEXT("Input.Attack.Light");

	//強攻撃押下タグ
	const FName Tag_Input_Attack_HeavyPressed = TEXT("Input.Attack.Heavy");

	//強攻撃離しタグ
	const FName Tag_Input_Attack_HeavyReleased = TEXT("Input.Attack.Heavy.Release");

	//コンボ予約受付中タグ
	const FName Tag_State_Attack_ComboReserveWindow = TEXT("State.Attack.ComboReserveWindow");

	//コンボ発動受付中タグ
	const FName Tag_State_Attack_ComboActivationWindow = TEXT("State.Attack.ComboActivationWindow");

}

//初期化
UPlayerAttackComboComponent::UPlayerAttackComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	m_LightAttackInputTag = FGameplayTag::RequestGameplayTag(Tag_Input_Attack_Light);
	m_HeavyAttackPressedInputTag = FGameplayTag::RequestGameplayTag(Tag_Input_Attack_HeavyPressed);
	m_HeavyAttackReleasedInputTag = FGameplayTag::RequestGameplayTag(Tag_Input_Attack_HeavyReleased);

	m_ComboReserveWindowStateTag = FGameplayTag::RequestGameplayTag(Tag_State_Attack_ComboReserveWindow);
	m_ComboActivationWindowStateTag = FGameplayTag::RequestGameplayTag(Tag_State_Attack_ComboActivationWindow);

}

//開始時
void UPlayerAttackComboComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	m_AttackComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UPlayerAttackComponent>()
		: nullptr;
}

//参照を初期化
void UPlayerAttackComboComponent::InitializeComboComponent(
	APlayerCharacter* Player,
	UPlayerAttackComponent* AttackComponent,
	UPlayerComboTable* ComboTable
)
{
	m_Player = Player;
	m_AttackComponent = AttackComponent;
	m_ComboTable = ComboTable;
}

//コンボテーブルを設定
void UPlayerAttackComboComponent::SetComboTable(UPlayerComboTable* ComboTable)
{
	m_ComboTable = ComboTable;
}

//弱攻撃要求
bool UPlayerAttackComboComponent::RequestLightAttack()
{
	return HandleAttackInputByTag(m_LightAttackInputTag);
}

//強攻撃押下要求
bool UPlayerAttackComboComponent::RequestHeavyAttackPressed()
{
	return HandleAttackInputByTag(m_HeavyAttackPressedInputTag);
}

//強攻撃離し要求
bool UPlayerAttackComboComponent::RequestHeavyAttackReleased()
{
	return HandleAttackInputByTag(m_HeavyAttackReleasedInputTag);
}

//攻撃入力タグを処理
bool UPlayerAttackComboComponent::HandleAttackInputByTag(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return false;
	if (!m_Player) return false;
	if (!m_AttackComponent) return false;
	if (!m_ComboTable) return false;


	const bool bHasCurrentAttack =
		m_AttackComponent->GetCurrentAttackTag().IsValid();

	const bool bIsHeavyReleasedInput =
		InputTag.MatchesTagExact(m_HeavyAttackReleasedInputTag);

	const bool bHasBufferedHeavyPressedInput =
		m_bHasBufferedInput &&
		m_BufferedInputTag.MatchesTagExact(m_HeavyAttackPressedInputTag);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AttackCombo][Input] Input:%s Current:%s Reserve:%s Activation:%s Buffered:%s Transition:%s"),
		*InputTag.ToString(),
		*m_AttackComponent->GetCurrentAttackTag().ToString(),
		m_bComboReserveWindowOpen ? TEXT("true") : TEXT("false"),
		m_bComboActivationWindowOpen ? TEXT("true") : TEXT("false"),
		m_bHasBufferedInput
		? *m_BufferedInputTag.ToString()
		: TEXT("None"),
		m_bComboTransitionInProgress ? TEXT("true") : TEXT("false")
	);

	//ジャスト回避後の発動窓中は予約入力を即時発動する
	if (
		m_bJustEvasiveCounterActivationWindowOpen &&
		m_AttackComponent->GetIsJustEvasiveCounterReady()
		)
	{
		//予約済みの強攻撃を離し入力で上書きしない
		if (bIsHeavyReleasedInput)
		{
			if (!bHasBufferedHeavyPressedInput)
			{
				return false;
			}

			m_bBufferedJustEvasiveHeavyReleased = true;

			return TryActivateBufferedJustEvasiveCounter();
		}

		BufferAttackInput(InputTag);
		return TryActivateBufferedJustEvasiveCounter();
	}

	//ジャスト回避後の予約窓中は入力を保存する
	if (
		m_bJustEvasiveCounterReserveWindowOpen &&
		m_AttackComponent->GetIsJustEvasiveCounterReady()
		)
	{
		//強攻撃を離した場合は押下予約を残して離し状態だけ保存する
		if (bIsHeavyReleasedInput)
		{
			if (!bHasBufferedHeavyPressedInput)
			{
				return false;
			}

			m_bBufferedJustEvasiveHeavyReleased = true;

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[JustEvasiveCounter] Heavy release cached")
			);

			return true;
		}

		BufferAttackInput(InputTag);
		return true;
	}

	//コンボ窓が開いているのにCurrentAttackTagが無いのは状態破綻
	//ここで1段目を再起動しに行くと、BP_GA_AttackLight01を二重起動して失敗する
	if (!bHasCurrentAttack &&
		(m_bComboReserveWindowOpen || m_bComboActivationWindowOpen))
	{
		return false;
	}

	if (!bHasCurrentAttack)
	{
		if (!CanStartAttackFromInput())
		{
			return false;
		}

		return TryStartAttackFromInput(InputTag);
	}

	if (
		m_AttackComponent->GetIsHeavyCharging() &&
		InputTag.MatchesTagExact(m_HeavyAttackReleasedInputTag)
		)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[AttackCombo] Heavy Release Accepted Current:%s Input:%s"),
			*m_AttackComponent->GetCurrentAttackTag().ToString(),
			*InputTag.ToString()
		);
		BufferAttackInput(InputTag);
		return TryActivateBufferedCombo();
	}

	if (m_bComboActivationWindowOpen)
	{
		BufferAttackInput(InputTag);
		return TryActivateBufferedCombo();
	}

	if (m_bComboReserveWindowOpen)
	{
		BufferAttackInput(InputTag);
		return true;
	}

	return false;
}
//攻撃開始入力を受け付けられるか
bool UPlayerAttackComboComponent::CanStartAttackFromInput() const
{
	if (!m_AttackComponent) return false;
	if (!m_AttackComponent->GetCanAttack()) return false;
	if (m_AttackComponent->GetIsAirAttackStart()) return false;
	if (m_AttackComponent->GetHeavyAttackStart()) return false;
	if (m_AttackComponent->GetIsHeavyCharging()) return false;

	return true;
}

//コンボ遷移前に受付窓を閉じる
void UPlayerAttackComboComponent::CloseComboWindowsForTransition()
{
	CloseComboReserveWindow();
	CloseComboActivationWindow();
}


//攻撃未開始状態から攻撃を開始
bool UPlayerAttackComboComponent::TryStartAttackFromInput(const FGameplayTag& InputTag)
{
	if (!m_ComboTable)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return false;
	}

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	const FPlayerComboLink* StartLink =
		m_ComboTable->FindComboLink(
			FGameplayTag(),
			InputTag,
			OwnedTags
		);

	if (!StartLink)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[AttackCombo] StartLink not found Input:%s Tags:%s"),
			*InputTag.ToString(),
			*OwnedTags.ToStringSimple()
		);

		return false;
	}

	return TryActivateComboLink(*StartLink);
}

//入力タグを予約
void UPlayerAttackComboComponent::BufferAttackInput(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	m_BufferedInputTag = InputTag;
	m_bHasBufferedInput = true;
	m_bBufferedJustEvasiveHeavyReleased = false;

	OnAttackInputBuffered.Broadcast(InputTag);
}


//予約入力をクリア
void UPlayerAttackComboComponent::ClearBufferedInput()
{
	m_BufferedInputTag = FGameplayTag();
	m_bHasBufferedInput = false;
	m_bBufferedJustEvasiveHeavyReleased = false;
}

//ジャスト回避後の攻撃予約受付開始
void UPlayerAttackComboComponent::OpenJustEvasiveCounterReserveWindow()
{
	if (m_bJustEvasiveCounterReserveWindowOpen) return;

	m_bJustEvasiveCounterReserveWindowOpen = true;
}

//ジャスト回避後の攻撃予約受付終了
void UPlayerAttackComboComponent::CloseJustEvasiveCounterReserveWindow()
{
	if (!m_bJustEvasiveCounterReserveWindowOpen) return;

	m_bJustEvasiveCounterReserveWindowOpen = false;
}

//ジャスト回避後の攻撃発動受付開始
void UPlayerAttackComboComponent::OpenJustEvasiveCounterActivationWindow()
{
	if (m_bJustEvasiveCounterActivationWindowOpen) return;

	m_bJustEvasiveCounterActivationWindowOpen = true;

	if (m_bHasBufferedInput)
	{
		TryActivateBufferedJustEvasiveCounter();
	}
}

//ジャスト回避後の攻撃発動受付終了
void UPlayerAttackComboComponent::CloseJustEvasiveCounterActivationWindow()
{
	if (!m_bJustEvasiveCounterActivationWindowOpen) return;

	m_bJustEvasiveCounterActivationWindowOpen = false;

	if (m_AttackComponent)
	{
		m_AttackComponent->SetJustEvasiveCounterReady(false);
	}
}

//予約入力からジャスト回避後の攻撃を発動
bool UPlayerAttackComboComponent::TryActivateBufferedJustEvasiveCounter()
{
	if (!m_bHasBufferedInput) return false;
	if (!m_BufferedInputTag.IsValid()) return false;
	if (!m_AttackComponent) return false;
	if (!m_AttackComponent->GetIsJustEvasiveCounterReady()) return false;

	const FGameplayTag CachedInputTag = m_BufferedInputTag;
	const bool bIsHeavyAttack = CachedInputTag.MatchesTagExact(m_HeavyAttackPressedInputTag);

	//発動前にHeavyを離していたか保存する
	const bool bReleaseHeavyAfterActivation =
		bIsHeavyAttack &&
		m_bBufferedJustEvasiveHeavyReleased;

	ClearBufferedInput();

	//ジャスト回避後の強攻撃は即Full状態にする
	if (bIsHeavyAttack)
	{
		m_AttackComponent->PrepareJustEvasiveFullCharge();
	}

	const bool bActivated = TryStartAttackFromInput(CachedInputTag);

	if (!bActivated)
	{
		if (bIsHeavyAttack)
		{
			m_AttackComponent->ClearJustEvasiveFullCharge();
		}

		BufferAttackInput(CachedInputTag);
		m_bBufferedJustEvasiveHeavyReleased = bReleaseHeavyAfterActivation;
		return false;
	}

	m_bJustEvasiveCounterReserveWindowOpen = false;
	m_bJustEvasiveCounterActivationWindowOpen = false;

	m_AttackComponent->ConsumeJustEvasiveCounterReady();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[JustEvasiveCounter] Activated Input:%s FullHeavy:%s Released:%s"),
		*CachedInputTag.ToString(),
		bIsHeavyAttack ? TEXT("true") : TEXT("false"),
		bReleaseHeavyAfterActivation ? TEXT("true") : TEXT("false")
	);

	//予約中に離していた場合はHeavy開始直後にFull派生を実行する
	if (bReleaseHeavyAfterActivation)
	{
		return HandleAttackInputByTag(m_HeavyAttackReleasedInputTag);
	}

	return true;
}



//現在攻撃タグと入力タグからコンボ派生を探す
const FPlayerComboLink* UPlayerAttackComboComponent::FindComboLinkFromCurrentState(
	const FGameplayTag& InputTag
) const
{
	if (!m_AttackComponent) return nullptr;
	if (!m_ComboTable) return nullptr;
	if (!InputTag.IsValid()) return nullptr;

	const FGameplayTag& CurrentAttackTag =
		m_AttackComponent->GetCurrentAttackTag();

	if (!CurrentAttackTag.IsValid())
	{
		return nullptr;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return nullptr;

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[HeavyReleaseTags] Current:%s Input:%s OwnedTags:%s"),
		*CurrentAttackTag.ToString(),
		*InputTag.ToString(),
		*OwnedTags.ToStringSimple()
	);

	return m_ComboTable->FindComboLink(
		CurrentAttackTag,
		InputTag,
		OwnedTags
	);
}

//予約入力からコンボ発動
bool UPlayerAttackComboComponent::TryActivateBufferedCombo()
{
	if (m_bComboTransitionInProgress)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[AttackCombo][Blocked] Combo transition already in progress")
		);

		return false;
	}

	if (!m_bHasBufferedInput) return false;
	if (!m_BufferedInputTag.IsValid()) return false;

	const FGameplayTag CachedInputTag = m_BufferedInputTag;

	const FPlayerComboLink* ComboLink =
		FindComboLinkFromCurrentState(CachedInputTag);

	if (!ComboLink)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[AttackCombo][LinkMissing] Current:%s Input:%s Reserve:%s Activation:%s"),
			m_AttackComponent
			? *m_AttackComponent->GetCurrentAttackTag().ToString()
			: TEXT("None"),
			*CachedInputTag.ToString(),
			m_bComboReserveWindowOpen ? TEXT("true") : TEXT("false"),
			m_bComboActivationWindowOpen ? TEXT("true") : TEXT("false")
		);

		return false;
	}

	ClearBufferedInput();

	const bool bActivated =
		TryActivateComboLink(*ComboLink);

	if (!bActivated)
	{
		//現在の攻撃がまだ生きている場合だけ入力を戻す
		if (
			m_AttackComponent &&
			m_AttackComponent->GetCurrentAttackTag().IsValid()
			)
		{
			BufferAttackInput(CachedInputTag);
		}

		return false;
	}

	return true;
}


//コンボ派生を発動
bool UPlayerAttackComboComponent::TryActivateComboLink(
	const FPlayerComboLink& ComboLink
)
{
	if (m_bComboTransitionInProgress)
	{
		return false;
	}

	if (!ComboLink.NextAbilityClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[AttackCombo][ActivateFailed] NextAbilityClass is null")
		);

		return false;
	}

	if (!m_AttackComponent)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[AttackCombo][ActivateFailed] AttackComponent is null")
		);

		return false;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[AttackCombo][ActivateFailed] ASC is null")
		);

		return false;
	}

	bool bHasSpec = false;

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (!Spec.Ability) continue;

		if (Spec.Ability->GetClass() == ComboLink.NextAbilityClass.Get())
		{
			bHasSpec = true;
			break;
		}
	}

	const FGameplayTag CurrentAttackTag =
		m_AttackComponent->GetCurrentAttackTag();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AttackCombo][TransitionStart] Current:%s Next:%s HasSpec:%s CancelCurrent:%s Reserve:%s Activation:%s"),
		*CurrentAttackTag.ToString(),
		*ComboLink.NextAbilityClass->GetName(),
		bHasSpec ? TEXT("true") : TEXT("false"),
		ComboLink.bCancelCurrentAttack ? TEXT("true") : TEXT("false"),
		m_bComboReserveWindowOpen ? TEXT("true") : TEXT("false"),
		m_bComboActivationWindowOpen ? TEXT("true") : TEXT("false")
	);

	//未登録Abilityを起動しようとして現在攻撃を消さない
	if (!bHasSpec)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[AttackCombo][MissingSpec] Ability:%s Current:%s"),
			*ComboLink.NextAbilityClass->GetName(),
			*CurrentAttackTag.ToString()
		);

		return false;
	}

	m_bComboTransitionInProgress = true;

	if (ComboLink.bCancelCurrentAttack)
	{
		//前段の受付窓を次段へ持ち越さない
		CloseComboWindowsForTransition();

		m_AttackComponent->SetComboTransitioning(true);

		FGameplayTagContainer CancelTags =
			ComboLink.CancelAbilityTags;

		//未設定時も攻撃Abilityを確実にキャンセルする
		if (CancelTags.IsEmpty())
		{
			const FGameplayTag AttackAbilityTag =
				FGameplayTag::RequestGameplayTag(
					TEXT("Ability.Attack"),
					false
				);

			if (AttackAbilityTag.IsValid())
			{
				CancelTags.AddTag(AttackAbilityTag);
			}
		}

		if (CancelTags.IsEmpty())
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("[AttackCombo][ActivateFailed] CancelAbilityTags is empty Current:%s Next:%s"),
				*CurrentAttackTag.ToString(),
				*ComboLink.NextAbilityClass->GetName()
			);

			m_AttackComponent->SetComboTransitioning(false);
			m_bComboTransitionInProgress = false;
			return false;
		}

		CancelAbilitiesByTags(CancelTags);
	}

	FGameplayTagContainer AfterCancelTags;
	ASC->GetOwnedGameplayTags(AfterCancelTags);

	const bool bActivated =
		ASC->TryActivateAbilityByClass(
			ComboLink.NextAbilityClass
		);

	if (bActivated)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[AttackCombo][TransitionResult] Current:%s Next:%s Result:true Tags:%s"),
			*CurrentAttackTag.ToString(),
			*ComboLink.NextAbilityClass->GetName(),
			*AfterCancelTags.ToStringSimple()
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[AttackCombo][TransitionResult] Current:%s Next:%s Result:false Tags:%s"),
			*CurrentAttackTag.ToString(),
			*ComboLink.NextAbilityClass->GetName(),
			*AfterCancelTags.ToStringSimple()
		);
	}


	if (!bActivated)
	{
		m_AttackComponent->SetComboTransitioning(false);

		//元Abilityはすでにキャンセルされているため、
		//中途半端な攻撃状態を残さず完全終了する
		if (ComboLink.bCancelCurrentAttack)
		{
			m_AttackComponent->FinishAttack();
		}

		m_bComboTransitionInProgress = false;
		return false;
	}

	m_bComboTransitionInProgress = false;
	return true;
}


//AbilityタグでAbilityをキャンセル
void UPlayerAttackComboComponent::CancelAbilitiesByTags(const FGameplayTagContainer& CancelTags)
{
	if (CancelTags.IsEmpty()) return;

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	ASC->CancelAbilities(&CancelTags);
}

//コンボ予約受付開始
void UPlayerAttackComboComponent::OpenComboReserveWindow()
{
	if (m_bComboReserveWindowOpen) return;

	m_bComboReserveWindowOpen = true;

	if (UAbilitySystemComponent* ASC = GetASC())
	{
		if (m_ComboReserveWindowStateTag.IsValid())
		{
			ASC->AddLooseGameplayTag(m_ComboReserveWindowStateTag);
		}
	}

	OnComboReserveWindowChanged.Broadcast(true);
}

//コンボ予約受付終了
void UPlayerAttackComboComponent::CloseComboReserveWindow()
{
	if (!m_bComboReserveWindowOpen) return;

	m_bComboReserveWindowOpen = false;

	if (UAbilitySystemComponent* ASC = GetASC())
	{
		if (m_ComboReserveWindowStateTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(m_ComboReserveWindowStateTag);
		}
	}

	OnComboReserveWindowChanged.Broadcast(false);
}

//コンボ発動受付開始
void UPlayerAttackComboComponent::OpenComboActivationWindow()
{
	if (m_bComboActivationWindowOpen) return;

	m_bComboActivationWindowOpen = true;

	if (UAbilitySystemComponent* ASC = GetASC())
	{
		if (m_ComboActivationWindowStateTag.IsValid())
		{
			ASC->AddLooseGameplayTag(m_ComboActivationWindowStateTag);
		}
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AttackCombo][Window] Activation Open Current:%s Buffered:%s"),
		m_AttackComponent
		? *m_AttackComponent->GetCurrentAttackTag().ToString()
		: TEXT("None"),
		m_bHasBufferedInput
		? *m_BufferedInputTag.ToString()
		: TEXT("None")
	);

	//先に窓が開いたことを通知する
	OnComboActivationWindowChanged.Broadcast(true);

	//予約入力があればコンボを発動する
	TryActivateBufferedCombo();
}

//コンボ発動受付終了
void UPlayerAttackComboComponent::CloseComboActivationWindow()
{
	if (!m_bComboActivationWindowOpen) return;

	m_bComboActivationWindowOpen = false;

	if (UAbilitySystemComponent* ASC = GetASC())
	{
		if (m_ComboActivationWindowStateTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(m_ComboActivationWindowStateTag);
		}
	}

	OnComboActivationWindowChanged.Broadcast(false);
}

//ASCを取得
UAbilitySystemComponent* UPlayerAttackComboComponent::GetASC() const
{
	if (!m_Player) return nullptr;

	return m_Player->GetAbilitySystemComponent();
}
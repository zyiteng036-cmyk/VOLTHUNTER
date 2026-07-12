#include "PlayerCharacter.h"
#include "PlayerAttributeSet.h"
#include "../AbilityPlayer/Movement/GA_Move.h"
#include "../Enemy/EnemyBase.h"
#include "../Event/GameplayAreaEventManager/GameplayAreaEventManager.h"
#include "../GameMode/GameMode_Ingame.h"
#include "../HitJudgmentComponent/HitJudgmentComponent.h"
#include "../PlayerComponent/Attack/PlayerAttackComboComponent.h"
#include "../PlayerComponent/Attack/PlayerAttackMotionComponent.h"
#include "../PlayerComponent/Attack/PlayerAttackTargetComponent.h"
#include "../PlayerComponent/CameraRigComponent.h"
#include "../PlayerComponent/CustomCharacterMovementComponent.h"
#include "../PlayerComponent/PlayerAttackComponent.h"
#include "../PlayerComponent/PlayerElectroGaugeComponent.h"
#include "../PlayerComponent/PlayerEvasiveComponent.h"
#include "../PlayerComponent/PlayerSkillComponent.h"
#include "../PlayerComponent/Skill/PlayerThunderFlashComponent.h"
#include "../PlayerNotifySubSystem/PlayerNotifySubsystem.h"
#include "../PlayerSplineMove/Player_SplineMove.h"
#include "../SequenceWorldSubsystem/SequenceWorldSubsystem.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "TimerManager.h"
#include "../PlayerSettingsSubsystem/PlayerSettingsSubsystem.h"
#include "Engine/GameInstance.h"

//サブオブジェクトを生成して初期化
APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	//GASとAttributeを生成
	m_AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	m_PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));

	//差し替えたカスタム移動コンポーネントを取得
	m_CustomMovementComponent = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());

	//プレイヤー追従カメラを構成
	m_SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_SpringArm->SetupAttachment(GetCapsuleComponent());
	m_SpringArm->SetRelativeLocation(m_DefaultSpringArmRelativeLocation);
	m_SpringArm->bUsePawnControlRotation = true;
	m_SpringArm->bInheritPitch = true;
	m_SpringArm->bInheritYaw = true;
	m_SpringArm->bInheritRoll = false;
	m_SpringArm->SocketOffset = m_DefaultCameraSocketOffset;
	m_SpringArm->ProbeChannel = ECC_Visibility;

	m_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_Camera->SetupAttachment(m_SpringArm, USpringArmComponent::SocketName);
	m_Camera->bUsePawnControlRotation = false;

	//プレイヤー機能ごとのComponentを生成
	m_CameraRigComponent = CreateDefaultSubobject<UCameraRigComponent>(TEXT("CameraRigComponent"));
	m_EvasiveComp = CreateDefaultSubobject<UPlayerEvasiveComponent>(TEXT("EvasiveComponent"));
	m_ElectroGaugeComp = CreateDefaultSubobject<UPlayerElectroGaugeComponent>(TEXT("ElectroGaugeComponent"));
	m_AttackComp = CreateDefaultSubobject<UPlayerAttackComponent>(TEXT("PlayerAttackComponent"));
	m_AttackTargetComponent = CreateDefaultSubobject<UPlayerAttackTargetComponent>(TEXT("AttackTargetComponent"));
	m_AttackComboComponent = CreateDefaultSubobject<UPlayerAttackComboComponent>(TEXT("AttackComboComponent"));
	m_AttackMotionComponent = CreateDefaultSubobject<UPlayerAttackMotionComponent>(TEXT("AttackMotionComponent"));
	m_HitJudgmentComponent = CreateDefaultSubobject<UHitJudgmentComponent>(TEXT("HitJudgmentComponent"));
	m_SkillComp = CreateDefaultSubobject<UPlayerSkillComponent>(TEXT("SkillComponent"));
	m_ThunderFlashComponent = CreateDefaultSubobject<UPlayerThunderFlashComponent>(TEXT("ThunderFlashComponent"));

	//落下中の横方向速度を維持しやすくする
	GetCharacterMovement()->FallingLateralFriction = DefaultFallingLateralFriction;
}

//開始時にプレイヤーシステムを初期化
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//設定画面で変更されるプレイヤー設定を取得
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		m_PlayerSettingsSubsystem = GameInstance->GetSubsystem<UPlayerSettingsSubsystem>();
	}

	InitializeCharacterSettings();
	InitializeAbilitySystem();
	InitializeAttributes();
	BindWorldEvents();

	OnPlayerRespawn.Broadcast(this);
}


//キャラクター基本設定を初期化
void APlayerCharacter::InitializeCharacterSettings()
{
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent) return;

	//移動方向へキャラクターを自動旋回
	MovementComponent->bOrientRotationToMovement = true;
	MovementComponent->RotationRate = m_CharacterRotationRate;
}

//AbilitySystemComponentを初期化
void APlayerCharacter::InitializeAbilitySystem()
{
	if (!m_AbilitySystemComponent) return;

	m_AbilitySystemComponent->InitAbilityActorInfo(this, this);

	GiveAndActivateMoveAbility();
	GiveActionAbilities();
	GivePassiveStartupAbilities();

	//初期選択スキルを使用できるように登録
	if (m_SkillComp)
	{
		m_SkillComp->GiveInitialSkillAbilities();
	}
}

//Attributeの初期値を設定
void APlayerCharacter::InitializeAttributes()
{
	if (!m_PlayerAttributeSet) return;

	//最大値が未設定の場合だけ安全な初期値を使用
	if (m_PlayerAttributeSet->GetMaxHealth() <= 0.f)
	{
		m_PlayerAttributeSet->SetMaxHealth(DefaultMaxHealth);
	}

	if (m_PlayerAttributeSet->GetMaxElectroGauge() <= 0.f)
	{
		m_PlayerAttributeSet->SetMaxElectroGauge(DefaultMaxElectroGauge);
	}

	//開始時はHP最大、電力ゲージ0で初期化
	m_PlayerAttributeSet->SetHealth(m_PlayerAttributeSet->GetMaxHealth());
	m_PlayerAttributeSet->SetElectroGauge(0.f);
}

//移動Abilityを登録して起動
void APlayerCharacter::GiveAndActivateMoveAbility()
{
	if (!m_AbilitySystemComponent || !m_MoveAbilityClass) return;

	FGameplayAbilitySpec MoveSpec(m_MoveAbilityClass, DefaultAbilityLevel);
	const FGameplayAbilitySpecHandle MoveHandle = m_AbilitySystemComponent->GiveAbility(MoveSpec);

	m_AbilitySystemComponent->TryActivateAbility(MoveHandle);

	//常駐する移動AbilityのPrimaryInstanceを保持
	if (FGameplayAbilitySpec* ActiveSpec = m_AbilitySystemComponent->FindAbilitySpecFromHandle(MoveHandle))
	{
		m_ActiveMoveAbility = Cast<UGA_Move>(ActiveSpec->GetPrimaryInstance());
	}
}

//入力用Abilityを登録
void APlayerCharacter::GiveActionAbilities()
{
	if (!m_AbilitySystemComponent) return;

	for (int32 MappingIndex = 0; MappingIndex < ActionInputMappings.Num(); ++MappingIndex)
	{
		FPlayerAbilityInputMapping& Mapping = ActionInputMappings[MappingIndex];
		if (!Mapping.AbilityClass) continue;

		Mapping.InputID = MappingIndex;

		FGameplayAbilitySpec AbilitySpec(Mapping.AbilityClass, DefaultAbilityLevel, Mapping.InputID, this);

		//AbilitySpecへ起動元の入力タグを記録
		if (Mapping.InputTag.IsValid())
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(Mapping.InputTag);
		}

		m_AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

//常時保持するAbilityを登録
void APlayerCharacter::GivePassiveStartupAbilities()
{
	if (!m_AbilitySystemComponent) return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : m_PassiveStartupAbilities)
	{
		if (!AbilityClass) continue;

		m_AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, DefaultAbilityLevel, INDEX_NONE, this));
	}
}

//移動Abilityを再起動
void APlayerCharacter::ReactivateMoveAbility()
{
	if (!m_AbilitySystemComponent || !m_MoveAbilityClass) return;

	m_AbilitySystemComponent->TryActivateAbilityByClass(m_MoveAbilityClass);

	//再起動された移動AbilityのPrimaryInstanceを取得
	for (FGameplayAbilitySpec& AbilitySpec : m_AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability) continue;
		if (!AbilitySpec.Ability->GetClass()->IsChildOf(m_MoveAbilityClass)) continue;

		m_ActiveMoveAbility = Cast<UGA_Move>(AbilitySpec.GetPrimaryInstance());
		return;
	}
}

//Enhanced InputのMappingContextを登録
void APlayerCharacter::RegisterInputMappingContext()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!InputSubsystem || !m_InputMapping) return;

	InputSubsystem->AddMappingContext(m_InputMapping, 0);
}

//ワールドとComponentのイベントを登録
void APlayerCharacter::BindWorldEvents()
{
	UWorld* World = GetWorld();
	if (!World) return;

	//ボス戦の開始と終了を受け取る
	if (UGameplayAreaEventManager* EventManager = World->GetSubsystem<UGameplayAreaEventManager>())
	{
		EventManager->OnBossActive.AddDynamic(this, &APlayerCharacter::OnBossBattleStarted);
		EventManager->OnBossDead.AddDynamic(this, &APlayerCharacter::OnBossBattleEnded);
	}

	//ボス演出終了後に操作を復帰
	if (USequenceWorldSubsystem* SequenceSubsystem = World->GetSubsystem<USequenceWorldSubsystem>())
	{
		SequenceSubsystem->OnBossSequenceFinished.AddDynamic(this, &APlayerCharacter::EnableControl);
	}

	//攻撃ヒットと電力状態変更を監視
	if (m_HitJudgmentComponent)
	{
		m_HitJudgmentComponent->OnAttackHit.AddUniqueDynamic(this, &APlayerCharacter::OnAttackHit);
	}

	if (m_ElectroGaugeComp)
	{
		m_ElectroGaugeComp->OnElectroStateChanged.AddUniqueDynamic(this, &APlayerCharacter::HandleElectroStateChanged);
		m_bIsOverChargeMode = m_ElectroGaugeComp->IsOvercharge();
	}
}

//Enhanced Inputをバインド
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	RegisterInputMappingContext();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent) return;

	//移動と視点入力を登録
	if (m_MoveAction)
	{
		EnhancedInputComponent->BindAction(m_MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::MoveInput);
		EnhancedInputComponent->BindAction(m_MoveAction, ETriggerEvent::Completed, this, &APlayerCharacter::MoveInput);
	}

	if (m_LookAction)
	{
		EnhancedInputComponent->BindAction(m_LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::LookInput);
	}

	//スキル選択入力を登録
	if (m_SkillSelectAction)
	{
		EnhancedInputComponent->BindAction(m_SkillSelectAction, ETriggerEvent::Triggered, this, &APlayerCharacter::SkillSelectInput);
		EnhancedInputComponent->BindAction(m_SkillSelectAction, ETriggerEvent::Completed, this, &APlayerCharacter::SkillSelectReleased);
	}

	//スプライン移動開始入力を登録
	if (m_SplineMoveAction)
	{
		EnhancedInputComponent->BindAction(m_SplineMoveAction, ETriggerEvent::Started, this, &APlayerCharacter::SplineMoveInput);
	}

	//Abilityごとの押下と離し入力を登録
	for (int32 MappingIndex = 0; MappingIndex < ActionInputMappings.Num(); ++MappingIndex)
	{
		FPlayerAbilityInputMapping& Mapping = ActionInputMappings[MappingIndex];
		if (!Mapping.InputAction) continue;

		Mapping.InputID = MappingIndex;

		EnhancedInputComponent->BindAction(Mapping.InputAction, ETriggerEvent::Started, this, &APlayerCharacter::HandleActionPressed, Mapping.InputID);

		if (Mapping.bNotifyInputReleased)
		{
			EnhancedInputComponent->BindAction(Mapping.InputAction, ETriggerEvent::Completed, this, &APlayerCharacter::HandleActionReleased, Mapping.InputID);
		}
	}
}

//移動入力を処理
void APlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();

	//攻撃方向参照用に最新の入力値を保存
	m_LastMoveInputVector = InputVector;

	if (m_bInputLocked)
	{
		//入力ロック中は移動Abilityへゼロ入力を渡す
		if (m_ActiveMoveAbility)
		{
			m_ActiveMoveAbility->UpdateMoveInput(FVector2D::ZeroVector);
		}

		return;
	}

	if (m_ActiveMoveAbility)
	{
		m_ActiveMoveAbility->UpdateMoveInput(InputVector);
	}
}

//視点入力を処理
void APlayerCharacter::LookInput(const FInputActionValue& Value)
{
	if (m_bInputLocked) return;

	const FVector2D InputAxis = Value.Get<FVector2D>();
	if (InputAxis.IsNearlyZero()) return;

	if (m_CameraRigComponent)
	{
		m_CameraRigComponent->NotifyCameraInput();
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	const bool bIsGamepad = PlayerController && PlayerController->IsInputKeyDown(EKeys::Gamepad_RightX);

	//デバイスごとの基本感度を取得
	const float DeviceSensitivity = bIsGamepad ? m_GamepadSensitivity : m_MouseSensitivity;

	float SettingSensitivity = 1.f;
	float InvertXMultiplier = 1.f;
	float InvertYMultiplier = 1.f;

	if (m_PlayerSettingsSubsystem)
	{
		//設定画面の値を毎入力時に参照するため即時反映される
		SettingSensitivity = m_PlayerSettingsSubsystem->GetCameraSensitivity();
		InvertXMultiplier = m_PlayerSettingsSubsystem->GetInvertCameraX() ? -1.f : 1.f;
		InvertYMultiplier = m_PlayerSettingsSubsystem->GetInvertCameraY() ? -1.f : 1.f;
	}

	const float FinalSensitivity = DeviceSensitivity * SettingSensitivity;

	AddControllerYawInput(InputAxis.X * FinalSensitivity * InvertXMultiplier);
	AddControllerPitchInput(InputAxis.Y * FinalSensitivity * InvertYMultiplier);
}


//スキル選択入力を処理
void APlayerCharacter::SkillSelectInput(const FInputActionValue& Value)
{
	if (m_bInputLocked || !m_SkillComp) return;

	m_SkillComp->InputSkillSelect(Value);
}

//スキル選択入力解除を処理
void APlayerCharacter::SkillSelectReleased()
{
	if (!m_SkillComp) return;

	m_SkillComp->InputSkillReleased();
}

//スプライン移動入力を処理
void APlayerCharacter::SplineMoveInput()
{
	if (!m_CurrentSplineMoveActor.IsValid()) return;

	m_CurrentSplineMoveActor->RequestStartSplineMove(this);
}

//アクション押下入力を処理
void APlayerCharacter::HandleActionPressed(const FInputActionValue&, int32 InputID)
{
	if (m_bInputLocked) return;
	if (!m_AbilitySystemComponent) return;
	if (InputID == INDEX_NONE) return;

	const FGameplayTag ActionBlockedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Input.ActionBlocked"), false);

	//アクション禁止中は新しい入力を受け付けない
	if (ActionBlockedTag.IsValid() && m_AbilitySystemComponent->HasMatchingGameplayTag(ActionBlockedTag))
	{
		return;
	}

	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecByInputID(InputID);
	if (!AbilitySpec) return;

	//GASへ押下状態を必ず通知する
	//これを行わないとWaitInputReleaseへ離し通知が届かない
	m_AbilitySystemComponent->AbilityLocalInputPressed(InputID);

	//押下通知によって起動しなかった場合だけ明示的に起動する
	if (!AbilitySpec->IsActive())
	{
		m_AbilitySystemComponent->TryActivateAbility(AbilitySpec->Handle);
	}
}


//アクション離し入力を処理
void APlayerCharacter::HandleActionReleased(const FInputActionValue&, int32 InputID)
{
	if (!m_AbilitySystemComponent || InputID == INDEX_NONE) return;

	UE_LOG(LogTemp, Warning, TEXT("[ActionReleased] InputID:%d"), InputID);

	m_AbilitySystemComponent->AbilityLocalInputReleased(InputID);
}

//InputIDからAbilitySpecを取得
FGameplayAbilitySpec* APlayerCharacter::FindAbilitySpecByInputID(int32 InputID) const
{
	if (!m_AbilitySystemComponent) return nullptr;

	for (FGameplayAbilitySpec& AbilitySpec : m_AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.InputID == InputID)
		{
			return &AbilitySpec;
		}
	}

	return nullptr;
}

//ダメージAbilityを起動
bool APlayerCharacter::ActivateDamageAbility()
{
	return EnsureAbilityAndActivate(m_DamageAbilityClass);
}

//死亡Abilityを起動
bool APlayerCharacter::ActivateDeathAbility()
{
	//Blueprintで登録し忘れた場合も実行できるように付与を保証
	return EnsureAbilityAndActivate(m_DeathAbilityClass);
}

//Abilityを必要に応じて付与して起動
bool APlayerCharacter::EnsureAbilityAndActivate(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!m_AbilitySystemComponent || !AbilityClass) return false;

	bool bHasAbilitySpec = false;

	//すでに同じAbilityが付与されているか確認
	for (const FGameplayAbilitySpec& AbilitySpec : m_AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability) continue;

		if (AbilitySpec.Ability->GetClass()->IsChildOf(AbilityClass))
		{
			bHasAbilitySpec = true;
			break;
		}
	}

	//未付与の場合だけ新しいAbilitySpecを追加
	if (!bHasAbilitySpec)
	{
		m_AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, DefaultAbilityLevel, INDEX_NONE, this));
	}

	return m_AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
}

//AbilitySystemComponentを取得
UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return m_AbilitySystemComponent;
}

//ダメージを受け取る
void APlayerCharacter::TakeDamage(const FDamageInfo& DamageInfo)
{
	if (m_bIsDead || m_bDebugGodMode) return;

	//被ダメージ無敵中はダメージ処理を行わない
	if (m_bDamageInvincible || m_bJustEvasiveInvincible)
	{
		return;
	}

	//ダメージ確定前にジャスト回避を判定
	if (m_EvasiveComp)
	{
		AActor* AttackActor = DamageInfo.AttackActor;

		if (IsValid(AttackActor))
		{
			RegisterJustEvasiveAttacker(AttackActor);
		}

		if (m_EvasiveComp->GetIsJustEvasive())
		{
			return;
		}

		if (m_EvasiveComp->TryHandleJustEvasiveBeforeDamage(AttackActor))
		{
			return;
		}
	}

	//HPへ最終ダメージを適用
	ApplyPlayerDamage(DamageInfo.Damage);
	OnPlayerDamaged.Broadcast(this, DamageInfo);

	//HPがなくなった場合は死亡処理へ移行
	if (GetCurrentHP() <= 0.f)
	{
		m_bBossBattleActive = false;
		m_bFallDie = false;

		PlayerDying();

		if (!ActivateDeathAbility())
		{
			PlayerDied();
		}

		return;
	}

	//生存時は無敵、ノックバック、ダメージAbilityを開始
	StartDamageInvincible();
	ApplyKnockBack(DamageInfo);
	ActivateDamageAbility();
}

//HPへダメージを適用
void APlayerCharacter::ApplyPlayerDamage(float DamageAmount)
{
	//オーバーチャージ中は設定倍率で被ダメージを軽減
	const float FinalDamage = IsOverChargeMode() ? DamageAmount * m_OverchargeDamageRate : DamageAmount;

	SetHealthValue(GetCurrentHP() - FinalDamage);
}

//HPを指定値に設定
void APlayerCharacter::SetHealthValue(float NewHealth)
{
	if (!m_PlayerAttributeSet) return;

	const float ClampedHealth = FMath::Clamp(NewHealth, 0.f, GetMaxHP());
	m_PlayerAttributeSet->SetHealth(ClampedHealth);
}

//ノックバックを適用
void APlayerCharacter::ApplyKnockBack(const FDamageInfo& DamageInfo)
{
	if (DamageInfo.KnockbackDirection.IsNearlyZero()) return;
	if (DamageInfo.KnockbackScale <= 0.f) return;

	const FVector KnockBackVelocity = DamageInfo.KnockbackDirection.GetSafeNormal() * DamageInfo.KnockbackScale * m_KnockBackPowerMultiplier;

	LaunchCharacter(KnockBackVelocity, true, true);
}

//攻撃ヒット時の状態を更新
void APlayerCharacter::OnAttackHit(const AActor* HitActor)
{
	m_bIsHit = true;

	//命中した敵を次の攻撃対象として保持
	if (const AEnemyBase* HitEnemy = Cast<AEnemyBase>(HitActor))
	{
		if (m_AttackTargetComponent)
		{
			m_AttackTargetComponent->SetLockedAttackTarget(HitEnemy);
		}
	}
}

//攻撃ヒット状態をリセット
void APlayerCharacter::ClearAttackHitState()
{
	m_bIsHit = false;
}

//被ダメージ後の無敵時間を開始
void APlayerCharacter::StartDamageInvincible()
{
	m_bDamageInvincible = true;

	GetWorldTimerManager().SetTimer(m_DamageInvincibleTimerHandle, this, &APlayerCharacter::OnDamageInvincibleEnd, m_DamageInvincibleDuration, false);
}

//被ダメージ後の無敵時間を終了
void APlayerCharacter::OnDamageInvincibleEnd()
{
	m_bDamageInvincible = false;
	m_bIsDamage = false;

	//ダメージ状態のままなら通常状態へ戻す
	if (!m_bIsDead && m_CurrentActionState == EPlayerActionState::Damaged)
	{
		SetActionState(EPlayerActionState::None);
	}

	GetWorldTimerManager().ClearTimer(m_DamageInvincibleTimerHandle);
}

//やられ状態を開始
void APlayerCharacter::PlayerDying()
{
	if (m_bIsDead) return;

	m_bIsDead = true;
	m_bBossBattleActive = false;

	SetActionState(EPlayerActionState::Dead);
	SetInputLocked(true);

	//死亡演出用カメラへ切り替える
	if (m_CameraRigComponent)
	{
		m_CameraRigComponent->RequestDieCamera();
	}

	//通常死亡時は移動を完全に停止
	if (!m_bFallDie)
	{
		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}
	}

	//外部Subsystemへやられ状態開始を通知
	if (UPlayerNotifySubsystem* NotifySubsystem = FindPlayerNotifySubsystem())
	{
		NotifySubsystem->NotifyPlayerDying(this);
	}

	OnPlayerDying.Broadcast(this);
	OnPlayerDieBP(m_bFallDie);
}

//死亡確定処理を実行
void APlayerCharacter::PlayerDied()
{
	//外部Subsystemへ死亡確定を通知
	if (UPlayerNotifySubsystem* NotifySubsystem = FindPlayerNotifySubsystem())
	{
		NotifySubsystem->NotifyPlayerDied(this);
	}

	OnPlayerDied.Broadcast(this);

	//GameModeへリスポーンを要求
	UWorld* World = GetWorld();
	if (!World) return;

	if (AGameMode_Ingame* GameMode = Cast<AGameMode_Ingame>(World->GetAuthGameMode()))
	{
		GameMode->RequestRespawn(this);
	}
}

//リスポーン時の状態を初期化
void APlayerCharacter::OnRespawn()
{
	//残っているAbilityをすべて停止
	if (m_AbilitySystemComponent)
	{
		m_AbilitySystemComponent->CancelAllAbilities();
	}

	ResetTransientState();
	SetHealthValue(GetMaxHP());

	//各機能ComponentのRuntime状態を初期化
	if (m_AttackComp)
	{
		m_AttackComp->ResetAttack();
	}

	if (m_EvasiveComp)
	{
		m_EvasiveComp->ResetEvasiveState();
	}

	if (m_SkillComp)
	{
		m_SkillComp->ResetSkill();
	}

	if (m_ElectroGaugeComp)
	{
		m_ElectroGaugeComp->ResetGauge();
	}

	if (m_CustomMovementComponent)
	{
		m_CustomMovementComponent->ResetMovementState();
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	if (m_CameraRigComponent)
	{
		m_CameraRigComponent->ResetCameraState();
	}

	//常駐する移動Abilityを再起動
	ReactivateMoveAbility();

	OnPlayerRespawnBP();
	OnPlayerRespawn.Broadcast(this);
}

//リスポーン時に一時状態を初期化
void APlayerCharacter::ResetTransientState()
{
	m_bIsDead = false;
	m_bFallDie = false;
	m_bIsDamage = false;
	m_bIsHit = false;
	m_bIsEnhancedAttack = false;
	m_bDamageInvincible = false;
	m_bJustEvasiveInvincible = false;
	m_bInputLocked = false;
	m_bBossBattleActive = false;
	m_bIsOverChargeMode = false;

	SetActionState(EPlayerActionState::None);
	ClearJustEvasiveAttacker();

	//リスポーン前に残っているタイマーを停止
	GetWorldTimerManager().ClearTimer(m_DamageInvincibleTimerHandle);
	GetWorldTimerManager().ClearTimer(m_JustEvasiveInvincibleTimerHandle);
}

//ジャスト回避後の無敵時間を開始
void APlayerCharacter::StartJustEvasiveInvincible()
{
	m_bJustEvasiveInvincible = true;

	if (!GetWorld()) return;

	//連続成功時は既存タイマーを更新
	GetWorldTimerManager().ClearTimer(m_JustEvasiveInvincibleTimerHandle);
	GetWorldTimerManager().SetTimer(m_JustEvasiveInvincibleTimerHandle, this, &APlayerCharacter::OnJustEvasiveInvincibleEnd, m_JustEvasiveInvincibleDuration, false);
}

//ジャスト回避後の無敵時間を終了
void APlayerCharacter::OnJustEvasiveInvincibleEnd()
{
	m_bJustEvasiveInvincible = false;

	GetWorldTimerManager().ClearTimer(m_JustEvasiveInvincibleTimerHandle);
}

//電力状態変更を受け取る
void APlayerCharacter::HandleElectroStateChanged(EElectroState PreviousState, EElectroState NewState)
{

	m_bIsOverChargeMode = NewState == EElectroState::Overcharge;

	if (!GEngine) return;

	GEngine->AddOnScreenDebugMessage(
		-1,
		m_DebugMessageDuration,
		m_bIsOverChargeMode ? FColor::Cyan : FColor::Red,
		m_bIsOverChargeMode ? TEXT("OverCharge: ON") : TEXT("OverCharge: OFF")
	);
}

//オーバーチャージ状態か確認
bool APlayerCharacter::IsOverChargeMode() const
{
	return m_bIsOverChargeMode;
}

//ワールド外落下時に死亡処理を開始
void APlayerCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	if (m_bIsDead) return;

	m_bFallDie = true;
	PlayerDying();
}

//接地状態を取得
bool APlayerCharacter::GetOnGround() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	return MovementComponent && MovementComponent->IsMovingOnGround();
}

//現在HPを取得
float APlayerCharacter::GetCurrentHP() const
{
	return m_PlayerAttributeSet ? m_PlayerAttributeSet->GetHealth() : 0.f;
}

//最大HPを取得
float APlayerCharacter::GetMaxHP() const
{
	return m_PlayerAttributeSet ? m_PlayerAttributeSet->GetMaxHealth() : DefaultMaxHealth;
}

//現在HPの割合を取得
float APlayerCharacter::GetHealthPercentage() const
{
	const float MaxHealth = GetMaxHP();
	if (MaxHealth <= 0.f) return 0.f;

	return GetCurrentHP() / MaxHealth;
}

//HPを回復
void APlayerCharacter::AddHealth(float HealAmount)
{
	SetHealthValue(GetCurrentHP() + HealAmount);
}

//アクション状態を設定
void APlayerCharacter::SetActionState(EPlayerActionState NewState)
{
	if (m_CurrentActionState == NewState) return;

	const EPlayerActionState PreviousState = m_CurrentActionState;
	m_CurrentActionState = NewState;

	OnActionStateChanged.Broadcast(PreviousState, NewState);
}

//ジャスト回避対象を登録
void APlayerCharacter::RegisterJustEvasiveAttacker(const AActor* Attacker)
{
	m_JustEvasive_Attacker = Attacker;

	if (!GetWorld()) return;

	//対象が有効か一定間隔で監視
	GetWorldTimerManager().SetTimer(
		m_JustEvasiveTargetCheckTimerHandle,
		this,
		&APlayerCharacter::CheckJustEvasiveTargetValidity,
		m_JustEvasiveTargetCheckInterval,
		true
	);
}

//ジャスト回避対象をクリア
void APlayerCharacter::ClearJustEvasiveAttacker()
{
	m_JustEvasive_Attacker = nullptr;

	GetWorldTimerManager().ClearTimer(m_JustEvasiveTargetCheckTimerHandle);
}

//ジャスト回避対象の有効性を確認
void APlayerCharacter::CheckJustEvasiveTargetValidity()
{
	if (!m_JustEvasive_Attacker.IsValid())
	{
		ClearJustEvasiveAttacker();
		return;
	}

	//ボス戦中は距離による解除を行わない
	if (m_bBossBattleActive) return;

	const AActor* TargetActor = m_JustEvasive_Attacker.Get();

	//死亡中の敵は反撃対象から外す
	if (const AEnemyBase* TargetEnemy = Cast<AEnemyBase>(TargetActor))
	{
		if (TargetEnemy->GetIsDying())
		{
			ClearJustEvasiveAttacker();
			return;
		}
	}

	//設定距離を超えた対象は解除
	if (GetDistanceTo(TargetActor) > m_JustEvasiveTargetKeepDistance)
	{
		ClearJustEvasiveAttacker();
	}
}

//ジャスト回避成功を通知
void APlayerCharacter::BroadcastJustEvasiveSuccess(const AActor* Attacker)
{
	RegisterJustEvasiveAttacker(Attacker);
	SetActionState(EPlayerActionState::JustEvasiveCounter);

	//反撃開始直後の重複被弾を防ぐ
	StartJustEvasiveInvincible();

	if (m_CustomMovementComponent)
	{
		m_CustomMovementComponent->StartJustEvasiveCounterMove(Attacker);
	}

	if (m_CameraRigComponent)
	{
		m_CameraRigComponent->RequestJustEvasiveLookCamera(const_cast<AActor*>(Attacker), false);
	}

	//UIや演出側へジャスト回避成功を通知
	if (UPlayerNotifySubsystem* NotifySubsystem = FindPlayerNotifySubsystem())
	{
		NotifySubsystem->NotifyJustEvasive(Attacker);
	}
}

//入力ロック状態を設定
void APlayerCharacter::SetInputLocked(bool bLock)
{
	m_bInputLocked = bLock;

	if (GEngine)
	{
		const FString Message = FString::Printf(TEXT("SetInputLocked Called: %s"), bLock ? TEXT("true") : TEXT("false"));
		GEngine->AddOnScreenDebugMessage(-1, 10.f, bLock ? FColor::Red : FColor::Green, Message);
	}

	if (!bLock) return;

	//入力ロック時は現在の移動を停止
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (m_ActiveMoveAbility)
	{
		m_ActiveMoveAbility->UpdateMoveInput(FVector2D::ZeroVector);
	}
}

//プレイヤーの一時停止状態を切り替える
void APlayerCharacter::PausePlayer(bool bPause)
{
	//移動Componentを停止または再開
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		if (bPause)
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->SetMovementMode(MOVE_None);
			MovementComponent->SetComponentTickEnabled(false);
		}
		else
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
			MovementComponent->SetComponentTickEnabled(true);
		}
	}

	//Montageと通常アニメーションを停止または再開
	if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
	{
		if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
		{
			if (bPause)
			{
				AnimInstance->Montage_Pause(nullptr);
			}
			else
			{
				AnimInstance->Montage_Resume(nullptr);
			}
		}

		SkeletalMesh->GlobalAnimRateScale = bPause ? 0.f : 1.f;
	}

	//停止中はクリア演出カメラを使用
	if (m_CameraRigComponent)
	{
		if (bPause)
		{
			m_CameraRigComponent->RequestClearCamera();
		}
		else
		{
			m_CameraRigComponent->ResetCameraState();
		}
	}
}

//アクション確定時の共通状態を更新
void APlayerCharacter::OnActionCommitted(EPlayerActionCommit Action)
{
	switch (Action)
	{
	case EPlayerActionCommit::LightAttack:
	case EPlayerActionCommit::OtherAttack:
		SetActionState(EPlayerActionState::Attack);
		break;

	case EPlayerActionCommit::Jump:
		SetActionState(EPlayerActionState::Jump);
		SetIsEnhancedAttack(false);
		break;

	case EPlayerActionCommit::Evasive:
		SetActionState(EPlayerActionState::Evasive);
		SetIsEnhancedAttack(false);
		break;

	case EPlayerActionCommit::Move:
		SetActionState(EPlayerActionState::Move);
		SetIsEnhancedAttack(false);
		break;

	case EPlayerActionCommit::Skill:
		SetActionState(EPlayerActionState::Skill);
		break;

	case EPlayerActionCommit::Damaged:
		SetActionState(EPlayerActionState::Damaged);
		SetIsEnhancedAttack(false);
		break;

	default:
		SetIsEnhancedAttack(false);
		break;
	}
}

//現在触れているスプライン移動Actorを設定
void APlayerCharacter::SetCurrentSplineMoveActor(APlayer_SplineMove* Actor)
{
	m_CurrentSplineMoveActor = Actor;
}

//スプライン移動開始を通知
void APlayerCharacter::NotifySplineMoveStarted()
{
	if (m_CameraRigComponent)
	{
		m_CameraRigComponent->RequestSplineMoveCamera();
	}
}

//スプライン移動終了を通知
void APlayerCharacter::NotifySplineMoveFinished()
{
	if (m_CameraRigComponent)
	{
		m_CameraRigComponent->ClearCameraRequest();
	}
}

//オープニングカメラを要求
void APlayerCharacter::OpeningCamera()
{
	if (m_CameraRigComponent)
	{
		m_CameraRigComponent->RequestOpeningCamera();
	}

	OnOpeningCameraRequested.Broadcast(this);
}

//プレイヤーコリジョンを復帰
void APlayerCharacter::RevivalCollision()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

//プレイヤーコリジョンを一時無効化
void APlayerCharacter::DeleteCollision()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

//操作と移動を無効化
void APlayerCharacter::DisableControl()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("DisableControl Called"));
	}

	SetInputLocked(true);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
	}
}

//操作と移動を復帰
void APlayerCharacter::EnableControl()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("EnableControl Called"));
	}

	SetInputLocked(false);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}
}

//ボス戦開始時の処理
void APlayerCharacter::OnBossBattleStarted()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("OnBossBattleStarted Called"));
	}

	m_bBossBattleActive = true;
	DisableControl();
}

//ボス戦終了時の処理
void APlayerCharacter::OnBossBattleEnded()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("OnBossBattleEnded Called"));
	}

	m_bBossBattleActive = false;
}

//HP無限状態を切り替える
void APlayerCharacter::Debug_ToggleInfiniteHP()
{
	m_bDebugGodMode = !m_bDebugGodMode;

	if (!GEngine) return;

	const FColor MessageColor = m_bDebugGodMode ? FColor::Cyan : FColor::Red;
	const FString Message = m_bDebugGodMode ? TEXT("GOD MODE: ON") : TEXT("GOD MODE: OFF");

	GEngine->AddOnScreenDebugMessage(-1, m_DebugMessageDuration, MessageColor, Message);
}

//オーバーチャージ状態を切り替える
void APlayerCharacter::Debug_ToggleOverCharge()
{
	if (!m_ElectroGaugeComp) return;

	m_ElectroGaugeComp->DebugToggleOverCharge();
}

//プレイヤー死亡を実行
void APlayerCharacter::Debug_TogglePlayerDie()
{
	PlayerDying();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, m_DebugMessageDuration, FColor::Cyan, TEXT("Debug PlayerDie"));
	}
}

//ボスイベント位置へワープ
void APlayerCharacter::Debug_WarpBossEvent()
{
	TeleportTo(m_DebugBossWarpLocation, GetActorRotation());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, m_DebugMessageDuration, FColor::Cyan, TEXT("Debug BossWarp"));
	}
}

//プレイヤー通知Subsystemを取得
UPlayerNotifySubsystem* APlayerCharacter::FindPlayerNotifySubsystem() const
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return nullptr;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return nullptr;

	return LocalPlayer->GetSubsystem<UPlayerNotifySubsystem>();
}
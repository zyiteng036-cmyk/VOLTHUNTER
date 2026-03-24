//PlayerCharacter.Cpp

#include "PlayerCharacter.h"
#include "GameMode/GameMode_Ingame.h"
#include "SequenceWorldSubsystem/SequenceWorldSubsystem.h"
#include "PlayerNotifySubSystem/PlayerNotifySubsystem.h"
#include "PlayerComponent/Player_MovementComponent.h"
#include "PlayerComponent/Player_CameraComponent.h"
#include "PlayerComponent/Player_EvasiveComponent.h"
#include "PlayerComponent/Player_AttackComponent.h"
#include "PlayerComponent/Player_ElectroGaugeComponent.h"
#include "PlayerComponent/Player_SkillComponent.h"
#include "PlayerSplineMove/Player_SplineMove.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HitJudgmentComponent/HitJudgmentComponent.h"
#include "Engine/World.h"
#include "PlayerController/MainPlayerController.h"
#include "TimerManager.h"
#include "Event/GameplayAreaEventManager/GameplayAreaEventManager.h"

//無名名前空間
namespace {
	constexpr float FallingLateralFriction = 0.f;
}



// コンストラクタ
APlayerCharacter::APlayerCharacter()
	: m_IsBossBattleActive(false)
	, m_MovementComponent(nullptr)
	, m_CameraComponent(nullptr)
	, m_EvasiveComponent(nullptr)
	, m_AttackComponent(nullptr)
	, m_ElectroGaugeComponent(nullptr)
	, m_SkillComponent(nullptr)
	, m_BufferedNextAbility(nullptr)
	, m_AbilityPlayer_AttackLight01(nullptr)
	, m_AbilityPlayer_AttackLight02(nullptr)
	, m_AbilityPlayer_AttackLight03(nullptr)
	, m_AbilityPlayer_AttackHeavy(nullptr)
	, m_AbilityPlayer_AttackHeavyCharge(nullptr)
	, m_AbilityPlayer_AirAttackLight(nullptr)
	, m_AbilityPlayer_AirAttackHeavy(nullptr)
	, m_AbilityPlayer_Evasive(nullptr)
	, m_AbilityPlayer_BackEvasive(nullptr)
	, m_AbilityPlayer_Damage(nullptr)
	, m_AbilityPlayer_Die(nullptr)
	, m_AbilityPlayer_Skill01(nullptr)
	, m_HitJudgmentComponent(nullptr)
	, m_SpringArm(nullptr)
	, m_Camera(nullptr)
	, m_CurrentSplineMoveActor(nullptr)
	, m_InputMapping(nullptr)
	, m_MoveForwardAction(nullptr)
	, m_MoveRightAction(nullptr)
	, m_CameraRotatePitch(nullptr)
	, m_CameraRotateYaw(nullptr)
	, m_JumpAction(nullptr)
	, m_DashAction(nullptr)
	, m_CameraResetAction(nullptr)
	, m_TargetLockOnAction(nullptr)
	, m_AttackLightAction(nullptr)
	, m_AttackHeavyAction(nullptr)
	, m_TargetChageAction(nullptr)
	, m_SkillSelectAction(nullptr)
	, m_SkillActiveAction(nullptr)
	, m_JustEvasive_Attacker(nullptr)
	, m_PlayerHP(PlayerParam.MaxHP)
	, m_Invincible(false)
	, m_InvincibleDuration(0.f)
	, m_IsDie(false)
	, m_IsFallDie(false)
	, m_IsHit(false)
	, m_IsEnhancedAttack(false)
	, m_IsDebugGodMode(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// カメラ回転で自動的にプレイヤーは向かない
	// カメラ操作とキャラクターの向きを分離するため、
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;


	// 移動入力方向へキャラクターを自動回転させる
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	//スプリングアーム設定
	m_SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_SpringArm->SetupAttachment(GetMesh(),TEXT("Spine2"));
	m_SpringArm->bUsePawnControlRotation = true;
	//視点の高さ
	m_SpringArm->SocketOffset = PlayerParam.CameraSocketOffset;
	//pawnは無視
	m_SpringArm->ProbeChannel = ECC_Visibility;

	//カメラ設定
	m_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_Camera->SetupAttachment(m_SpringArm);
	m_Camera->bUsePawnControlRotation = false;

	// ロックオン用スプリングアーム
	m_LockOnSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("LockOnSpringArm"));
	m_LockOnSpringArm->SetupAttachment(GetCapsuleComponent());
	m_LockOnSpringArm->TargetArmLength = 350.0f;
	m_LockOnSpringArm->SetRelativeLocation(FVector(0.0f, -70.0f, 140.0f)); // 少し上にオフセット
	m_LockOnSpringArm->bEnableCameraLag = true;
	m_LockOnSpringArm->CameraLagSpeed = 5.0f;
	m_LockOnSpringArm->bUsePawnControlRotation = false; // ロックオン時は制御しない
	m_LockOnSpringArm->ProbeChannel = ECC_Visibility;

	//ロックオン用カメラ
	m_LockOnCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("LockOnCamera"));
	m_LockOnCamera->SetupAttachment(m_LockOnSpringArm, USpringArmComponent::SocketName);
	m_LockOnCamera->bUsePawnControlRotation = false;
	m_LockOnCamera->SetActive(false);




	//AbilitySystemComponentの生成
	m_AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	//プレイヤー設定
	//落下中の減速無し
	GetCharacterMovement()->FallingLateralFriction = FallingLateralFriction;

	m_MovementComponent = CreateDefaultSubobject<UPlayer_MovementComponent>(TEXT("PlayerMovementComponent"));

	m_CameraComponent = CreateDefaultSubobject<UPlayer_CameraComponent>(TEXT("PlayerCameraComponent"));

	m_EvasiveComponent = CreateDefaultSubobject<UPlayer_EvasiveComponent>(TEXT("PlayerEvasiveComponent"));

	m_AttackComponent = CreateDefaultSubobject<UPlayer_AttackComponent>(TEXT("PlayerAttackComponent"));

	m_ElectroGaugeComponent = CreateDefaultSubobject<UPlayer_ElectroGaugeComponent>(TEXT("PlayerElectroGaugeComponent"));

	m_HitJudgmentComponent = CreateDefaultSubobject<UHitJudgmentComponent>(TEXT("HitJudgmentComponent"));

	m_SkillComponent = CreateDefaultSubobject<UPlayer_SkillComponent>(TEXT("PlayerSkillComponent"));

}

//スタート時、生成時
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();



	//移動速度
	GetCharacterMovement()->MaxWalkSpeed = PlayerParam.RunSpeed;
	GetCharacterMovement()->GravityScale = PlayerParam.GravityScale;
	//空中操作
	GetCharacterMovement()->AirControl = PlayerParam.AirControl;


	//Enhanced Inputをプレイヤーに割り当てる
	//キャラクターを操作しているコントローラーを取得
	m_PlayerController = Cast<APlayerController>(GetController());
	if (!m_PlayerController.IsValid())return;

	ULocalPlayer* LocalPlayer = m_PlayerController->GetLocalPlayer();
	if (!LocalPlayer)return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!InputSubsystem)return;

	//入力設定、登録
	InputSubsystem->AddMappingContext(m_InputMapping, 0);

	if (!m_AbilitySystemComponent) return;

	// ===== プレイヤーアビリティ登録 =====
	//全アビリティは起動のみを GAS に
	//コンボ順や条件分岐は各 Ability 内で制御する
	//弱１
	if (m_AbilityPlayer_AttackLight01)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_AttackLight01, 1, INDEX_NONE, this));
	}
	//弱２
	if (m_AbilityPlayer_AttackLight02)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_AttackLight02, 1, INDEX_NONE, this));
	}
	//弱３
	if (m_AbilityPlayer_AttackLight03)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_AttackLight03, 1, INDEX_NONE, this));
	}

	//攻撃強ロング
	if (m_AbilityPlayer_AttackHeavy)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_AttackHeavy, 1, INDEX_NONE, this));
	}

	//強攻撃チャージ
	if (m_AbilityPlayer_AttackHeavyCharge)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_AttackHeavyCharge, 1, INDEX_NONE, this));
	}


	//空中攻撃弱１
	if (m_AbilityPlayer_AirAttackLight)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_AirAttackLight, 1, INDEX_NONE, this));
	}

	//空中攻撃強
	if (m_AbilityPlayer_AirAttackHeavy)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_AirAttackHeavy, 1, INDEX_NONE, this));
	}


	//回避
	if (m_AbilityPlayer_Evasive)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_Evasive, 1, INDEX_NONE, this));
	}
	//バック回避
	if (m_AbilityPlayer_BackEvasive)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_BackEvasive, 1, INDEX_NONE, this));
	}

	//ダメージアビリティ
	if (m_AbilityPlayer_Damage)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_Damage, 1, INDEX_NONE, this));

	}

	//死亡アビリティ
	if (m_AbilityPlayer_Die)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_Die, 1, INDEX_NONE, this));

	}


	if (m_AbilityPlayer_Skill01)
	{
		m_AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(m_AbilityPlayer_Skill01, 1, INDEX_NONE, this));

	}
	//ヒット判断コンポーネントにヒット時に呼んでもらうメソッドを登録
	m_HitJudgmentComponent->OnAttackHit.AddDynamic(this, &APlayerCharacter::OnAttackHit);

	if (UWorld* World = GetWorld())
	{
		if (UGameplayAreaEventManager* EventManager = World->GetSubsystem<UGameplayAreaEventManager>())
		{
			//ボス戦開始通知を登録
			EventManager->OnBossActive.AddDynamic(this, &APlayerCharacter::OnBossBattleStarted);
			//ボス撃破通知を登録
			EventManager->OnBossDead.AddDynamic(this, &APlayerCharacter::OnBossBattleEnded);
		}
		if (USequenceWorldSubsystem* SequenceSubsystem = World->GetSubsystem<USequenceWorldSubsystem>())
		{
			SequenceSubsystem->OnBossSequenceFinished.AddDynamic(this, &APlayerCharacter::EnableControl);
		}
	}
}

//毎フレーム
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckJustEvasiveTargetValidity();

	_updateInvincibleDuraction(DeltaTime);
}

// 入力バインド
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	//移動
	//前後
	EnhancedInput->BindAction(m_MoveForwardAction, ETriggerEvent::Triggered, m_MovementComponent, &UPlayer_MovementComponent::Input_MoveForward);
	//左右
	EnhancedInput->BindAction(m_MoveRightAction, ETriggerEvent::Triggered, m_MovementComponent, &UPlayer_MovementComponent::Input_MoveRight);

	//ジャンプ
	EnhancedInput->BindAction(m_JumpAction, ETriggerEvent::Started, m_MovementComponent, &UPlayer_MovementComponent::Input_Jump);
	EnhancedInput->BindAction(m_JumpAction, ETriggerEvent::Completed, m_MovementComponent, &UPlayer_MovementComponent::Input_JumpRelease);


	//ダッシュ
	EnhancedInput->BindAction(m_DashAction, ETriggerEvent::Started, m_MovementComponent, &UPlayer_MovementComponent::Input_Dash);

	//カメラ
	EnhancedInput->BindAction(m_CameraRotatePitch, ETriggerEvent::Triggered, m_CameraComponent, &UPlayer_CameraComponent::Input_CameraRotatePitch);
	EnhancedInput->BindAction(m_CameraRotateYaw, ETriggerEvent::Triggered, m_CameraComponent, &UPlayer_CameraComponent::Input_CameraRotateYaw);

	//カメラリセット
	EnhancedInput->BindAction(m_CameraResetAction, ETriggerEvent::Started, m_CameraComponent, &UPlayer_CameraComponent::Input_CameraReset);

	//カメラロックオン
	EnhancedInput->BindAction(m_TargetLockOnAction, ETriggerEvent::Started, m_CameraComponent, &UPlayer_CameraComponent::Input_TargetLockOn);

	//ターゲットを変える
	EnhancedInput->BindAction(m_TargetChageAction, ETriggerEvent::Started, m_CameraComponent, &UPlayer_CameraComponent::Input_TargetChange);

	//回避スウェイ
	EnhancedInput->BindAction(m_EvasiveAction, ETriggerEvent::Started, m_EvasiveComponent, &UPlayer_EvasiveComponent::Input_Evasive);


	//弱攻撃
	EnhancedInput->BindAction(m_AttackLightAction, ETriggerEvent::Started, m_AttackComponent, &UPlayer_AttackComponent::Input_AttackLight);

	//溜め強攻撃
	EnhancedInput->BindAction(m_AttackHeavyAction, ETriggerEvent::Started, m_AttackComponent, &UPlayer_AttackComponent::Input_AttackHeavy);

	//溜め強攻撃離したとき
	EnhancedInput->BindAction(m_AttackHeavyAction, ETriggerEvent::Completed, m_AttackComponent, &UPlayer_AttackComponent::Completed_AttackHeavy);

	//スキル選択
	EnhancedInput->BindAction(m_SkillSelectAction, ETriggerEvent::Triggered, m_SkillComponent, &UPlayer_SkillComponent::Input_SkillSelect);

	//入力を連続で入れないようにするため
	EnhancedInput->BindAction(m_SkillSelectAction, ETriggerEvent::Completed, m_SkillComponent, &UPlayer_SkillComponent::Input_SkillReleased);

	//スキル発動
	EnhancedInput->BindAction(m_SkillActiveAction, ETriggerEvent::Started, m_SkillComponent, &UPlayer_SkillComponent::Input_SkillActive);

	//スプライン移動
	EnhancedInput->BindAction(m_SplineMoveAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_SplineMove);

}

void APlayerCharacter::OnRespawn()
{
	m_IsDie = false;
	m_IsDamage = false;
	m_IsFallDie = false;

	//HPリセット
	m_PlayerHP = PlayerParam.MaxHP;

	GetWorldTimerManager().ClearTimer(m_DamageInvincibleTimerHandle);
	m_Invincible = false;

	OnPlayerRespawnBP();


	//移動復帰
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	//入力復帰
	EnableInput(Cast<APlayerController>(GetController()));

	m_JustEvasive_Attacker = nullptr;
	//攻撃・状態リセット
	m_AttackComponent->ResetAttack();
	m_CameraComponent->ResetCamera();
	m_ElectroGaugeComponent->ResetGauge();

}


//攻撃処理
void APlayerCharacter::TakeDamage(const FDamageInfo& _damageInfo)
{
	//もし死んでいれば処理されない
	if (m_IsDie)return;

	//無敵状態であれば処理しない
	if (m_Invincible)return;


	// ジャスト回避中なら完全無効
	// ジャスト回避成功中はダメージを一切受けない
	if (m_EvasiveComponent && m_EvasiveComponent->GetIsEvasive())
	{
		//敵の位置を取得
		// ジャスト回避成功時に反撃対象として使用
		m_JustEvasive_Attacker = _damageInfo.AttackActor;
		m_InvincibleDuration = 0.f;
		//m_IsEnhancedAttack = true;
		return;
	}

	if (m_IsDebugGodMode)return;


	//ダメージを受ける
	PlayerTakeDamege(_damageInfo.Damage);
	m_IsDamage = true;

	if (m_CameraComponent)
	{
		m_CameraComponent->CameraShakEnd();
	}


	if (m_PlayerHP <= 0)
	{
		PlayerDying();
		return;
	}

	//無敵時間を開始 (連続ヒット防止)
	m_Invincible = true;

	// 無敵時間の長さ（秒）
	float InvincibleDuration = 1.f;

	// タイマーをセット
	GetWorldTimerManager().SetTimer(
		m_DamageInvincibleTimerHandle,
		this,
		&APlayerCharacter::OnDamageInvincibleEnd,
		InvincibleDuration,
		false
	);

	//攻撃を受けたらゲージを減らす
	m_ElectroGaugeComponent->SubtractionElectoroGauge(5.f);

	//強攻撃で攻撃溜めているときはダメージ食らうがヒットモーションは入らない
	//if (m_AttackComponent->GetIsHeavyCharging() || m_AttackComponent->GetHeavyAttackStart())return;

	//攻撃を受けたら攻撃リセット
	m_AttackComponent->ResetAttack();
	//UE_LOG(LogTemp, Warning, TEXT("HP--"));

	if (m_PlayerHP > 0)
	{
		if (m_AbilityPlayer_Damage) {
			m_AbilitySystemComponent->TryActivateAbilityByClass(m_AbilityPlayer_Damage);
			m_AttackComponent->SetCanAttack(false);
		}
	}


	//吹っ飛ばし
	BlowEnemy(_damageInfo.KnockbackDirection, _damageInfo.KnockbackScale);

}

bool APlayerCharacter::GetIsEnhancedAttack() const
{
	return m_IsEnhancedAttack;
}

void APlayerCharacter::PlayerDying()
{
	if (m_IsDie)return;
	m_IsDie = true;

	if (m_IsBossBattleActive)
	{
		m_IsBossBattleActive = false;
	}

	if (m_CameraComponent)
	{
		m_CameraComponent->OnPlayerDie();
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UPlayerNotifySubsystem* Subsystem = LP->GetSubsystem<UPlayerNotifySubsystem>())
			{
				Subsystem->NotifyPlayerDying(this);
			}
		}
	}
	if (OnPlayerDying.IsBound())
	{
		OnPlayerDying.Broadcast(this);
	}

	//入力停止
	DisableInput(Cast<APlayerController>(GetController()));

	//移動停止
	if (!m_IsFallDie) {
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}


	// BP に通知
	OnPlayerDieBP(m_IsFallDie);

	if (m_AbilityPlayer_Die)
	{
		m_AbilitySystemComponent->TryActivateAbilityByClass(m_AbilityPlayer_Die);
	}

}

void APlayerCharacter::PlayerDied()
{
	// サブシステムに今までの「完全に死んだ」通知を送る
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UPlayerNotifySubsystem* Subsystem = LP->GetSubsystem<UPlayerNotifySubsystem>())
			{
				Subsystem->NotifyPlayerDied(this);
			}
		}
	}

	// デリゲート通知
	if (OnPlayerDied.IsBound())
	{
		OnPlayerDied.Broadcast(this);
	}

	if (AGameMode_Ingame* GameMode = Cast<AGameMode_Ingame>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->RequestRespawn(this);
	}
}


void APlayerCharacter::FellOutOfWorld(const UDamageType& dmgType)
{
	// 既に死亡済みなら何もしない
	if (m_IsDie)
	{
		return;
	}
	m_IsFallDie = true;
	PlayerDying();
}

//ノックバック
void APlayerCharacter::BlowEnemy(const FVector& _blowVector, const float _blowScale)
{
	FVector BlowVector = _blowVector.GetSafeNormal() * _blowScale * PlayerParam.GravityScale;

	//キャラクター吹っ飛ばす処理//第一引数吹っ飛ばしベクトル//第二引数水平方向に速度を上書きするが//第三引数垂直方向に速度上書きするか
	LaunchCharacter(BlowVector, true, true);
}

void APlayerCharacter::PlayerTakeDamege(const float& _Damage)
{
	//オーバーチャージ中はダメージ軽減
	if (m_ElectroGaugeComponent->IsOvercharge())
	{
		m_PlayerHP -= _Damage * 0.5;
		return;
	}

	m_PlayerHP -= _Damage;

}

void APlayerCharacter::OnDamageInvincibleEnd()
{
	// 無敵解除
	m_Invincible = false;
	m_IsDamage = false;
	// タイマーハンドルをクリア
	GetWorldTimerManager().ClearTimer(m_DamageInvincibleTimerHandle);
}

void APlayerCharacter::_updateInvincibleDuraction(float DeltaTime)
{
	if (m_Invincible)
	{
		// 無敵中なら時間を加算
		m_InvincibleDuration += DeltaTime;

		// 2.0秒を超えたら強制解除
		if (m_InvincibleDuration > 2.0f)
		{
			OnDamageInvincibleEnd(); // 強制的に無敵解除関数を呼ぶ
		}
	}
	else
	{
		// 無敵じゃないときはタイマーを0にしておく
		m_InvincibleDuration = 0.0f;
	}
}

void APlayerCharacter::AddHealth(float _HealAmount)
{
	m_PlayerHP += _HealAmount;

	//最大HPを超えないように
	if (m_PlayerHP > PlayerParam.MaxHP)
	{
		m_PlayerHP = PlayerParam.MaxHP;
	}
}


UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const {
	return m_AbilitySystemComponent;
}

const FPlayerParam& APlayerCharacter::GetPlayerParam() const
{
	return PlayerParam;
}

//地面についたとき
void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::OnLanded(Hit);


	if (m_MovementComponent->GetIsJump()) {

		m_MovementComponent->OnPlayerLanded();
		m_EvasiveComponent->SetCanEvasive(true);
		//空中強攻撃終了
		m_AttackComponent->AirFallAttackEnd();

		// 着地時に空中コンボ状態を完全にリセットする
		m_AttackComponent->SetIsAirAttackStart(false);
	}
}

void APlayerCharacter::OnAttackHit(const AActor* _hitActor) {
	//ヒット時に呼んでほしい処理書く
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Hit"));
	m_IsHit = true;

	if (const AEnemyBase* HitEnemy = Cast<AEnemyBase>(_hitActor))
	{
		if (m_AttackComponent)
		{
			m_AttackComponent->SetLockedAttackTarget(HitEnemy);
		}
	}

}


void APlayerCharacter::Input_SplineMove()
{
	if (!m_CurrentSplineMoveActor.IsValid())
		return;

	m_CurrentSplineMoveActor->RequestStartSplineMove(this);
}


void APlayerCharacter::SetCurrentSplineMoveActor(APlayer_SplineMove* Actor)
{
	m_CurrentSplineMoveActor = Actor;
}

void APlayerCharacter::PausePlayer(const bool _isPause) {
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		if (_isPause)
		{
			MoveComp->StopMovementImmediately();
			MoveComp->SetMovementMode(MOVE_None);
			MoveComp->SetComponentTickEnabled(false);
		}
		else
		{
			MoveComp->SetMovementMode(MOVE_Walking); // 必要に応じて元のモードへ
			MoveComp->SetComponentTickEnabled(true);
		}
	}

	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh)
	{
		UAnimInstance* AnimInstance = SkelMesh->GetAnimInstance();
		if (AnimInstance)
		{
			if (_isPause)
			{
				AnimInstance->Montage_Pause(nullptr);   // 全モンタージュ一時停止
			}
			else
			{
				AnimInstance->Montage_Resume(nullptr);  // モンタージュ再開
			}
		}

		SkelMesh->GlobalAnimRateScale = _isPause ? 0.0f : 1.0f;
	}

	//クリア時のカメラ
	if (m_CameraComponent)
	{
		if (_isPause)
		{
			m_CameraComponent->ClearCamera();
		}
		else
		{
			m_CameraComponent->ResetCamera();

		}
	}

	SetActorTickEnabled(!_isPause);
}

void APlayerCharacter::SetIsEnhancedAttack(const bool _EnhancedAttack)
{

	m_IsEnhancedAttack = _EnhancedAttack;
}

void APlayerCharacter::SetInvincible(const bool _invincible)
{
	m_Invincible = _invincible;
}

void APlayerCharacter::OnActionCommitted(EPlayerActionCommit Action)
{
	switch (Action)
	{
	case EPlayerActionCommit::LightAttack:
		// 消費は別処理
		break;
	case EPlayerActionCommit::OtherAttack:
		break;
	default:
		SetIsEnhancedAttack(false);
		break;
	}
}

void APlayerCharacter::RevivalCollision()
{
	// --- コリジョン復帰 ---
	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Block
	);
}


void APlayerCharacter::DeleteCollision()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Ignore
	);
}

void APlayerCharacter::BroadcastJustEvasiveSuccess(const AActor* Attacker)
{
	// サブシステムを取得して通知
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UPlayerNotifySubsystem* Subsystem = LP->GetSubsystem<UPlayerNotifySubsystem>())
			{
				Subsystem->NotifyJustEvasive(Attacker);
			}
		}
	}
}

void APlayerCharacter::OpeningCamera()
{
	m_CameraComponent->PlayInGameOpening();
}

void APlayerCharacter::OnBossBattleStarted()
{
	m_IsBossBattleActive = true;
	DisableControl();
}

void APlayerCharacter::OnBossBattleEnded()
{
	m_IsBossBattleActive = false;
}


void APlayerCharacter::DisableControl()
{
	//入力停止
	DisableInput(Cast<APlayerController>(GetController()));

	//移動停止
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
}

void APlayerCharacter::EnableControl()
{
	//移動復帰
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	//入力復帰
	EnableInput(Cast<APlayerController>(GetController()));

}

void APlayerCharacter::Debug_ToggleInfiniteHP()
{
	// フラグを反転
	m_IsDebugGodMode = !m_IsDebugGodMode;

	if (m_IsDebugGodMode)
	{

		if (GEngine)
		{
			// ログを見やすく（シアン色など）
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("GOD MODE: ON (HP Infinite)"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("GOD MODE: OFF"));
		}
	}
}

void APlayerCharacter::Debug_ToggleOverCharge()
{
	m_ElectroGaugeComponent->DebugOverCharge();
	if (GEngine)
	{
		// ログを見やすく（シアン色など）
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("God OverCharge"));
	}

}

void APlayerCharacter::Debug_TogglePlayerDie()
{
	PlayerDying();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("God PlayerDie"));
	}

}

void APlayerCharacter::Debug_WarpBossEvent()
{
	FVector TargetLocation(-31013.0f, 6854.0f, 4600.0f);

	TeleportTo(TargetLocation, GetActorRotation());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("GodWarp"));
	}
}

void APlayerCharacter::CheckJustEvasiveTargetValidity()
{
	// ターゲットがいなければ何もしない
	if (!m_JustEvasive_Attacker) return;

	// 敵がDestroyされている（無効）ならリセット
	if (!IsValid(m_JustEvasive_Attacker))
	{
		m_JustEvasive_Attacker = nullptr;
		return;
	}

	if (m_IsBossBattleActive)
	{
		return;
	}

	if (const AEnemyBase* TargetEnemy = Cast<AEnemyBase>(m_JustEvasive_Attacker))
	{
		if (TargetEnemy->GetIsDying())
		{
			UE_LOG(LogTemp, Log, TEXT("Target Lost : Enemy Died"));
			m_JustEvasive_Attacker = nullptr;
			return;
		}
	}

	const float Distance = GetDistanceTo(m_JustEvasive_Attacker);
	UE_LOG(LogTemp, Warning, TEXT("Dist: %f / Limit: %f"), Distance, PlayerParam.JustEvasiveTargetKeepDistance);
	if (Distance > PlayerParam.JustEvasiveTargetKeepDistance)
	{
		UE_LOG(LogTemp, Error, TEXT("Target Lost! Dist: %f > Limit: %f"), Distance, PlayerParam.JustEvasiveTargetKeepDistance);
		m_JustEvasive_Attacker = nullptr;
		return;
	}

}


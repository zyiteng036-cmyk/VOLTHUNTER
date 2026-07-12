#include "CustomCharacterMovementComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Solver/PlayerMovementSolver.h"
#include "../DataAssets/Player/PlayerMovementParameter.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

namespace
{
	//ジャスト回避後に敵方向を維持する時間
	constexpr float JustEvasiveFacingLockDuration = 0.25f;

	//空中状態タグ
	const FName Tag_State_Movement_Airborne = TEXT("State.Movement.Airborne");
}

//初期化
UCustomCharacterMovementComponent::UCustomCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

//開始時
void UCustomCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	//各状態終了時に戻す標準移動値を保存
	m_DefaultBrakingDecelerationWalking = BrakingDecelerationWalking;
	m_DefaultGroundFriction = GroundFriction;
	m_DefaultBrakingFrictionFactor = BrakingFrictionFactor;
	m_DefaultAirControl = AirControl;

	ApplyParameterDefaults();
}

//毎フレーム更新
void UCustomCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//入力継続時間とアイドル時の旋回設定を更新
	UpdateJumpHold(DeltaTime);
	UpdateIdleRotation();
}

//攻撃開始時の移動入力を生入力から保存
void UCustomCharacterMovementComponent::CacheAttackStartInputFromRaw(const FVector2D& InputVector)
{
	m_AttackStartInputState.RawInput = FVector2D::ZeroVector;
	m_AttackStartInputState.WorldDirection = FVector::ZeroVector;
	m_AttackStartInputState.bHasInput = false;

	if (!m_MovementParameter) return;

	if (InputVector.SizeSquared() < m_MovementParameter->InputDeadZoneSq)
	{
		return;
	}

	const FVector InputWorldDirection = FPlayerMovementSolver::CalculateInputWorldDirection(InputVector, GetControlRotationFlat());

	if (InputWorldDirection.IsNearlyZero())
	{
		return;
	}

	m_AttackStartInputState.RawInput = InputVector;
	m_AttackStartInputState.WorldDirection = InputWorldDirection.GetSafeNormal2D();
	m_AttackStartInputState.bHasInput = true;
}

//パラメータ初期値を反映
void UCustomCharacterMovementComponent::ApplyParameterDefaults()
{
	if (!m_MovementParameter) return;

	MaxWalkSpeed = m_MovementParameter->RunSpeed;
	GravityScale = m_MovementParameter->GravityScale;
	AirControl = m_MovementParameter->AirControl;

	m_ExternalSpeedMultiplier = m_MovementParameter->DefaultExternalSpeedMultiplier;
}

//移動入力を更新
void UCustomCharacterMovementComponent::UpdateMovementInput(const FVector2D& InputVector)
{
	if (!m_MovementParameter) return;

	if (!m_bCanMovement)
	{
		ClearMovementInput();
		return;
	}

	//生入力を保存してデッドゾーン判定を更新
	m_InputState.RawInput = InputVector;
	m_InputState.bHasInput = HasMoveInput();

	if (!m_InputState.bHasInput)
	{
		ClearMovementInput();
		return;
	}

	const FVector InputWorldDirection = FPlayerMovementSolver::CalculateInputWorldDirection(InputVector, GetControlRotationFlat());

	if (InputWorldDirection.IsNearlyZero())
	{
		ClearMovementInput();
		return;
	}

	m_InputState.WorldDirection = InputWorldDirection;

	//Dash中は入力方向を固定しない
	//現在のカメラ基準入力で移動させる
	if (m_CurrentMovementState == EPlayerMovementState::Dash)
	{
		m_LockedDashDirection = InputWorldDirection.GetSafeNormal2D();

		AddInputVector(m_InputState.WorldDirection, false);
		return;
	}

	//DashDecel中は慣性を残すため、入力で加速させない
	if (m_CurrentMovementState == EPlayerMovementState::DashDecel)
	{
		return;
	}

	AddInputVector(m_InputState.WorldDirection, false);
}

//移動入力をクリア
void UCustomCharacterMovementComponent::ClearMovementInput()
{
	m_InputState.RawInput = FVector2D::ZeroVector;
	m_InputState.WorldDirection = FVector::ZeroVector;
	m_InputState.bHasInput = false;
}

//移動入力を減衰
void UCustomCharacterMovementComponent::DecayMovementInput()
{
	if (!m_MovementParameter) return;

	//入力終了後も残っている値を徐々に減衰
	m_InputState.RawInput *= m_MovementParameter->MoveInputDecayRate;

	m_InputState.bHasInput = HasMoveInput();

	if (!m_InputState.bHasInput)
	{
		ClearMovementInput();
		return;
	}

	m_InputState.WorldDirection = FPlayerMovementSolver::CalculateInputWorldDirection(m_InputState.RawInput, GetControlRotationFlat());
}

//入力があるか
bool UCustomCharacterMovementComponent::HasMoveInput() const
{
	if (!m_MovementParameter) return false;

	return m_InputState.RawInput.SizeSquared() >= m_MovementParameter->InputDeadZoneSq;
}

//アイドル状態を更新
void UCustomCharacterMovementComponent::UpdateIdleRotation()
{
	if (!m_MovementParameter) return;

	const bool bMoving = Velocity.SizeSquared() > m_MovementParameter->IdleVelocitySqThreshold;
	const bool bGrounded = IsMovingOnGround();

	//接地中かつ停止中のみアイドルとして扱う
	m_bIsIdle = !bMoving && bGrounded;

	if (m_bIsIdle)
	{
		RotationRate = FRotator::ZeroRotator;
		return;
	}

	RotationRate = FRotator(0.f, m_MovementParameter->MoveRotationRateYaw, 0.f);
}

//ダッシュ開始可能か
bool UCustomCharacterMovementComponent::CanStartDash() const
{
	if (!m_MovementParameter) return false;
	if (!m_bCanMovement) return false;
	if (!IsMovingOnGround()) return false;

	if (m_CurrentMovementState == EPlayerMovementState::Evasive) return false;
	if (m_CurrentMovementState == EPlayerMovementState::EvasiveRecovery) return false;
	if (m_CurrentMovementState == EPlayerMovementState::Disabled) return false;

	if (!HasMoveInput()) return false;

	return true;
}

//Dash開始
bool UCustomCharacterMovementComponent::StartDash()
{
	if (!CanStartDash()) return false;

	//開始時の入力とカメラ向きからダッシュ方向を確定
	m_LockedDashDirection = CalculateDashStartDirection();

	if (m_LockedDashDirection.IsNearlyZero())
	{
		return false;
	}

	//移動入力なし時間を初期化
	m_DashNoInputElapsedTime = 0.f;

	SetMovementState(EPlayerMovementState::Dash);

	if (m_MovementParameter)
	{
		//ダッシュ速度を即時設定して開始時の加速感を作る
		MaxWalkSpeed = m_MovementParameter->DashSpeed;

		const FVector DashVelocity = FPlayerMovementSolver::CalculateDashLaunchVelocity(m_LockedDashDirection, m_MovementParameter);

		Velocity.X = DashVelocity.X;
		Velocity.Y = DashVelocity.Y;
	}

	if (CharacterOwner)
	{
		CharacterOwner->SetActorRotation(m_LockedDashDirection.Rotation());
	}

	return true;
}

//Dash中か取得
bool UCustomCharacterMovementComponent::IsDashing() const
{
	return m_CurrentMovementState == EPlayerMovementState::Dash;
}

//ダッシュ状態を更新
void UCustomCharacterMovementComponent::UpdateDashState(float DeltaTime)
{
	if (!m_MovementParameter) return;

	if (
		m_CurrentMovementState != EPlayerMovementState::Dash &&
		m_CurrentMovementState != EPlayerMovementState::DashDecel
		)
	{
		return;
	}

	if (m_CurrentMovementState == EPlayerMovementState::Dash)
	{
		MaxWalkSpeed = FMath::FInterpTo(MaxWalkSpeed, m_MovementParameter->DashSpeed, DeltaTime, m_MovementParameter->DashSpeedInterpSpeed);

		const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.f);

		const float HorizontalSpeedSq = HorizontalVelocity.SizeSquared();

		//速度が終了値以下なら即終了
		if (
			HorizontalSpeedSq <=
			m_MovementParameter->DashEndVelocitySqThreshold
			)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Dash] EndBySpeed Speed:%f Threshold:%f"), FMath::Sqrt(HorizontalSpeedSq), FMath::Sqrt(m_MovementParameter->DashEndVelocitySqThreshold));

			EndDash();
			return;
		}

		//移動入力がある間はダッシュを維持
		if (HasMoveInput())
		{
			m_DashNoInputElapsedTime = 0.f;
			return;
		}

		//移動入力がない場合だけ時間を進める
		m_DashNoInputElapsedTime += DeltaTime;

		if (
			m_DashNoInputElapsedTime >=
			m_MovementParameter->DashNoInputEndDelay
			)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Dash] BeginDecelByNoInput Elapsed:%f Delay:%f Speed:%f"), m_DashNoInputElapsedTime, m_MovementParameter->DashNoInputEndDelay, FMath::Sqrt(HorizontalSpeedSq));

			m_DashNoInputElapsedTime = 0.f;
			BeginDashDecel();
		}

		return;
	}

	if (m_CurrentMovementState == EPlayerMovementState::DashDecel)
	{
		Velocity = FPlayerMovementSolver::CalculateDashDecelVelocity(Velocity, DeltaTime, m_MovementParameter);

		if (
			FPlayerMovementSolver::ShouldFinishDashDecel(
				Velocity,
				m_MovementParameter
			)
			)
		{
			EndDash();
		}
	}
}

//Dash減速へ移行
void UCustomCharacterMovementComponent::BeginDashDecel()
{
	if (m_CurrentMovementState != EPlayerMovementState::Dash) return;
	if (!m_MovementParameter) return;

	SetMovementState(EPlayerMovementState::DashDecel);

	//減速中だけ専用のブレーキと摩擦を使用
	BrakingDecelerationWalking = m_MovementParameter->DashDecelBraking;
	GroundFriction = m_MovementParameter->DashDecelFriction;
	BrakingFrictionFactor = 1.f;

}

//Dash終了
void UCustomCharacterMovementComponent::EndDash()
{
	if (
		m_CurrentMovementState != EPlayerMovementState::Dash &&
		m_CurrentMovementState != EPlayerMovementState::DashDecel
		)
	{
		return;
	}

	//ダッシュ固有の入力時間と方向を初期化
	m_DashNoInputElapsedTime = 0.f;
	m_LockedDashDirection = FVector::ZeroVector;

	if (m_MovementParameter)
	{
		MaxWalkSpeed = m_MovementParameter->RunSpeed;
	}

	//ダッシュ前の通常移動設定へ戻す
	BrakingDecelerationWalking = m_DefaultBrakingDecelerationWalking;

	GroundFriction = m_DefaultGroundFriction;

	BrakingFrictionFactor = m_DefaultBrakingFrictionFactor;

	if (IsMovingOnGround())
	{
		SetMovementState(EPlayerMovementState::Normal);
	}
	else
	{
		SetMovementState(EPlayerMovementState::Airborne);
	}
}

//ジャンプ開始可能か
bool UCustomCharacterMovementComponent::CanStartJump() const
{
	if (!m_MovementParameter) return false;
	if (!m_bCanMovement) return false;
	if (m_bIsJumpCooldown) return false;
	if (!IsMovingOnGround()) return false;

	if (m_CurrentMovementState == EPlayerMovementState::Evasive) return false;
	if (m_CurrentMovementState == EPlayerMovementState::EvasiveRecovery) return false;
	if (m_CurrentMovementState == EPlayerMovementState::Disabled) return false;

	return true;
}

//ジャンプ開始
bool UCustomCharacterMovementComponent::StartJump()
{
	if (!CanStartJump()) return false;

	//現在速度を基準にジャンプ開始速度を計算
	Velocity = FPlayerMovementSolver::CalculateJumpVelocity(Velocity, m_MovementParameter);

	SetMovementMode(MOVE_Falling);
	SetMovementState(EPlayerMovementState::Airborne);

	m_bIsJumpCharging = true;
	m_JumpHoldTime = 0.f;

	return true;
}

//ジャンプ入力を離した時
void UCustomCharacterMovementComponent::ReleaseJump()
{
	if (!m_bIsJumpCharging) return;

	//短押し・長押しでジャンプ高度を変えない
	m_bIsJumpCharging = false;
}

//Dash開始方向を計算
FVector UCustomCharacterMovementComponent::CalculateDashStartDirection() const
{
	if (!m_MovementParameter)
	{
		return FVector::ZeroVector;
	}

	//Dash開始時はキャッシュ済みWorldDirectionを信用しない
	//RawInputと現在のカメラ向きからその場で方向を作る
	if (m_InputState.RawInput.SizeSquared() >= m_MovementParameter->InputDeadZoneSq)
	{
		const FVector DashInputDirection = FPlayerMovementSolver::CalculateInputWorldDirection(m_InputState.RawInput, GetControlRotationFlat());

		if (!DashInputDirection.IsNearlyZero())
		{
			return DashInputDirection.GetSafeNormal2D();
		}
	}

	//保険。入力が取れない場合はカメラ正面へDash
	const FRotator ControlRotation = GetControlRotationFlat();
	const FVector CameraForward = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X);

	return CameraForward.GetSafeNormal2D();
}

//ジャンプ長押し時間を更新
void UCustomCharacterMovementComponent::UpdateJumpHold(float DeltaTime)
{
	if (!m_bIsJumpCharging) return;

	m_JumpHoldTime += DeltaTime;
}

//回避開始可能か
bool UCustomCharacterMovementComponent::CanStartEvasive() const
{
	if (!m_MovementParameter) return false;

	//空中回避は禁止
	if (!IsMovingOnGround()) return false;

	if (m_CurrentMovementState == EPlayerMovementState::Evasive) return false;
	if (m_CurrentMovementState == EPlayerMovementState::EvasiveRecovery) return false;
	if (m_CurrentMovementState == EPlayerMovementState::Disabled) return false;

	return true;
}
//回避開始
bool UCustomCharacterMovementComponent::StartEvasive()
{
	if (!CanStartEvasive()) return false;

	if (m_CurrentMovementState == EPlayerMovementState::Dash ||
		m_CurrentMovementState == EPlayerMovementState::DashDecel)
	{
		CancelDash();
	}

	//回避開始フレームの入力を固定して方向を決定
	const FVector2D InputAtStart = m_InputState.RawInput;

	const bool bHasInputAtStart = m_MovementParameter && InputAtStart.SizeSquared() >= m_MovementParameter->InputDeadZoneSq;

	const FVector ActorForward = CharacterOwner ? CharacterOwner->GetActorForwardVector() : FVector::ForwardVector;

	m_LastEvasiveDirectionType = bHasInputAtStart ? EPlayerEvasiveDirectionType::InputDirection : EPlayerEvasiveDirectionType::BackStep;

	m_LockedEvasiveDirection = FPlayerMovementSolver::CalculateEvasiveDirection(InputAtStart, GetControlRotationFlat(), ActorForward, m_MovementParameter);

	if (m_LockedEvasiveDirection.IsNearlyZero())
	{
		m_LockedEvasiveDirection = -ActorForward.GetSafeNormal2D();
	}

	//回避中は通常移動入力を停止
	SetCanMovement(false);
	SetMovementState(EPlayerMovementState::Evasive);

	if (CharacterOwner)
	{
		const FVector LaunchVelocity = FPlayerMovementSolver::CalculateEvasiveLaunchVelocity(m_LockedEvasiveDirection, m_MovementParameter);

		CharacterOwner->LaunchCharacter(LaunchVelocity, true, true);
	}

	return true;
}

//回避終了
void UCustomCharacterMovementComponent::EndEvasive()
{
	//回避が止めた移動は、状態に関係なく必ず戻す
	SetCanMovement(true);

	if (m_bJustEvasiveFacingLock)
	{
		m_LockedEvasiveDirection = FVector::ZeroVector;

		if (IsMovingOnGround())
		{
			SetMovementState(EPlayerMovementState::Normal);
		}
		else
		{
			SetMovementState(EPlayerMovementState::Airborne);
		}

		return;
	}

	if (m_CurrentMovementState != EPlayerMovementState::Evasive &&
		m_CurrentMovementState != EPlayerMovementState::EvasiveRecovery)
	{
		return;
	}

	m_LockedEvasiveDirection = FVector::ZeroVector;

	//硬直時間が設定されている場合は回避後硬直へ移行
	if (m_MovementParameter && m_MovementParameter->EvasiveRecoveryTime > 0.f)
	{
		m_EvasiveRecoveryTimer = m_MovementParameter->EvasiveRecoveryTime;
		SetMovementState(EPlayerMovementState::EvasiveRecovery);
		return;
	}

	if (IsMovingOnGround())
	{
		SetMovementState(EPlayerMovementState::Normal);
	}
	else
	{
		SetMovementState(EPlayerMovementState::Airborne);
	}
}

//回避後硬直を更新
void UCustomCharacterMovementComponent::UpdateEvasiveRecovery(float DeltaTime)
{
	if (m_CurrentMovementState != EPlayerMovementState::EvasiveRecovery) return;

	m_EvasiveRecoveryTimer -= DeltaTime;

	if (m_EvasiveRecoveryTimer > 0.f) return;

	m_EvasiveRecoveryTimer = 0.f;

	//硬直終了時も必ず移動可能へ戻す
	SetCanMovement(true);

	if (IsMovingOnGround())
	{
		SetMovementState(EPlayerMovementState::Normal);
	}
	else
	{
		SetMovementState(EPlayerMovementState::Airborne);
	}
}

//ジャスト回避後の敵方向固定を更新
void UCustomCharacterMovementComponent::UpdateJustEvasiveFacingLock(float DeltaTime)
{
	if (!m_bJustEvasiveFacingLock) return;

	m_JustEvasiveFacingLockTime -= DeltaTime;

	if (m_JustEvasiveFacingLockTime > 0.f) return;

	EndJustEvasiveFacingLock();
}

//ジャスト回避後の敵方向固定を終了
void UCustomCharacterMovementComponent::EndJustEvasiveFacingLock()
{
	if (!m_bJustEvasiveFacingLock) return;

	m_bJustEvasiveFacingLock = false;
	m_JustEvasiveFacingLockTime = 0.f;

	bOrientRotationToMovement = m_bSavedOrientRotationToMovement;
}

//移動計算前の状態更新
void UCustomCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	if (!m_MovementParameter) return;

	//上昇中と落下中の状態に応じた重力倍率を反映
	GravityScale = FPlayerMovementSolver::CalculateGravityScale(Velocity, IsMovingOnGround(), m_MovementParameter);

	UpdateJustEvasiveFacingLock(DeltaSeconds);
	UpdateDashState(DeltaSeconds);
	UpdateEvasiveRecovery(DeltaSeconds);
}

//ジャスト回避カウンター移動開始
void UCustomCharacterMovementComponent::StartJustEvasiveCounterMove(const AActor* TargetActor)
{
	if (!TargetActor) return;
	if (!CharacterOwner) return;

	const FVector OwnerLocation = CharacterOwner->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();

	FVector Direction = TargetLocation - OwnerLocation;
	Direction.Z = 0.f;

	if (!Direction.Normalize()) return;

	//敵方向を向かせるだけで、敵方向への速度は足さない
	const FRotator TargetRotation = Direction.Rotation();

	if (!m_bJustEvasiveFacingLock)
	{
		m_bSavedOrientRotationToMovement = bOrientRotationToMovement;
	}

	bOrientRotationToMovement = false;
	m_bJustEvasiveFacingLock = true;
	m_JustEvasiveFacingLockTime = JustEvasiveFacingLockDuration;

	ClearMovementInput();
	SetCanMovement(true);

	CharacterOwner->SetActorRotation(TargetRotation);
}

//MovementMode変更時
void UCustomCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	//落下開始時に移動状態を空中へ同期
	if (MovementMode == MOVE_Falling)
	{
		if (m_CurrentMovementState == EPlayerMovementState::Dash ||
			m_CurrentMovementState == EPlayerMovementState::DashDecel ||
			m_CurrentMovementState == EPlayerMovementState::Evasive)
		{
			return;
		}

		SetMovementState(EPlayerMovementState::Airborne);
		return;
	}

	//着地時にジャンプ状態を終了してクールタイムを開始
	if (MovementMode == MOVE_Walking)
	{
		if (m_CurrentMovementState == EPlayerMovementState::Airborne)
		{
			m_bIsJumpCharging = false;
			StartJumpCooldown();
			SetMovementState(EPlayerMovementState::Normal);
		}
	}
}

//ジャンプクールタイム開始
void UCustomCharacterMovementComponent::StartJumpCooldown()
{
	if (!m_MovementParameter) return;

	m_bIsJumpCooldown = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(m_JumpCooldownTimerHandle, this, &UCustomCharacterMovementComponent::OnJumpCooldownFinished, m_MovementParameter->JumpCooldownTime, false);
	}
}

//ジャンプクールタイム終了
void UCustomCharacterMovementComponent::OnJumpCooldownFinished()
{
	m_bIsJumpCooldown = false;
}

//キャラクター回転制御
void UCustomCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (!CharacterOwner)
	{
		Super::PhysicsRotation(DeltaTime);
		return;
	}

	if (m_CurrentMovementState == EPlayerMovementState::Evasive)
	{
		return;
	}

	if (m_bJustEvasiveFacingLock)
	{
		return;
	}

	if (m_CurrentMovementState == EPlayerMovementState::Dash)
	{
		//ダッシュ中は現在の入力方向を優先して旋回
		FVector FaceDirection = FVector::ZeroVector;

		if (!m_InputState.WorldDirection.IsNearlyZero())
		{
			FaceDirection = m_InputState.WorldDirection.GetSafeNormal2D();
		}
		else if (!m_LockedDashDirection.IsNearlyZero())
		{
			FaceDirection = m_LockedDashDirection.GetSafeNormal2D();
		}

		if (!FaceDirection.IsNearlyZero())
		{
			const FRotator TargetRotation = FaceDirection.Rotation();
			const FRotator CurrentRotation = CharacterOwner->GetActorRotation();

			CharacterOwner->SetActorRotation(FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, RotationRate.Yaw));
		}

		return;
	}

	if (m_CurrentMovementState == EPlayerMovementState::DashDecel)
	{
		//減速中は残っている速度方向へ旋回
		const FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.f);

		if (!HorizontalVelocity.IsNearlyZero())
		{
			const FRotator TargetRotation = HorizontalVelocity.GetSafeNormal().Rotation();
			const FRotator CurrentRotation = CharacterOwner->GetActorRotation();

			CharacterOwner->SetActorRotation(FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, RotationRate.Yaw));
		}

		return;
	}

	Super::PhysicsRotation(DeltaTime);
}

//最大速度取得
float UCustomCharacterMovementComponent::GetMaxSpeed() const
{
	return Super::GetMaxSpeed() * m_ExternalSpeedMultiplier;
}

//移動状態を変更
void UCustomCharacterMovementComponent::SetMovementState(EPlayerMovementState NewState)
{
	if (m_CurrentMovementState == NewState)
	{
		//状態が同じでもタグだけ破綻している場合に復旧する
		UpdateMovementStateGameplayTags(NewState);
		return;
	}

	const EPlayerMovementState PreviousState = m_CurrentMovementState;

	m_CurrentMovementState = NewState;

	UpdateMovementStateGameplayTags(NewState);

	BroadcastMovementStateChanged(PreviousState, NewState);
}

//移動状態をリセット
void UCustomCharacterMovementComponent::ResetMovementState()
{
	//入力と各移動状態のRuntime値を初期化
	ClearMovementInput();

	m_LockedDashDirection = FVector::ForwardVector;
	m_LockedEvasiveDirection = FVector::ForwardVector;

	if (m_bJustEvasiveFacingLock)
	{
		bOrientRotationToMovement = m_bSavedOrientRotationToMovement;
	}

	m_bJustEvasiveFacingLock = false;
	m_JustEvasiveFacingLockTime = 0.f;

	m_bCanMovement = true;
	m_bIsIdle = false;
	m_bIsJumpCharging = false;
	m_bIsJumpCooldown = false;
	m_JumpHoldTime = 0.f;
	m_EvasiveRecoveryTimer = 0.f;

	Velocity = FVector::ZeroVector;
	StopMovementImmediately();

	ApplyParameterDefaults();

	SetMovementState(EPlayerMovementState::Normal);
}
//移動可能か設定
void UCustomCharacterMovementComponent::SetCanMovement(bool bCanMove)
{
	m_bCanMovement = bCanMove;

	if (!m_bCanMovement)
	{
		ClearMovementInput();
	}
}

//外部速度倍率を設定
void UCustomCharacterMovementComponent::SetExternalSpeedMultiplier(float NewMultiplier)
{
	m_ExternalSpeedMultiplier = FMath::Max(NewMultiplier, 0.f);
}

//ダッシュ中か
bool UCustomCharacterMovementComponent::GetIsDash() const
{
	return m_CurrentMovementState == EPlayerMovementState::Dash ||
		m_CurrentMovementState == EPlayerMovementState::DashDecel;
}

//回避中か
bool UCustomCharacterMovementComponent::GetIsEvasive() const
{
	return m_CurrentMovementState == EPlayerMovementState::Evasive;
}

//空中状態か
bool UCustomCharacterMovementComponent::GetIsAirborne() const
{
	return m_CurrentMovementState == EPlayerMovementState::Airborne;
}

//操作方向の平面回転を取得
FRotator UCustomCharacterMovementComponent::GetControlRotationFlat() const
{
	if (!CharacterOwner || !CharacterOwner->Controller)
	{
		return FRotator::ZeroRotator;
	}

	FRotator ControlRotation = CharacterOwner->Controller->GetControlRotation();
	ControlRotation.Pitch = 0.f;
	ControlRotation.Roll = 0.f;

	return ControlRotation;
}

//入力方向を取得
bool UCustomCharacterMovementComponent::GetMoveInputDirection(FVector& OutDirection) const
{
	OutDirection = FVector::ZeroVector;

	if (!m_InputState.bHasInput) return false;

	OutDirection = m_InputState.WorldDirection;

	return !OutDirection.IsNearlyZero();
}

//入力がなければ正面方向を返す
bool UCustomCharacterMovementComponent::GetDesiredMoveDirection(FVector& OutDirection) const
{
	if (GetMoveInputDirection(OutDirection))
	{
		return true;
	}

	if (CharacterOwner)
	{
		OutDirection = CharacterOwner->GetActorForwardVector();
		return false;
	}

	OutDirection = FVector::ForwardVector;
	return false;
}

//Dash停止要求
void UCustomCharacterMovementComponent::RequestStopDash()
{
	if (m_CurrentMovementState == EPlayerMovementState::Dash)
	{
		BeginDashDecel();
		return;
	}

	if (m_CurrentMovementState == EPlayerMovementState::DashDecel)
	{
		return;
	}
}
//Dashを強制キャンセル
void UCustomCharacterMovementComponent::CancelDash()
{
	if (m_CurrentMovementState != EPlayerMovementState::Dash &&
		m_CurrentMovementState != EPlayerMovementState::DashDecel)
	{
		return;
	}

	EndDash();
}

//攻撃開始時の移動入力を保存
void UCustomCharacterMovementComponent::CacheAttackStartInput()
{
	m_AttackStartInputState.RawInput = m_InputState.RawInput;
	m_AttackStartInputState.WorldDirection = m_InputState.WorldDirection;
	m_AttackStartInputState.bHasInput = m_InputState.bHasInput;

	UE_LOG(LogTemp, Warning, TEXT("[AttackInputCache] Raw:%s World:%s Has:%s"), *m_AttackStartInputState.RawInput.ToString(), *m_AttackStartInputState.WorldDirection.ToString(), m_AttackStartInputState.bHasInput ? TEXT("true") : TEXT("false"));
}

//攻撃開始時の移動入力をクリア
void UCustomCharacterMovementComponent::ClearAttackStartInput()
{
	m_AttackStartInputState.RawInput = FVector2D::ZeroVector;
	m_AttackStartInputState.WorldDirection = FVector::ZeroVector;
	m_AttackStartInputState.bHasInput = false;
}

//状態変更通知
void UCustomCharacterMovementComponent::BroadcastMovementStateChanged(EPlayerMovementState PreviousState, EPlayerMovementState NewState)
{
	OnMovementStateChanged.Broadcast(PreviousState, NewState);
}

//移動状態をGameplayTagへ反映
void UCustomCharacterMovementComponent::UpdateMovementStateGameplayTags(EPlayerMovementState NewState)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(CharacterOwner);

	if (!Player) return;

	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();

	if (!ASC) return;

	//空中状態を表すGameplayTagを取得
	const FGameplayTag AirborneStateTag = FGameplayTag::RequestGameplayTag(Tag_State_Movement_Airborne, false);

	if (!AirborneStateTag.IsValid()) return;

	const bool bShouldHaveAirborneTag = NewState == EPlayerMovementState::Airborne;

	const bool bHasAirborneTag = ASC->HasMatchingGameplayTag(AirborneStateTag);

	//空中へ移行した時だけTagを追加
	if (bShouldHaveAirborneTag && !bHasAirborneTag)
	{
		ASC->AddLooseGameplayTag(AirborneStateTag);

		UE_LOG(LogTemp, Warning, TEXT("[MovementTag] Add State.Movement.Airborne"));

		return;
	}

	//地上状態へ戻った時だけTagを解除
	if (!bShouldHaveAirborneTag && bHasAirborneTag)
	{
		ASC->RemoveLooseGameplayTag(AirborneStateTag);

		UE_LOG(LogTemp, Warning, TEXT("[MovementTag] Remove State.Movement.Airborne"));
	}
}
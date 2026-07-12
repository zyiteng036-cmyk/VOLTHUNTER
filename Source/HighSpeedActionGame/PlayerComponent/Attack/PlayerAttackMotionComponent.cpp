#include "PlayerAttackMotionComponent.h"
#include "../../DataAssets/Player/PlayerAttackParameter.h"
#include "../../DataAssets/Player/PlayerMovementParameter.h"
#include "../../Enemy/EnemyBase.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../CustomCharacterMovementComponent.h"
#include "../PlayerAttackComponent.h"
#include "../PlayerEvasiveComponent.h"
#include "../Solver/PlayerAttackSolver.h"
#include "PlayerAttackTargetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

//初期化
UPlayerAttackMotionComponent::UPlayerAttackMotionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//所有者と関連コンポーネントを取得
void UPlayerAttackMotionComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player) return;

	m_AttackComponent = m_Player->FindComponentByClass<UPlayerAttackComponent>();
	m_TargetComponent = m_Player->FindComponentByClass<UPlayerAttackTargetComponent>();
	m_CustomMovementComponent = m_Player->FindComponentByClass<UCustomCharacterMovementComponent>();
	m_EvasiveComponent = m_Player->FindComponentByClass<UPlayerEvasiveComponent>();
}

//使用する参照を初期化
void UPlayerAttackMotionComponent::InitializeMotionComponent(APlayerCharacter* Player, UPlayerAttackComponent* AttackComponent, UPlayerAttackTargetComponent* TargetComponent, UPlayerAttackParameter* AttackParameter)
{
	m_Player = Player;
	m_AttackComponent = AttackComponent;
	m_TargetComponent = TargetComponent;
	m_AttackParameter = AttackParameter;

	if (!m_Player) return;

	m_CustomMovementComponent = m_Player->FindComponentByClass<UCustomCharacterMovementComponent>();
	m_EvasiveComponent = m_Player->FindComponentByClass<UPlayerEvasiveComponent>();
}

//攻撃パラメータを設定
void UPlayerAttackMotionComponent::SetAttackParameter(UPlayerAttackParameter* AttackParameter)
{
	m_AttackParameter = AttackParameter;
}

//攻撃移動状態を更新
void UPlayerAttackMotionComponent::TickAttackMotion(float DeltaTime)
{
	UpdateAirAttackUnlock();
	UpdateAirFallAttack(DeltaTime);

	//空中ダッシュ攻撃中だけ踏み込み位置を継続更新
	if (m_bIsAirDashAttack)
	{
		TickAttackStep(DeltaTime);
	}
}

//ジャンプ開始時の高さを記録
void UPlayerAttackMotionComponent::OnJumpStarted()
{
	if (!m_Player) return;

	m_JumpStartZ = m_Player->GetActorLocation().Z;
	m_bCanAirAttack = false;
}

//現在の攻撃種別に応じた踏み込みを開始
void UPlayerAttackMotionComponent::BeginCurrentAttackStep()
{
	if (!m_AttackComponent) return;

	const EPlayerAttackType CurrentAttackType = m_AttackComponent->GetCurrentAttackType();

	//空中弱攻撃は専用の3次元踏み込みを使用
	if (CurrentAttackType == EPlayerAttackType::AirLight || CurrentAttackType == EPlayerAttackType::AirDash)
	{
		BeginAirDashAttack();
		return;
	}

	BeginAttackStep();
}

//地上攻撃踏み込みを開始
void UPlayerAttackMotionComponent::BeginAttackStep()
{
	if (!m_Player || !m_AttackParameter) return;

	//ジャスト回避反撃では通常踏み込みより先にワープを試す
	if (CanExecuteJustEvasiveWarp() && ExecuteJustEvasiveWarp())
	{
		return;
	}

	ResetStepState();

	FVector MoveDirection = FVector::ZeroVector;
	bool bHasMoveInput = false;
	GetCurrentMoveInput(MoveDirection, bHasMoveInput);

	const FVector PlayerLocation = m_Player->GetActorLocation();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AttackDirection] HasInput:%s MoveDirection:%s"),
		bHasMoveInput ? TEXT("true") : TEXT("false"),
		*MoveDirection.ToString()
	);

	//移動入力がある場合は敵探索より入力方向を優先
	if (bHasMoveInput && !MoveDirection.IsNearlyZero())
	{
		const FPlayerAttackStepResult StepResult = FPlayerAttackSolver::CalculateForwardStepTarget(
			PlayerLocation,
			MoveDirection,
			m_Player->GetActorForwardVector(),
			true,
			m_AttackParameter
		);

		ApplyAttackStepResult(StepResult);
		return;
	}

	//入力がない場合だけ近くの攻撃対象を検索
	AEnemyBase* TargetEnemy = m_TargetComponent
		? m_TargetComponent->FindAttackTarget(PlayerLocation, FVector::ZeroVector, false)
		: nullptr;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AttackTarget] Target:%s"),
		TargetEnemy ? *TargetEnemy->GetName() : TEXT("None")
	);

	if (TargetEnemy)
	{
		//対象がいる場合は敵の位置まで踏み込む
		const FPlayerAttackStepResult StepResult = FPlayerAttackSolver::CalculateStepTargetToEnemy(PlayerLocation, TargetEnemy);
		ApplyAttackStepResult(StepResult);
		return;
	}

	//入力も対象もない場合は正面へ踏み込む
	const FPlayerAttackStepResult StepResult = FPlayerAttackSolver::CalculateForwardStepTarget(
		PlayerLocation,
		FVector::ZeroVector,
		m_Player->GetActorForwardVector(),
		false,
		m_AttackParameter
	);

	ApplyAttackStepResult(StepResult);
}

//踏み込み位置を更新
void UPlayerAttackMotionComponent::TickAttackStep(float DeltaTime)
{
	if (!m_Player || !m_AttackParameter || !m_bHasAttackTargetLocation) return;

	const FVector CurrentLocation = m_Player->GetActorLocation();
	const float StepSpeed = m_bIsAirDashAttack ? m_AttackParameter->AirAttackSpeed : m_AttackParameter->AttackSpeed;
	const bool bIgnoreZ = !m_bIsAirDashAttack;

	//空中ダッシュ攻撃だけ高さを含めて移動
	const FVector NextLocation = FPlayerAttackSolver::CalculateStepMoveLocation(
		CurrentLocation,
		m_AttackTargetLocation,
		StepSpeed,
		DeltaTime,
		m_AttackParameter->AttackStepStopDistance,
		bIgnoreZ
	);

	m_Player->SetActorLocation(NextLocation, true);
}

//攻撃踏み込みを終了
void UPlayerAttackMotionComponent::EndAttackStep()
{
	if (!m_Player) return;

	m_Player->RevivalCollision();

	//強攻撃継続中でなければ踏み込み無敵を解除
	if (!m_AttackComponent || !m_AttackComponent->GetHeavyAttackStart())
	{
		m_Player->SetInvincible(false);
	}

	//攻撃終了後は通常の移動方向旋回へ戻す
	if (UCharacterMovementComponent* MovementComponent = m_Player->GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = true;
	}

	ResetStepState();
	m_bIsAirDashAttack = false;
}

//空中攻撃状態を開始
void UPlayerAttackMotionComponent::BeginAirAttack()
{
	if (!m_Player) return;

	UCharacterMovementComponent* MovementComponent = m_Player->GetCharacterMovement();
	if (!MovementComponent) return;

	m_bIsAirAttackStarted = true;

	//空中攻撃中は重力と現在速度を停止
	MovementComponent->SetMovementMode(MOVE_Flying);
	MovementComponent->GravityScale = 0.f;
	MovementComponent->Velocity = FVector::ZeroVector;
}

//空中攻撃状態を終了
void UPlayerAttackMotionComponent::EndAirAttack()
{
	if (!m_Player) return;

	UCharacterMovementComponent* MovementComponent = m_Player->GetCharacterMovement();
	if (!MovementComponent) return;

	MovementComponent->GravityScale = GetDefaultGravityScale();

	//攻撃終了時に空中なら通常落下へ戻す
	if (!MovementComponent->IsMovingOnGround())
	{
		MovementComponent->SetMovementMode(MOVE_Falling);
	}

	m_bIsAirAttackStarted = false;
}

//空中ダッシュ攻撃を開始
void UPlayerAttackMotionComponent::BeginAirDashAttack()
{
	if (!m_Player || !m_AttackParameter) return;

	m_bIsAirDashAttack = true;
	m_Player->DeleteCollision();

	FVector MoveDirection = FVector::ZeroVector;
	bool bHasMoveInput = false;
	GetCurrentMoveInput(MoveDirection, bHasMoveInput);

	const FVector PlayerLocation = m_Player->GetActorLocation();

	//入力方向を考慮して空中攻撃対象を検索
	AEnemyBase* TargetEnemy = m_TargetComponent
		? m_TargetComponent->FindAttackTarget(PlayerLocation, MoveDirection, bHasMoveInput)
		: nullptr;

	const FPlayerAttackStepResult StepResult = FPlayerAttackSolver::CalculateAirDashTarget(
		PlayerLocation,
		MoveDirection,
		m_Player->GetActorForwardVector(),
		bHasMoveInput,
		TargetEnemy,
		m_AttackParameter
	);

	ApplyAttackStepResult(StepResult);
}

//空中ダッシュ攻撃を終了
void UPlayerAttackMotionComponent::EndAirDashAttack()
{
	if (!m_Player) return;

	m_bIsAirDashAttack = false;

	ResetStepState();
	m_Player->RevivalCollision();
}

//空中叩き落とし攻撃を開始
void UPlayerAttackMotionComponent::BeginAirFallAttack()
{
	if (!m_Player || !m_AttackParameter) return;

	m_Player->DeleteCollision();

	m_bIsAirFallAttack = true;
	m_bIsAirFallCharging = true;
	m_AirFallChargeTime = 0.f;

	BeginAirAttack();

	//叩き落とし前にプレイヤーを少し上昇させる
	if (UCharacterMovementComponent* MovementComponent = m_Player->GetCharacterMovement())
	{
		FVector RiseVelocity = MovementComponent->Velocity;
		RiseVelocity.Z = m_AttackParameter->AirFallRiseSpeed;
		MovementComponent->Velocity = RiseVelocity;
	}
}

//空中叩き落とし攻撃を終了
void UPlayerAttackMotionComponent::EndAirFallAttack()
{
	if (!m_Player) return;

	m_Player->RevivalCollision();

	m_bIsAirFallAttack = false;
	m_bIsAirFallCharging = false;
	m_AirFallChargeTime = 0.f;
	m_bCanAirAttack = false;
	m_JumpStartZ = 0.f;

	EndAirAttack();
}

//ジャスト回避反撃ワープを実行
bool UPlayerAttackMotionComponent::ExecuteJustEvasiveWarp()
{
	if (!m_Player) return false;

	const AActor* AttackerActor = m_Player->GetJustEvasive_Attacker();
	if (!IsValid(AttackerActor)) return false;

	AEnemyBase* TargetEnemy = Cast<AEnemyBase>(const_cast<AActor*>(AttackerActor));
	if (!TargetEnemy) return false;

	const FPlayerAttackStepResult StepResult = FPlayerAttackSolver::CalculateJustEvasiveWarpTarget(
		m_Player->GetActorLocation(),
		TargetEnemy
	);

	if (!StepResult.bHasTargetLocation) return false;

	//反撃位置へ移動する間は被ダメージを無効化
	m_Player->SetInvincible(true);
	m_Player->SetActorLocation(StepResult.TargetLocation, false);

	FVector LookDirection = StepResult.LookDirection;
	LookDirection.Z = 0.f;

	//反撃対象の方向へプレイヤーを向ける
	if (!LookDirection.IsNearlyZero())
	{
		m_Player->SetActorRotation(LookDirection.Rotation());
	}

	OnAttackMotionTargetChanged.Broadcast(TargetEnemy);

	return true;
}

//ジャスト回避反撃ワープを実行できるか
bool UPlayerAttackMotionComponent::CanExecuteJustEvasiveWarp() const
{
	if (!m_Player || !m_EvasiveComponent) return false;
	if (!m_EvasiveComponent->GetIsJustEvasive()) return false;

	return IsValid(m_Player->GetJustEvasive_Attacker());
}

//すべての攻撃移動状態をリセット
void UPlayerAttackMotionComponent::ResetAttackMotion()
{
	ResetStepState();
	ResetAirAttackState();

	if (!m_Player) return;

	m_Player->RevivalCollision();

	if (UCharacterMovementComponent* MovementComponent = m_Player->GetCharacterMovement())
	{
		//攻撃移動による速度と重力設定を初期化
		MovementComponent->StopMovementImmediately();
		MovementComponent->GravityScale = GetDefaultGravityScale();

		if (MovementComponent->MovementMode == MOVE_Flying)
		{
			MovementComponent->SetMovementMode(MOVE_Falling);
		}
	}
}

//踏み込み状態をリセット
void UPlayerAttackMotionComponent::ResetStepState()
{
	m_bHasAttackTargetLocation = false;
	m_AttackTargetLocation = FVector::ZeroVector;
}

//空中攻撃状態をリセット
void UPlayerAttackMotionComponent::ResetAirAttackState()
{
	m_bIsAirAttackStarted = false;
	m_bCanAirAttack = false;
	m_JumpStartZ = 0.f;
	m_bIsAirDashAttack = false;
	m_bIsAirFallAttack = false;
	m_bIsAirFallCharging = false;
	m_AirFallChargeTime = 0.f;
}

//空中攻撃解禁状態を更新
void UPlayerAttackMotionComponent::UpdateAirAttackUnlock()
{
	if (!IsJumping() || m_bCanAirAttack) return;
	if (!m_AttackParameter || !m_Player) return;

	const float CurrentZ = m_Player->GetActorLocation().Z;

	//ジャンプ開始位置から必要な高さへ到達したか判定
	m_bCanAirAttack = FPlayerAttackSolver::CanUnlockAirAttack(m_JumpStartZ, CurrentZ, m_AttackParameter);
}

//空中叩き落とし状態を更新
void UPlayerAttackMotionComponent::UpdateAirFallAttack(float DeltaTime)
{
	if (!m_bIsAirFallAttack || !m_Player || !m_AttackParameter) return;

	UCharacterMovementComponent* MovementComponent = m_Player->GetCharacterMovement();
	if (!MovementComponent) return;

	if (m_bIsAirFallCharging)
	{
		m_AirFallChargeTime += DeltaTime;

		//設定時間に到達するまで上空で溜め状態を維持
		if (!FPlayerAttackSolver::ShouldFinishAirFallCharge(m_AirFallChargeTime, m_AttackParameter))
		{
			return;
		}

		m_bIsAirFallCharging = false;

		//溜め終了後は落下状態へ戻して下方向速度を設定
		MovementComponent->SetMovementMode(MOVE_Falling);
		MovementComponent->GravityScale = GetDefaultGravityScale();
		MovementComponent->Velocity = FVector(0.f, 0.f, -m_AttackParameter->AirFallSpeed);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[AirHeavy] FallStart ChargeTime:%f Speed:%f"),
			m_AirFallChargeTime,
			m_AttackParameter->AirFallSpeed
		);

		return;
	}

	//着地するまで叩き落とし速度を維持
	if (!MovementComponent->IsMovingOnGround())
	{
		MovementComponent->Velocity = FVector(0.f, 0.f, -m_AttackParameter->AirFallSpeed);
	}
}

//計算結果から踏み込み先を設定
void UPlayerAttackMotionComponent::ApplyAttackStepResult(const FPlayerAttackStepResult& StepResult)
{
	if (!m_Player || !StepResult.bHasTargetLocation) return;

	m_AttackTargetLocation = StepResult.TargetLocation;
	m_bHasAttackTargetLocation = true;

	FVector LookDirection = StepResult.LookDirection;
	LookDirection.Z = 0.f;

	//踏み込み開始時に攻撃方向へ向きを合わせる
	if (!LookDirection.IsNearlyZero())
	{
		m_Player->SetActorRotation(LookDirection.Rotation());
	}

	OnAttackMotionTargetChanged.Broadcast(StepResult.TargetActor);
}

//現在の移動入力状態を取得
void UPlayerAttackMotionComponent::GetCurrentMoveInput(FVector& OutMoveDirection, bool& bOutHasMoveInput) const
{
	OutMoveDirection = FVector::ZeroVector;
	bOutHasMoveInput = false;

	if (!m_CustomMovementComponent) return;

	const FPlayerMovementInputState& AttackStartInputState = m_CustomMovementComponent->GetAttackStartInputState();

	//攻撃開始時に保存した入力を優先
	if (AttackStartInputState.bHasInput && !AttackStartInputState.WorldDirection.IsNearlyZero())
	{
		OutMoveDirection = AttackStartInputState.WorldDirection.GetSafeNormal2D();
		bOutHasMoveInput = true;
		return;
	}

	//保存入力がない場合は現在の移動入力を使用
	const FPlayerMovementInputState& MoveInputState = m_CustomMovementComponent->GetMovementInputState();

	OutMoveDirection = MoveInputState.WorldDirection;
	bOutHasMoveInput = MoveInputState.bHasInput;
}

//接地状態か
bool UPlayerAttackMotionComponent::IsGrounded() const
{
	if (!m_Player) return false;

	const UCharacterMovementComponent* MovementComponent = m_Player->GetCharacterMovement();

	return MovementComponent && MovementComponent->IsMovingOnGround();
}

//空中にいるか
bool UPlayerAttackMotionComponent::IsJumping() const
{
	return !IsGrounded();
}

//通常重力倍率を取得
float UPlayerAttackMotionComponent::GetDefaultGravityScale() const
{
	if (m_CustomMovementComponent)
	{
		if (const UPlayerMovementParameter* MovementParameter = m_CustomMovementComponent->GetMovementParameter())
		{
			return MovementParameter->GravityScale;
		}
	}

	return 1.f;
}
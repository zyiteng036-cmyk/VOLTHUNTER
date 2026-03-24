// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_MovementComponent.h"
#include "Player_CameraComponent.h"
#include "Player_EvasiveComponent.h"
#include "Player_AttackComponent.h"
#include "Player_ElectroGaugeComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../PlayerCharacter.h"

// Sets default values for this component's properties
UPlayer_MovementComponent::UPlayer_MovementComponent()
	:m_Player(nullptr)
	, m_CameraComponent(nullptr)
	, m_EvasiveComponent(nullptr)
	, m_AttackComponent(nullptr)
	, m_ElectroComponent(nullptr)
	, m_DashEffect(nullptr)
	, m_DashEffectComp(nullptr)
	, m_CurrentMoveInput(FVector2D::ZeroVector)
	, m_CanMovement(true)
	, m_IsIdle(false)
	, m_IsJump(false)
	, m_JumpHoldTime(0.f)
	, m_IsDash(false)
	, m_DashStart(false)
	, m_HasMoveInput(false)
	, m_DashDirection(FVector::ZeroVector)
	, m_IsDashDecelerating(false)
	, m_IsJumpCooldown(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayer_MovementComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player)		return;


	m_CameraComponent = m_Player->FindComponentByClass<UPlayer_CameraComponent>();
	if (!m_CameraComponent)return;

	m_EvasiveComponent = m_Player->FindComponentByClass<UPlayer_EvasiveComponent>();
	if (!m_EvasiveComponent)return;

	m_AttackComponent = m_Player->FindComponentByClass<UPlayer_AttackComponent>();
	if (!m_AttackComponent)return;

	m_ElectroComponent = m_Player->FindComponentByClass<UPlayer_ElectroGaugeComponent>();
	if (!m_ElectroComponent)return;
	//移動速度
	m_Player->GetCharacterMovement()->MaxWalkSpeed = PlayerParam.RunSpeed;
	m_Player->GetCharacterMovement()->GravityScale = PlayerParam.GravityScale;
	//空中操作
	m_Player->GetCharacterMovement()->AirControl = PlayerParam.AirControl;

}

void UPlayer_MovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_Player) return;


	//入力を減らす
	m_CurrentMoveInput *= 0.5f;


	// アイドル状態更新
	_updateIdle();

	// ダッシュ処理
	_updateDash(DeltaTime);

	//ジャンプ処理
	_updateJump(DeltaTime);



	//アイドル時キャラクターの回転速度を０に
	if (m_IsIdle)
	{
		m_Player->GetCharacterMovement()->RotationRate = FRotator::ZeroRotator;

	}
	//アイドル時以外
	else
	{
		m_Player->GetCharacterMovement()->RotationRate = FRotator(0.f, 1300.f, 0.f);
	}

	if (m_EvasiveComponent->GetIsJustEvasive())
	{
		m_Player->bUseControllerRotationYaw = false;
	}
}


//アイドル状態
void UPlayer_MovementComponent::_updateIdle()
{
	//移動しているかどうか
	bool bMoving = m_Player->GetVelocity().SizeSquared() > 30.f;
	//地上にいるかどうか
	bool bIsGround = m_Player->GetCharacterMovement()->IsMovingOnGround();

	//上記がtrueの場合アイドル状態とする
	m_IsIdle = (!bMoving && bIsGround);
}



//ダッシュ
void UPlayer_MovementComponent::_updateDash(float DeltaTime)
{
	if (!m_IsDash)return;

	FVector CurrentVelocity = m_Player->GetCharacterMovement()->Velocity;

	//入力がなくなったらダッシュ解除
	if (m_CurrentMoveInput.SizeSquared() < 0.1)
	{
		m_IsDashDecelerating = true;

		m_Player->GetCharacterMovement()->BrakingDecelerationWalking = 500.f;
		m_Player->GetCharacterMovement()->GroundFriction = 2.f;

		// 慣性の強さ（InterpSpeed）。
		float InertiaSpeed = 3.0f;

		FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, FVector::ZeroVector, DeltaTime, InertiaSpeed);

		// 実際に速度を適用する
		m_Player->GetCharacterMovement()->Velocity = NewVelocity;

		// 十分に速度が落ちたらダッシュ終了
		if (NewVelocity.SizeSquared() < 900.0f)
		{
			EndDash();
		}
		return; // 入力がない時はここで処理終了
	}
	// もし「減速モード中」に入力が入ったら、ダッシュを再開せず、ダッシュ自体を終わらせる
	if (m_IsDashDecelerating)
	{
		EndDash(); // これでRunSpeedに戻り、通常の移動入力処理(TickやInput_Move)に任せる
		return;
	}

	// ロックオン中はロックオンカメラのYawを使う
	FRotator DashRotation;
	if (m_CameraComponent->GetIsTargetLockedOn() && m_CameraComponent->GetLockOnCamera())
	{
		FRotator CameraRotation = m_CameraComponent->GetLockOnCamera()->GetComponentRotation();
		DashRotation = FRotator(0.f, CameraRotation.Yaw, 0.f);
	}
	else
	{
		DashRotation = GetControlRotationFlat();
	}

	FVector Forward = FRotationMatrix(DashRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(DashRotation).GetUnitAxis(EAxis::Y);

	FVector InputDirection = Forward * m_CurrentMoveInput.X + Right * m_CurrentMoveInput.Y;

	if (!InputDirection.IsNearlyZero())
		m_DashDirection = InputDirection.GetSafeNormal();

	m_Player->GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(
		m_Player->GetCharacterMovement()->MaxWalkSpeed,
		PlayerParam.DashSpeed,
		DeltaTime,
		2.0f // 補間速度。大きくするとすぐ2000に戻る
	);
}


//ジャンプ
void UPlayer_MovementComponent::_updateJump(float DeltaTime)
{
	if (!m_Player) return;

	auto* MoveComp = m_Player->GetCharacterMovement();
	if (!MoveComp) return;

	// ===== ジャンプ入力・チャージ処理 =====
	if (m_IsJumpCharging)
	{
		m_JumpHoldTime += DeltaTime;

	}

	// ===== 重力制御（ジャンプ物理） =====
	if (!MoveComp->IsMovingOnGround())
	{
		// 落下中
		if (MoveComp->Velocity.Z < 0.f)
		{
			MoveComp->GravityScale = PlayerParam.GravityScaleFall;
		}
		// 上昇中
		else
		{
			MoveComp->GravityScale = PlayerParam.GravityScale;
		}
	}
	else
	{
		// 地上に戻ったら必ず戻す
		MoveComp->GravityScale = PlayerParam.GravityScale;
	}
}

//----------入力コールバック------------
//移動
//前後入力
void UPlayer_MovementComponent::Input_MoveForward(const FInputActionValue& Value)
{
	if (!GetCanMovement())return;
	if (m_AttackComponent->GetIsInAttackState())return;
	if (m_EvasiveComponent->GetIsEvasive() || m_EvasiveComponent->GetIsJustEvasive())return;

	m_Player->OnActionCommitted(EPlayerActionCommit::Move);

	//入力値
	float ForwardAxis = Value.Get<float>();

	//入力がある場合
	if (FMath::Abs(ForwardAxis) > 0.05f)
	{
		m_CurrentMoveInput.X = ForwardAxis;
		m_HasMoveInput = true;
	}
	//入力がない場合
	else if (FMath::Abs(ForwardAxis) < 0.01)
	{
		m_CurrentMoveInput.X = 0.f;
		m_HasMoveInput = false;
	}

	//入力方向だけ受け取り攻撃中は移動しない
	if (m_AttackComponent->GetIsAttack())return;

	FRotator MoveRotation;

	if (m_CameraComponent->GetIsTargetLockedOn() && m_CameraComponent->GetLockOnCamera())
	{
		// ロックオン中はロックオンカメラのYawのみ使用
		FRotator CameraRotation = m_CameraComponent->GetLockOnCamera()->GetComponentRotation();
		MoveRotation = FRotator(0.f, CameraRotation.Yaw, 0.f);
	}
	else
	{
		// 通常時はControllerのYawのみ使用
		FRotator ControlRotation = GetControlRotationFlat();
		MoveRotation = ControlRotation;
	}
	//前後方向取得
	FVector ForwardFace = FRotationMatrix(MoveRotation).GetUnitAxis(EAxis::X);


	//プレイヤーの向きを取得
	//AddMovementInput(方向ベクトル, スティックの倒し具合);
	m_Player->AddMovementInput(ForwardFace, ForwardAxis);
}

//左右入力
void UPlayer_MovementComponent::Input_MoveRight(const FInputActionValue& Value)
{
	if (!GetCanMovement())return;
	if (m_AttackComponent->GetIsInAttackState())return;
	if (m_EvasiveComponent->GetIsEvasive() || m_EvasiveComponent->GetIsJustEvasive())return;

	m_Player->OnActionCommitted(EPlayerActionCommit::Move);


	//入力値
	float RightAxis = Value.Get<float>();

	//入力がある場合
	if (FMath::Abs(RightAxis) > 0.05f)
	{
		m_CurrentMoveInput.Y = RightAxis;
		m_HasMoveInput = true;
	}
	//入力がない場合
	else if (FMath::Abs(RightAxis) < 0.01)
	{
		m_CurrentMoveInput.Y = 0.f;
		m_HasMoveInput = false;
	}

	//入力方向だけ受け取り攻撃中は移動しない
	if (m_AttackComponent->GetIsAttack())return;


	FRotator MoveRotation;

	if (m_CameraComponent->GetIsTargetLockedOn() && m_CameraComponent->GetLockOnCamera())
	{
		FRotator CameraRotation = m_CameraComponent->GetLockOnCamera()->GetComponentRotation();
		MoveRotation = FRotator(0.f, CameraRotation.Yaw, 0.f);
	}
	else
	{
		FRotator ControlRot = GetControlRotationFlat();
		MoveRotation = ControlRot;
	}

	//左右方向取得
	FVector RightFace = FRotationMatrix(MoveRotation).GetUnitAxis(EAxis::Y);

	//プレイヤーの向きを取得
	//AddMovementInput(方向ベクトル, スティックの倒し具合);
	m_Player->AddMovementInput(RightFace, RightAxis);

}


//ジャンプ入力
void UPlayer_MovementComponent::Input_Jump(const FInputActionValue& Value)
{
	if (!GetCanMovement())return;
	if (!m_Player)return;
	if (m_AttackComponent->GetIsInAttackState())return;
	if (m_AttackComponent->GetIsAttack())return;

	//回避中禁止
	if (m_EvasiveComponent->GetIsEvasive() || m_EvasiveComponent->GetIsJustEvasive())return;

	//着地後クールタイム
	if (m_IsJumpCooldown)return;

	//二段ジャンプを防ぐ
	if (!m_Player->GetCharacterMovement()->IsMovingOnGround())return;

	m_Player->OnActionCommitted(EPlayerActionCommit::Jump);

	m_AttackComponent->OnJumpStarted();
	m_EvasiveComponent->SetCanEvasive(false);

	StartJump(PlayerParam.LongJumpZVelocity);

	m_IsJumpCharging = true;
	m_JumpHoldTime = 0.f;

	m_IsJump = true;
}

void UPlayer_MovementComponent::Input_JumpRelease()
{
	if (!m_IsJumpCharging) return;

	if (m_JumpHoldTime < PlayerParam.LongJumpThreshold)
	{
		if (m_Player->GetCharacterMovement()->Velocity.Z > 0.f)
		{
			//上昇速度を直接カットして書き戻す
			FVector CurrentVelocity = m_Player->GetCharacterMovement()->Velocity;

			// 上昇力を35%に減衰（好きな値に調整可）
			CurrentVelocity.Z *= 0.35f;

			// 最低保証（これ以上低くはしない）
			if (CurrentVelocity.Z < PlayerParam.ShortJumpZVelocity)
			{
				CurrentVelocity.Z = PlayerParam.ShortJumpZVelocity;
			}

			// 速度を適用
			m_Player->GetCharacterMovement()->Velocity = CurrentVelocity;
		}
	}

	m_IsJumpCharging = false;
}

void UPlayer_MovementComponent::StartJump(float _JumpZVelocity)
{
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		FVector Velocity = MoveComp->Velocity;

		// Z速度を完全に上書き（加算ではないため、前の慣性が残らない）
		Velocity.Z = _JumpZVelocity;

		MoveComp->Velocity = Velocity;

		// 強制的に空中状態へ移行（接地判定を切る）
		MoveComp->SetMovementMode(MOVE_Falling);
	}
}

void UPlayer_MovementComponent::EndDash()
{
	if (!m_IsDash) return;

	m_IsDash = false;

	// ダッシュ終了時に通常の移動速度に戻す
	m_Player->GetCharacterMovement()->MaxWalkSpeed = PlayerParam.RunSpeed;
	m_Player->GetCharacterMovement()->BrakingDecelerationWalking = 2048.f; // UEデフォルト値付近
	m_Player->GetCharacterMovement()->GroundFriction = 8.f;               // UEデフォルト値付近
	m_Player->GetCharacterMovement()->AirControl = PlayerParam.AirControl;
	if (m_DashEffectComp)
	{
		m_DashEffectComp->Deactivate();
	}

}

//ダッシュ入力
void UPlayer_MovementComponent::Input_Dash(const FInputActionValue& Value)
{
	if (!GetCanMovement())return;

	if (m_IsDash)
	{
		EndDash();
		return;
	}

	//アイドル時はダッシュ禁止
	if (m_CurrentMoveInput.SizeSquared() < 0.1f)
	{
		return;
	}

	//ジャンプ中はダッシュ禁止
	if (!m_Player->GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	m_Player->OnActionCommitted(EPlayerActionCommit::Move);

	// ロックオン中はロックオンカメラのYawを使う
	//ダッシュ方向の回転
	FRotator DashRotation;
	//ロックオン中のカメラ
	if (m_CameraComponent->GetIsTargetLockedOn() && m_CameraComponent->GetLockOnCamera())
	{
		//カメラの水平成分Yaw
		FRotator CameraRotation = m_CameraComponent->GetLockOnCamera()->GetComponentRotation();
		DashRotation = FRotator(0.f, CameraRotation.Yaw, 0.f);
	}
	//通常のダッシュ方向
	else
	{
		DashRotation = GetControlRotationFlat();
	}

	//前方向ベクトルと右方向ベクトルの取得
	FVector Forward = FRotationMatrix(DashRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(DashRotation).GetUnitAxis(EAxis::Y);

	//カメラの向きの移動方向
	FVector MoveDirection = Forward * m_CurrentMoveInput.X + Right * m_CurrentMoveInput.Y;

	if (MoveDirection.IsNearlyZero())
	{
		MoveDirection = Forward;
	}

	m_DashDirection = MoveDirection.GetSafeNormal();
	m_IsDash = true;
	m_DashStart = true;

	m_IsDashDecelerating = false;

	m_Player->GetCharacterMovement()->AirControl = PlayerParam.AirControlDash;
	//スタートダッシュ
	m_Player->LaunchCharacter(m_DashDirection * PlayerParam.StartDashSpeed, true, true);
	
	if (!m_DashEffect || !GetOwner())return;

	//ダッシュエフェクト開始
	if (!m_DashEffectComp)
	{
		m_DashEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			m_DashEffect,
			m_Player->GetMesh(),         
			FName(NAME_None),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false                      
		);
	}
	else
	{
		m_DashEffectComp->Activate(true);
	}


}


//移動のための平面成分
FRotator UPlayer_MovementComponent::GetControlRotationFlat() const
{
	FRotator ControlRotator = m_Player->Controller->GetControlRotation();
	ControlRotator.Pitch = 0.f;
	ControlRotator.Roll = 0.f;

	return ControlRotator;
}

//入力があるかどうか
bool UPlayer_MovementComponent::GetMoveInputDirection(FVector& Direction) const
{
	//入力がなければfalse
	if (m_CurrentMoveInput.SizeSquared() < 0.1f) return false;

	FRotator BaseRotation;

	if (m_CameraComponent->GetIsTargetLockedOn() && m_CameraComponent->GetLockOnCamera())
	{
		FRotator CameraRotation = m_CameraComponent->GetLockOnCamera()->GetComponentRotation();

		BaseRotation = FRotator(0.f, CameraRotation.Yaw, 0.f);
	}
	else
	{
		BaseRotation = GetControlRotationFlat();
	}

	const FVector Forward = FRotationMatrix(BaseRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(BaseRotation).GetUnitAxis(EAxis::Y);

	Direction = (Forward * m_CurrentMoveInput.X + Right * m_CurrentMoveInput.Y).GetSafeNormal();

	return !Direction.IsNearlyZero();
}


//プレイヤーが動こうとしている方向
bool UPlayer_MovementComponent::GetDesiredMoveDirection(FVector& OutDirection) const
{
	// 入力がある場合は入力方向を最優先
	if (GetMoveInputDirection(OutDirection))
	{
		return true;
	}

	// 入力がない場合は現在の向きを基準にする
	OutDirection = m_Player->GetActorForwardVector();
	return false;
}


void UPlayer_MovementComponent::OnJustEvasive(const AActor* Target)
{
	if (!Target || !m_Player) return;

	FVector Direction = Target->GetActorLocation() - m_Player->GetActorLocation();
	Direction.Z = 0.f;

	if (!Direction.Normalize())return;

	FRotator ForwardRotation = Direction.Rotation();

	m_Player->SetActorRotation(ForwardRotation);

	// 自動回転に負けないよう一時的に制御
	m_Player->bUseControllerRotationYaw = true;
}

void UPlayer_MovementComponent::OnPlayerLanded()
{
	// ジャンプフラグを下ろす
	m_IsJump = false;

	// ジャンプ入力を禁止するフラグを立てる
	m_IsJumpCooldown = true;

	// 0.1秒後に OnJumpCooldownFinished を呼び出してフラグを解除する
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			JumpCooldownTimerHandle,
			this,
			&UPlayer_MovementComponent::OnJumpCooldownFinished,
			0.2f, //遅延時間
			false
		);
	}
}

void UPlayer_MovementComponent::OnJumpCooldownFinished()
{
	m_IsJumpCooldown = false;
}

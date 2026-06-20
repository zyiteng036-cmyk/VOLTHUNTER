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

//===マジックナンバー排除用の定数===
namespace MovementConstants
{
	constexpr float IdleVelocitySqThreshold = 30.0f;       //アイドル判定とする速度の二乗
	constexpr float MoveRotationRateYaw = 1300.0f;         //移動時のキャラの旋回速度
	constexpr float DashDecelBraking = 500.0f;             //ダッシュ終了時のブレーキ力
	constexpr float DashDecelFriction = 2.0f;              //ダッシュ終了時の摩擦力
	constexpr float DashInertiaInterpSpeed = 3.0f;         //ダッシュ終了時の慣性補間速度
	constexpr float DashEndVelocitySqThreshold = 900.0f;   //この速度以下になればダッシュ完全終了
	constexpr float JumpCooldownTime = 0.2f;               //着地後のジャンプクールタイム
	constexpr float JumpShortReleaseMultiplier = 0.35f;    //ジャンプボタン短押し時の上昇減衰率
	constexpr float InputDeadZoneSq = 0.1f;                //入力のデッドゾーン（二乗）
	constexpr float InputDecayRate = 0.5f;                 //入力の減衰率
	constexpr float InputAxisThreshold = 0.05f;            //入力ありと判定する閾値
	constexpr float InputAxisStopThreshold = 0.01f;        //入力なしと判定する閾値
	constexpr float DashSpeedInterpRate = 2.0f;            //ダッシュ速度の補間速度
	constexpr float DefaultBrakingDeceleration = 2048.0f;  //デフォルトのブレーキ力
	constexpr float DefaultGroundFriction = 8.0f;          //デフォルトの摩擦力
}

UPlayer_MovementComponent::UPlayer_MovementComponent()
{
	//毎フレームのTick処理を有効化する設定
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayer_MovementComponent::BeginPlay()
{
	//親クラスのBeginPlayを呼び出す
	Super::BeginPlay();

	//オーナーアクターをプレイヤーキャラクターとしてキャストして取得
	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player) return;

	//各必須コンポーネントを検索して参照を保持
	m_CameraComponent = m_Player->FindComponentByClass<UPlayer_CameraComponent>();
	m_EvasiveComponent = m_Player->FindComponentByClass<UPlayer_EvasiveComponent>();
	m_AttackComponent = m_Player->FindComponentByClass<UPlayer_AttackComponent>();
	m_ElectroComponent = m_Player->FindComponentByClass<UPlayer_ElectroGaugeComponent>();

	//初期の移動速度や重力などのパラメータ設定
	m_Player->GetCharacterMovement()->MaxWalkSpeed = m_PlayerParam.RunSpeed;
	m_Player->GetCharacterMovement()->GravityScale = m_PlayerParam.GravityScale;
	//空中操作の制御
	m_Player->GetCharacterMovement()->AirControl = m_PlayerParam.AirControl;
}

void UPlayer_MovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_Player) return;

	//入力を滑らかに減衰させる
	m_CurrentMoveInput *= MovementConstants::InputDecayRate;

	//アイドル状態更新
	_updateIdle();

	//ダッシュ処理
	_updateDash(DeltaTime);

	//ジャンプ処理
	_updateJump(DeltaTime);

	//アイドル時キャラクターの回転速度を0に設定
	if (m_IsIdle)
	{
		m_Player->GetCharacterMovement()->RotationRate = FRotator::ZeroRotator;
	}
	//アイドル時以外は旋回速度を設定
	else
	{
		m_Player->GetCharacterMovement()->RotationRate = FRotator(0.0f, MovementConstants::MoveRotationRateYaw, 0.0f);
	}

	//ジャスト回避中はプレイヤーのYaw自動回転を無効化
	if (m_EvasiveComponent->GetIsJustEvasive())
	{
		m_Player->bUseControllerRotationYaw = false;
	}
}

void UPlayer_MovementComponent::_updateIdle()
{
	if (!m_Player) return;

	//移動しているかどうか
	bool bMoving = m_Player->GetVelocity().SizeSquared() > MovementConstants::IdleVelocitySqThreshold;
	//地上にいるかどうか
	bool bIsGround = m_Player->GetCharacterMovement()->IsMovingOnGround();

	//上記が条件を満たす場合アイドル状態とする
	m_IsIdle = (!bMoving && bIsGround);
}

void UPlayer_MovementComponent::_updateDash(float DeltaTime)
{
	if (!m_IsDash) return;

	FVector CurrentVelocity = m_Player->GetCharacterMovement()->Velocity;

	//入力がなくなったらダッシュ解除処理へ
	if (m_CurrentMoveInput.SizeSquared() < MovementConstants::InputDeadZoneSq)
	{
		m_IsDashDecelerating = true;

		//減速時の摩擦とブレーキ力を設定
		m_Player->GetCharacterMovement()->BrakingDecelerationWalking = MovementConstants::DashDecelBraking;
		m_Player->GetCharacterMovement()->GroundFriction = MovementConstants::DashDecelFriction;

		//速度を徐々に0に向けて補間
		FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, FVector::ZeroVector, DeltaTime, MovementConstants::DashInertiaInterpSpeed);

		//実際に速度を適用する
		m_Player->GetCharacterMovement()->Velocity = NewVelocity;

		//十分に速度が落ちたらダッシュ完全終了
		if (NewVelocity.SizeSquared() < MovementConstants::DashEndVelocitySqThreshold)
		{
			EndDash();
		}
		return; //入力がない時はここで処理終了
	}

	//もし「減速モード中」に入力が入ったら、ダッシュを再開せずダッシュ自体を終わらせる
	if (m_IsDashDecelerating)
	{
		EndDash(); //これでRunSpeedに戻り通常の移動入力処理へ移行
		return;
	}

	//ダッシュ中の向きを取得
	FRotator DashRotation = GetControlRotationFlat();

	FVector Forward = FRotationMatrix(DashRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(DashRotation).GetUnitAxis(EAxis::Y);

	//入力方向の計算
	FVector InputDirection = Forward * m_CurrentMoveInput.X + Right * m_CurrentMoveInput.Y;

	//入力があればダッシュ方向を更新
	if (!InputDirection.IsNearlyZero())
	{
		m_DashDirection = InputDirection.GetSafeNormal();
	}

	//ダッシュ速度への補間
	m_Player->GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(
		m_Player->GetCharacterMovement()->MaxWalkSpeed,
		m_PlayerParam.DashSpeed,
		DeltaTime,
		MovementConstants::DashSpeedInterpRate //補間速度
	);
}

void UPlayer_MovementComponent::_updateJump(float DeltaTime)
{
	if (!m_Player) return;

	UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement();
	if (!MoveComp) return;

	//===== ジャンプ入力・チャージ処理 =====
	if (m_IsJumpCharging)
	{
		m_JumpHoldTime += DeltaTime;
	}

	//===== 重力制御（ジャンプ物理） =====
	if (!MoveComp->IsMovingOnGround())
	{
		//落下中
		if (MoveComp->Velocity.Z < 0.0f)
		{
			MoveComp->GravityScale = m_PlayerParam.GravityScaleFall;
		}
		//上昇中
		else
		{
			MoveComp->GravityScale = m_PlayerParam.GravityScale;
		}
	}
	else
	{
		//地上に戻ったら必ず基本の重力スケールに戻す
		MoveComp->GravityScale = m_PlayerParam.GravityScale;
	}
}

//----------入力コールバック------------

void UPlayer_MovementComponent::Input_MoveForward(const FInputActionValue& Value)
{
	if (!GetCanMovement()) return;
	if (m_AttackComponent->GetIsInAttackState()) return;
	if (m_EvasiveComponent->GetIsEvasive() || m_EvasiveComponent->GetIsJustEvasive()) return;

	m_Player->OnActionCommitted(EPlayerActionCommit::Move);

	//入力値の取得
	float ForwardAxis = Value.Get<float>();

	//入力がある場合
	if (FMath::Abs(ForwardAxis) > MovementConstants::InputAxisThreshold)
	{
		m_CurrentMoveInput.X = ForwardAxis;
		m_HasMoveInput = true;
	}
	//入力がない場合
	else if (FMath::Abs(ForwardAxis) < MovementConstants::InputAxisStopThreshold)
	{
		m_CurrentMoveInput.X = 0.0f;
		m_HasMoveInput = false;
	}

	//入力方向だけ受け取り攻撃中は移動しない
	if (m_AttackComponent->GetIsAttack()) return;

	//通常時はControllerのYawのみ使用
	FRotator MoveRotation = GetControlRotationFlat();
	//前後方向取得
	FVector ForwardFace = FRotationMatrix(MoveRotation).GetUnitAxis(EAxis::X);

	//プレイヤーの向きを取得して移動入力を追加
	m_Player->AddMovementInput(ForwardFace, ForwardAxis);
}

void UPlayer_MovementComponent::Input_MoveRight(const FInputActionValue& Value)
{
	if (!GetCanMovement()) return;
	if (m_AttackComponent->GetIsInAttackState()) return;
	if (m_EvasiveComponent->GetIsEvasive() || m_EvasiveComponent->GetIsJustEvasive()) return;

	m_Player->OnActionCommitted(EPlayerActionCommit::Move);

	//入力値の取得
	float RightAxis = Value.Get<float>();

	//入力がある場合
	if (FMath::Abs(RightAxis) > MovementConstants::InputAxisThreshold)
	{
		m_CurrentMoveInput.Y = RightAxis;
		m_HasMoveInput = true;
	}
	//入力がない場合
	else if (FMath::Abs(RightAxis) < MovementConstants::InputAxisStopThreshold)
	{
		m_CurrentMoveInput.Y = 0.0f;
		m_HasMoveInput = false;
	}

	//入力方向だけ受け取り攻撃中は移動しない
	if (m_AttackComponent->GetIsAttack()) return;

	FRotator MoveRotation = GetControlRotationFlat();
	//左右方向取得
	FVector RightFace = FRotationMatrix(MoveRotation).GetUnitAxis(EAxis::Y);

	//プレイヤーの向きを取得して移動入力を追加
	m_Player->AddMovementInput(RightFace, RightAxis);
}

void UPlayer_MovementComponent::Input_Jump(const FInputActionValue& Value)
{
	if (!GetCanMovement()) return;
	if (!m_Player) return;
	if (m_AttackComponent->GetIsInAttackState()) return;
	if (m_AttackComponent->GetIsAttack()) return;

	//回避中禁止
	if (m_EvasiveComponent->GetIsEvasive() || m_EvasiveComponent->GetIsJustEvasive()) return;

	//着地後クールタイム中は禁止
	if (m_IsJumpCooldown) return;

	//二段ジャンプを防ぐ
	if (!m_Player->GetCharacterMovement()->IsMovingOnGround()) return;

	m_Player->OnActionCommitted(EPlayerActionCommit::Jump);

	m_AttackComponent->OnJumpStarted();
	m_EvasiveComponent->SetCanEvasive(false);

	//ジャンプ開始
	StartJump(m_PlayerParam.LongJumpZVelocity);

	m_IsJumpCharging = true;
	m_JumpHoldTime = 0.0f;
	m_IsJump = true;
}

void UPlayer_MovementComponent::Input_JumpRelease()
{
	if (!m_IsJumpCharging) return;

	//短押しの場合の上昇減衰処理
	if (m_JumpHoldTime < m_PlayerParam.LongJumpThreshold)
	{
		if (m_Player->GetCharacterMovement()->Velocity.Z > 0.0f)
		{
			//上昇速度を直接カットして書き戻す
			FVector CurrentVelocity = m_Player->GetCharacterMovement()->Velocity;

			//上昇力を減衰
			CurrentVelocity.Z *= MovementConstants::JumpShortReleaseMultiplier;

			//最低保証速度
			if (CurrentVelocity.Z < m_PlayerParam.ShortJumpZVelocity)
			{
				CurrentVelocity.Z = m_PlayerParam.ShortJumpZVelocity;
			}

			//速度を適用
			m_Player->GetCharacterMovement()->Velocity = CurrentVelocity;
		}
	}

	m_IsJumpCharging = false;
}

void UPlayer_MovementComponent::StartJump(float JumpZVelocity)
{
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		FVector Velocity = MoveComp->Velocity;

		//Z速度を完全に上書き
		Velocity.Z = JumpZVelocity;
		MoveComp->Velocity = Velocity;

		//強制的に空中状態へ移行（接地判定を切る）
		MoveComp->SetMovementMode(MOVE_Falling);
	}
}

void UPlayer_MovementComponent::EndDash()
{
	if (!m_IsDash) return;

	m_IsDash = false;

	//ダッシュ終了時に通常の移動パラメータに戻す
	m_Player->GetCharacterMovement()->MaxWalkSpeed = m_PlayerParam.RunSpeed;
	m_Player->GetCharacterMovement()->BrakingDecelerationWalking = MovementConstants::DefaultBrakingDeceleration;
	m_Player->GetCharacterMovement()->GroundFriction = MovementConstants::DefaultGroundFriction;
	m_Player->GetCharacterMovement()->AirControl = m_PlayerParam.AirControl;

	if (m_DashEffectComp)
	{
		m_DashEffectComp->Deactivate();
	}
}

void UPlayer_MovementComponent::Input_Dash(const FInputActionValue& Value)
{
	if (!GetCanMovement()) return;

	//既にダッシュ中なら終了する
	if (m_IsDash)
	{
		EndDash();
		return;
	}

	//アイドル時はダッシュ禁止
	if (m_CurrentMoveInput.SizeSquared() < MovementConstants::InputDeadZoneSq)
	{
		return;
	}

	//ジャンプ中はダッシュ禁止
	if (!m_Player->GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	m_Player->OnActionCommitted(EPlayerActionCommit::Move);

	//ダッシュ方向の回転を取得
	FRotator DashRotation = GetControlRotationFlat();

	//前方向ベクトルと右方向ベクトルの取得
	FVector Forward = FRotationMatrix(DashRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(DashRotation).GetUnitAxis(EAxis::Y);

	//カメラの向きに基づいた移動方向
	FVector MoveDirection = Forward * m_CurrentMoveInput.X + Right * m_CurrentMoveInput.Y;

	//入力方向がほぼ0なら前方を向く
	if (MoveDirection.IsNearlyZero())
	{
		MoveDirection = Forward;
	}

	m_DashDirection = MoveDirection.GetSafeNormal();
	m_IsDash = true;
	m_DashStart = true;
	m_IsDashDecelerating = false;

	//ダッシュ時の空中操作パラメータを適用
	m_Player->GetCharacterMovement()->AirControl = m_PlayerParam.AirControlDash;

	//スタートダッシュの打ち出し
	m_Player->LaunchCharacter(m_DashDirection * m_PlayerParam.StartDashSpeed, true, true);

	if (!m_DashEffect || !GetOwner()) return;

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

bool UPlayer_MovementComponent::GetIsMoveInput() const
{
	//入力の大きさをマジックナンバーに依存せず判定
	return m_CurrentMoveInput.SizeSquared() >= MovementConstants::InputDeadZoneSq;
}

FRotator UPlayer_MovementComponent::GetControlRotationFlat() const
{
	//コントローラーの回転を取得し、PitchとRollを無効化して平面の回転のみにする
	FRotator ControlRotator = m_Player->Controller->GetControlRotation();
	ControlRotator.Pitch = 0.0f;
	ControlRotator.Roll = 0.0f;

	return ControlRotator;
}

bool UPlayer_MovementComponent::GetMoveInputDirection(FVector& Direction) const
{
	//入力がなければfalseを返す
	if (m_CurrentMoveInput.SizeSquared() < MovementConstants::InputDeadZoneSq) return false;

	FRotator BaseRotation = GetControlRotationFlat();

	const FVector Forward = FRotationMatrix(BaseRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(BaseRotation).GetUnitAxis(EAxis::Y);

	//入力に基づくベクトルを算出し正規化
	Direction = (Forward * m_CurrentMoveInput.X + Right * m_CurrentMoveInput.Y).GetSafeNormal();

	return !Direction.IsNearlyZero();
}

bool UPlayer_MovementComponent::GetDesiredMoveDirection(FVector& OutDirection) const
{
	//入力がある場合は入力方向を最優先する
	if (GetMoveInputDirection(OutDirection))
	{
		return true;
	}

	//入力がない場合は現在の向きを基準にする
	OutDirection = m_Player->GetActorForwardVector();
	return false;
}

void UPlayer_MovementComponent::OnJustEvasive(const AActor* Target)
{
	if (!Target || !m_Player) return;

	//対象との方向ベクトルを算出
	FVector Direction = Target->GetActorLocation() - m_Player->GetActorLocation();
	Direction.Z = 0.0f;

	//正規化できない場合は処理終了
	if (!Direction.Normalize()) return;

	//対象の方向を向かせる
	FRotator ForwardRotation = Direction.Rotation();
	m_Player->SetActorRotation(ForwardRotation);

	//自動回転に負けないよう一時的に制御
	m_Player->bUseControllerRotationYaw = true;
}

void UPlayer_MovementComponent::OnPlayerLanded()
{
	//ジャンプフラグを下ろす
	m_IsJump = false;

	//ジャンプ入力を禁止するフラグを立てる
	m_IsJumpCooldown = true;

	//一定時間後にOnJumpCooldownFinishedを呼び出してフラグを解除する
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			m_JumpCooldownTimerHandle,
			this,
			&UPlayer_MovementComponent::OnJumpCooldownFinished,
			MovementConstants::JumpCooldownTime, //遅延時間
			false
		);
	}
}

void UPlayer_MovementComponent::OnJumpCooldownFinished()
{
	m_IsJumpCooldown = false;
}
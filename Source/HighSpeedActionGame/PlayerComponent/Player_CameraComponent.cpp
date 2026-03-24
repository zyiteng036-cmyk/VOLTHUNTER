// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_CameraComponent.h"
#include "Player_MovementComponent.h"
#include "Player_AttackComponent.h"
#include "Player_EvasiveComponent.h"
#include "../PlayerCharacter.h"
#include "../Event/GameplayAreaEventManager/GameplayAreaEventManager.h"
#include "../SequenceWorldSubsystem/SequenceWorldSubsystem.h"
#include "../Enemy/EnemyBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"   
#include "Kismet/KismetMathLibrary.h"
#include "../PlayerSettingsSubsystem/PlayerSettingsSubsystem.h"



// Sets default values for this component's properties
UPlayer_CameraComponent::UPlayer_CameraComponent()
	:m_Player(nullptr)
	, m_EnemyDirectionInterpSpeed(15.f)
	, m_IsEnemyDirectionLooking(false)
	, m_IsTargetLockOn(false)
	, m_MinPitch(-50.f)
	, m_MaxPitch(20.f)
	, m_LastCamerainputTime(0.f)
	, m_CameraAutoDelay(1.f)
	, m_CameraReturnSpeed(0.8f)
	, m_HeavyChargeArmLength(250.f)
	, m_HeavyChargeInterpSpeed(6.f)
	, m_SkillZoomInArmLength(200.f)
	, m_SkillZoomOutArmLength(800.f)
	, m_SkillZoomInSpeed(10.f)
	, m_SkillZoomOutSpeed(6.f)
	, m_IsClearCamera(false)
	, m_ClearCurrentArmLength(0.f)
	, m_ClearTargetArmLength(0.f)
	, m_ClearZoomInterpSpeed(0.3f)
	, m_ClearZoomLength(300.f)
	, m_DefaultArmLength(0.f)
	, m_DefaultUsePawnControlRotation(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UPlayer_CameraComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UPlayerSettingsSubsystem* SettingsSubsystem = GameInstance->GetSubsystem<UPlayerSettingsSubsystem>())
		{
			//サブシステムに保存されているスライダーの値をそのまま代入！
			PlayerParam.CameraSensitivity = SettingsSubsystem->GetCameraSensitivity();

			//リバース設定
			m_bInvertCameraX = SettingsSubsystem->GetInvertCameraX();
			m_bInvertCameraY = SettingsSubsystem->GetInvertCameraY();
		}
	}

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player)		return;

	m_MovementComponent = m_Player->FindComponentByClass<UPlayer_MovementComponent>();
	if (!m_MovementComponent)return;

	m_AttackComponent = m_Player->FindComponentByClass<UPlayer_AttackComponent>();
	if (!m_AttackComponent)return;

	m_EvasiveComponent = m_Player->FindComponentByClass<UPlayer_EvasiveComponent>();
	if (!m_EvasiveComponent)return;

	m_PlayerController = Cast<APlayerController>(m_Player->GetController());
	if (!m_PlayerController.IsValid())return;


	// Player の Root として Capsule を使用
	USceneComponent* PlayerRoot = m_Player->GetRootComponent();
	if (!PlayerRoot)return;

	m_SpringArm = m_Player->m_SpringArm;
	m_Camera = m_Player->m_Camera;
	m_LockOnSpringArm = m_Player->m_LockOnSpringArm;
	m_LockOnCamera = m_Player->m_LockOnCamera;

	//Blueprint適用後の最終値の設定
	//カメラ距離
	m_SpringArm->TargetArmLength = PlayerParam.CameraLength;

	m_CameraFocusLocation = m_Player->GetActorLocation();
	m_CameraFocusVelocity = FVector::ZeroVector;


	// 管理クラスを生成して初期化
	OverrideHandler = NewObject<UCameraOverrideHandler>(this);
	OverrideHandler->Initialize(m_SpringArm, m_Camera);

	// OpeningHandlerの初期化
	if (OpeningHandler)
	{
		OpeningHandler->Initialize(m_SpringArm, m_Camera, m_PlayerController.Get());
	}
	m_IsOpeningCamera = false;

	if (DieHandler)
	{
		DieHandler->Initialize(m_SpringArm, m_PlayerController.Get());
	}



	// ActionHandler生成
	ActionHandler = NewObject<UCameraActionHandler>(this);
	if (ActionHandler)
	{
		// ポインタを渡す
		ActionHandler->Initialize(m_Player, m_PlayerController.Get(), m_SpringArm, m_Camera, m_MovementComponent);

		// パラメータをコピー（BPで設定した値をハンドラに渡す）
		ActionHandler->SetupParams(PlayerParam.CameraLength, PlayerParam.DefaultFOV, PlayerParam.DashStartArmLength, PlayerParam.DashStartFOV, PlayerParam.DashMidArmLength, PlayerParam.DashMidFOV);
	}
	if (BossHandler)
	{
		BossHandler->Initialize(m_SpringArm, m_Camera);
	}

	if (UWorld* World = GetWorld())
	{
		// サブシステム取得
		if (UGameplayAreaEventManager* EventManager = World->GetSubsystem<UGameplayAreaEventManager>())
		{
			// 死亡時（終了時）もバインドしておく
			EventManager->OnBossDead.AddDynamic(this, &UPlayer_CameraComponent::OnBossBattleEnd);
		}

		if (USequenceWorldSubsystem* SequenceSub = World->GetSubsystem<USequenceWorldSubsystem>())
		{
			SequenceSub->OnBossSequenceFinished.AddDynamic(this, &UPlayer_CameraComponent::OnBossBattleStart);
		}
	}

	if (m_SpringArm)
	{
		m_DefaultSocketOffset = m_SpringArm->SocketOffset;
		m_TargetSocketOffset = m_DefaultSocketOffset;
	}
}

// Called every frame
void UPlayer_CameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (DieHandler && DieHandler->IsActive())
	{
		return;
	}


	//クリア時は他を動かさない
	if (m_IsClearCamera)
	{
		_updateClearCameraZoom(DeltaTime);
		return;
	}

	// オープニング演出中
	if (m_IsOpeningCamera)
	{
		if (OpeningHandler)
		{
			// 更新処理
			bool bIsStillRunning = OpeningHandler->UpdateOpening(DeltaTime);

			// 演出が終わった瞬間
			if (!bIsStillRunning)
			{
				m_IsOpeningCamera = false;

				if (m_Player)
				{
					m_Player->EnableControl();
				}
			}
		}
		else
		{
			// エラー対策：Handlerがない場合も復帰させる
			m_IsOpeningCamera = false;
			if (m_Player)
			{
				m_Player->EnableControl();
			}
		}
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 敵方向補正などは残してもOK
	_updateEnemyDirectionCamera(DeltaTime);


	if (!m_IsTargetLockOn && !m_IsClearCamera)
	{
		_updateCameraFocus(DeltaTime);
		m_SpringArm->SetWorldLocation(m_CameraFocusLocation);
	}

	if (BossHandler && BossHandler->IsActive())
	{
		BossHandler->UpdateBossCamera(DeltaTime);

		return;
	}

	// ロックオン処理（Handlerに丸投げ）
	//if (m_IsTargetLockOn)
	//{
	//	if (LockOnHandler)
	//	{
	//		LockOnHandler->UpdateLockOn(DeltaTime);
	//	}
	//}

	bool bIsSkillActive = (m_SkillCameraPhase != ESkillCameraPhase::None);

	if (!m_IsEnemyDirectionLooking && !m_IsClearCamera && !bIsSkillActive) // ← !bIsSkillActive を追加
	{
		if (ActionHandler)
		{
			ActionHandler->UpdateActionCamera(DeltaTime);
		}
	}

	//強攻撃チャージカメラ（ここに入れる）
	if (_updateHeavyChargeCamera(DeltaTime))
	{
		return; // 他の距離制御をさせない
	}

	if (_updateSkillCamera(DeltaTime))
	{
		return;
	}

	//敵方向補正
	if (OverrideHandler && OverrideHandler->UpdateOverride(DeltaTime))
	{
		return;
	}

	//_updateDashCamera(DeltaTime);

	_updateAutoCameraReturn(DeltaTime);

	if (m_SpringArm)
	{
		m_SpringArm->SocketOffset = FMath::VInterpTo(
			m_SpringArm->SocketOffset,
			m_TargetSocketOffset,
			DeltaTime,
			m_SideOffsetInterpSpeed
		);
	}
}

void UPlayer_CameraComponent::_updateCameraFocus(float DeltaTime)
{
	const FVector PlayerLocation = m_Player->GetActorLocation();
	FVector ToPlayer = PlayerLocation - m_CameraFocusLocation;

	const float DeadZoneRadius = 50.f;
	const float ZFollowThreshold = 30.f;

	//デッドゾーン内なら何もしない
	if (ToPlayer.Size2D() < DeadZoneRadius && FMath::Abs(ToPlayer.Z) < ZFollowThreshold) { return; }
	//はみ出た分だけ追従
	FVector Target = PlayerLocation - ToPlayer.GetSafeNormal2D() * DeadZoneRadius;

	//曲線補間
	m_CameraFocusLocation = SmoothDamp(
		m_CameraFocusLocation,
		Target,
		m_CameraFocusVelocity,
		GetCameraFollowTime(),
		DeltaTime
	);
}




//カメラが一定時間たつと自動で戻る
void UPlayer_CameraComponent::_updateAutoCameraReturn(float DeltaTime)
{
}


//敵の方向へカメラを向ける
void UPlayer_CameraComponent::_updateEnemyDirectionCamera(float DeltaTime)
{
	if (!m_IsEnemyDirectionLooking || !m_EnemyTarget) return;

	//カメラ入力ある場合は自動回転しない
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - m_LastCamerainputTime < 0.1f)
	{
		m_IsEnemyDirectionLooking = false;
		m_EnemyTarget = nullptr;
		return;
	}

	FVector CamLocation = m_Camera->GetComponentLocation();
	FVector TargetLocation = m_EnemyTarget->GetActorLocation();
	TargetLocation.Z = m_Player->GetActorLocation().Z;

	FRotator TargetRotation = (TargetLocation - CamLocation).Rotation();
	FRotator NewRotation = FMath::RInterpTo(
		m_PlayerController->GetControlRotation(),
		TargetRotation,
		DeltaTime,
		m_EnemyDirectionInterpSpeed
	);

	m_PlayerController->SetControlRotation(NewRotation);

	// ほぼ向いたら終了
	if (NewRotation.Equals(TargetRotation, 2.0f))
	{
		m_IsEnemyDirectionLooking = false;
		m_EnemyTarget = nullptr;
	}
}

bool UPlayer_CameraComponent::_updateHeavyChargeCamera(float DeltaTime)
{
	if (!m_AttackComponent)return false;

	//強攻撃チャージ中でないなら処理なし
	if (!m_AttackComponent->GetIsHeavyCharging())
	{
		return false;
	}
	//チャージ中
	m_SpringArm->TargetArmLength = FMath::FInterpTo(m_SpringArm->TargetArmLength, m_HeavyChargeArmLength, DeltaTime, m_HeavyChargeInterpSpeed);
	return true;
}

bool UPlayer_CameraComponent::_updateSkillCamera(float DeltaTime)
{
	switch (m_SkillCameraPhase)
	{
	case ESkillCameraPhase::ZoomIn:
	{
		m_SpringArm->TargetArmLength = FMath::FInterpTo(m_SpringArm->TargetArmLength, m_SkillZoomInArmLength, DeltaTime, m_SkillZoomInSpeed);
		if (FMath::IsNearlyEqual(m_SpringArm->TargetArmLength, m_SkillZoomInArmLength, 1.f))
		{
			m_SkillCameraPhase = ESkillCameraPhase::ZoomOut;
		}
		return true;
	}

	case ESkillCameraPhase::ZoomOut:
	{
		m_SpringArm->TargetArmLength = FMath::FInterpTo(m_SpringArm->TargetArmLength, m_SkillZoomOutArmLength, DeltaTime, m_SkillZoomOutSpeed);
		return true;
	}

	default:
		return false;
	}
}

void UPlayer_CameraComponent::_updateClearCameraZoom(float DeltaTime)
{
	m_ClearCurrentArmLength = FMath::FInterpTo(
		m_ClearCurrentArmLength,
		m_ClearTargetArmLength,
		DeltaTime,
		m_ClearZoomInterpSpeed
	);

	m_SpringArm->TargetArmLength = m_ClearCurrentArmLength;
}




void UPlayer_CameraComponent::StartSkillCamera()
{
	m_SkillCameraPhase = ESkillCameraPhase::ZoomIn;
}

void UPlayer_CameraComponent::EndSkillCamera()
{
	m_SkillCameraPhase = ESkillCameraPhase::None;

	//通常距離へ
	m_SpringArm->TargetArmLength = PlayerParam.CameraLength;
}

//ロックオン解除
void UPlayer_CameraComponent::CancellationLockOn()
{
}

void UPlayer_CameraComponent::CameraShakEnd()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StopAllCameraShakes(true);
		}
	}

}

void UPlayer_CameraComponent::StartAnimCameraOverride(float TargetLength, float TargetFOV, float Speed)
{
	// 処理を丸投げ
	if (OverrideHandler)
	{
		OverrideHandler->StartOverride(TargetLength, TargetFOV, Speed);
	}
}

void UPlayer_CameraComponent::EndAnimCameraOverride()
{
	// 処理を丸投げ
	if (OverrideHandler)
	{
		OverrideHandler->EndOverride();
	}
}



//----------入力コールバック------------

//カメラ
//ピッチ回転入力
void UPlayer_CameraComponent::Input_CameraRotatePitch(const FInputActionValue& Value)
{
	if (m_IsClearCamera) return;
	if (m_IsEnemyDirectionLooking)return;

	//入力値
	float PitchAxis = Value.Get<float>();
	if (FMath::IsNearlyZero(PitchAxis))return;

	if (m_bInvertCameraY) {
		PitchAxis *= -1.0f;
	}

	//経過時間
	m_LastCamerainputTime = GetWorld()->GetTimeSeconds();

	//現在の回転を取得
	FRotator ControlRotate = m_PlayerController->GetControlRotation();

	ControlRotate.Pitch += PitchAxis * PlayerParam.CameraSensitivity;

	//縦方向の制限
	ControlRotate.Pitch = UKismetMathLibrary::ClampAngle(ControlRotate.Pitch, m_MinPitch, m_MaxPitch);

	//回転を取得
	m_PlayerController->SetControlRotation(ControlRotate);
}



//ヨウ回転入力
void UPlayer_CameraComponent::Input_CameraRotateYaw(const FInputActionValue& Value)
{
	if (m_IsClearCamera) return;
	if (m_IsEnemyDirectionLooking)return;
	//入力値
	float YawAxis = Value.Get<float>();
	if (FMath::IsNearlyZero(YawAxis))return;

	if (m_bInvertCameraX) {
		YawAxis *= -1.0f;
	}

	//経過時間
	m_LastCamerainputTime = GetWorld()->GetTimeSeconds();

	//回転を取得
	m_Player->AddControllerYawInput(YawAxis * PlayerParam.CameraSensitivity);
}




//カメラリセット入力
void UPlayer_CameraComponent::Input_CameraReset(const FInputActionValue& Value)
{
	if (m_IsClearCamera) return;
	if (!m_PlayerController.IsValid())return;

	//CharacterMovement取得
	UCharacterMovementComponent* MoveComponent = m_Player->GetCharacterMovement();
	if (!MoveComponent) return;

	//自動旋回を停止
	bool bPrevOrient = MoveComponent->bOrientRotationToMovement;
	MoveComponent->bOrientRotationToMovement = false;

	//リセット時の位置
	FRotator TargetRotation = m_Player->GetActorRotation();
	TargetRotation.Pitch = -15.f;
	TargetRotation.Roll = 0.f;

	//カメラ方向設定
	m_PlayerController->SetControlRotation(TargetRotation);

	//少し遅らせて自動旋回を戻す
	FTimerHandle TimerHandle;
	m_Player->GetWorldTimerManager().SetTimer(TimerHandle, [MoveComponent, bPrevOrient]()
		{
			MoveComponent->bOrientRotationToMovement = bPrevOrient;
		}, 0.1f, false);
}


//カメラロックオン
void UPlayer_CameraComponent::Input_TargetLockOn(const FInputActionValue& Value)
{
}

//ターゲットを変える
void UPlayer_CameraComponent::Input_TargetChange(const FInputActionValue& Value)
{
}


//ジャス回避成功時敵の方向へカメラを向ける
void UPlayer_CameraComponent::OnJEnemyDirection(const AActor* Target, bool _LowAngle)
{
	if (!Target) return;

	m_EnemyTarget = Target;
	m_IsEnemyDirectionLooking = true;

}

void UPlayer_CameraComponent::ClearCamera()
{
	if (!m_PlayerController.IsValid())return;

	m_IsClearCamera = true;
	m_ClearCameraPhase = EClearCameraPhase::Side;

	//キャラクターの回転を停止
	m_Player->GetCharacterMovement()->bOrientRotationToMovement = false;
	m_SpringArm->bUsePawnControlRotation = true;

	GetWorld()->GetTimerManager().SetTimer(
		m_BackClearTimer,
		this,
		&UPlayer_CameraComponent::SetBackClearCamera,
		1.0f,
		false
	);


	//真横クリアしてから３秒後
	GetWorld()->GetTimerManager().SetTimer(
		m_SideClearTimer,
		this,
		&UPlayer_CameraComponent::SetSideClearCamera,
		3.0f,
		false
	);

	//４秒後
	GetWorld()->GetTimerManager().SetTimer(
		m_FrontClearTimer,
		this,
		&UPlayer_CameraComponent::SetFrontClearCamera,
		4.33f,
		false
	);

	m_ClearCurrentArmLength = m_SpringArm->TargetArmLength;
	m_ClearTargetArmLength = m_ClearCurrentArmLength;
}

void UPlayer_CameraComponent::SetBackClearCamera()
{
	if (!m_SpringArm || !m_PlayerController.IsValid()) return;
	// SpringArm をキャラ横へ
	FRotator BackRot = m_Player->GetActorRotation();
	BackRot.Yaw += BackClearParam.AddYaw;
	BackRot.Pitch = BackClearParam.Pitch;

	m_PlayerController->SetControlRotation(BackRot);

	m_SpringArm->TargetArmLength = BackClearParam.TargetArmLength;

	//カメラが近寄る距離
	m_ClearTargetArmLength = m_ClearZoomLength;
	m_SpringArm->SocketOffset = BackClearParam.SocketOffset;

}

void UPlayer_CameraComponent::SetSideClearCamera()
{
	if (!m_SpringArm || !m_PlayerController.IsValid()) return;
	// SpringArm をキャラ横へ
	FRotator SideRot = m_Player->GetActorRotation();
	SideRot.Yaw += SideClearParam.AddYaw;
	SideRot.Pitch = SideClearParam.Pitch;

	m_PlayerController->SetControlRotation(SideRot);

	//カメラが近寄る距離
	m_ClearTargetArmLength = m_ClearZoomLength;
	m_SpringArm->SocketOffset = SideClearParam.SocketOffset;
}

void UPlayer_CameraComponent::SetFrontClearCamera()
{
	if (!m_SpringArm || !m_PlayerController.IsValid()) return;

	m_ClearCameraPhase = EClearCameraPhase::FrontAngle;

	FRotator FrontRot = m_Player->GetActorRotation();
	FrontRot.Yaw += FrontClearParam.AddYaw;
	FrontRot.Pitch = FrontClearParam.Pitch;

	m_PlayerController->SetControlRotation(FrontRot);

	//カメラが近寄る距離
	m_ClearTargetArmLength = m_ClearZoomLength;
	m_SpringArm->SocketOffset = FrontClearParam.SocketOffset;
}

void UPlayer_CameraComponent::PlayInGameOpening()
{
	if (OpeningHandler)
	{
		m_IsOpeningCamera = true;
		m_Player->DisableControl();
		//0秒待機、 4秒かけて、距離100(胸付近)から、180度旋回して戻る
		OpeningHandler->StartOpening(0.0f, 4.0f, 100.0f, 180.0f);

	}
}

void UPlayer_CameraComponent::SetCameraSideOffset(bool bIsSideView, float OffsetAmount)
{
	if (bIsSideView)
	{
		// Y軸にオフセットを加える（右にずらす）
		// SpringArmの向きに対して右側(Y)に値を設定
		m_TargetSocketOffset = m_DefaultSocketOffset + FVector(0.f, OffsetAmount, 0.f);
	}
	else
	{
		// 元に戻す
		m_TargetSocketOffset = m_DefaultSocketOffset;
	}
}

void UPlayer_CameraComponent::OnPlayerDie()
{
	if (DieHandler)
	{
		DieHandler->StartDieCamera();
	}
}

FVector UPlayer_CameraComponent::SmoothDamp(FVector Current, FVector Target, FVector& Velocity, float SmoothTime, float DeltaTime)
{
	//角振動数,ばねの強さ
	//SmoothDampが小さい→ Omega が大きい → 速く追従
	//SmoothTime が大きい → 重く、遅れる
	const float Omega = 2.f / SmoothTime;
	//１フレームでどれくらい進むか
	const float X = Omega * DeltaTime;
	//値が小さいほど「強く減衰」
	//大きいほど「慣性が残る」
	const float Exp = 1.f / (1.f + X + 0.48f * X * X + 0.235f * X * X * X);

	//目標からのズレ量
	FVector Change = Current - Target;
	FVector Temp = (Velocity + Omega * Change) * DeltaTime;

	Velocity = (Velocity - Omega * Temp) * Exp;
	return Target + (Change + Temp) * Exp;
}

float UPlayer_CameraComponent::GetCameraFollowTime() const
{
	if (m_MovementComponent->GetIsDash())
	{
		return 0.1f;
	}
	if (m_EvasiveComponent->GetIsEvasive())
	{
		return 0.15f;
	}
	if (m_AttackComponent->GetIsAirAttackStart())
	{
		return 0.15f;
	}
	if (m_AttackComponent->GetIsAttack())
	{
		return 0.1f;
	}

	return 0.05f; //通常
}


void UPlayer_CameraComponent::ResetCamera()
{
	// ===== Timer 停止 =====
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(m_SideClearTimer);
		World->GetTimerManager().ClearTimer(m_FrontClearTimer);
		World->GetTimerManager().ClearTimer(m_BackClearTimer);
	}

	// ===== Clear 終了 =====
	m_IsClearCamera = false;
	m_ClearCameraPhase = EClearCameraPhase::None;

	if (DieHandler)
	{
		DieHandler->EndDieCamera();
	}

	// ===== 他カメラ状態を全停止 =====
	m_IsEnemyDirectionLooking = false;
	m_EnemyTarget = nullptr;
	m_SkillCameraPhase = ESkillCameraPhase::None;

	CancellationLockOn();

	// ===== 回転責務を「通常」に強制復帰 =====
	if (m_SpringArm)
	{
		// 通常プレイでは ControllerRotation を使う
		m_SpringArm->bUsePawnControlRotation = true;
		m_SpringArm->TargetArmLength = PlayerParam.CameraLength;
		m_SpringArm->SocketOffset = PlayerParam.CameraSocketOffset;

		m_SpringArm->bInheritPitch = true;
		m_SpringArm->bInheritYaw = true;
		m_SpringArm->bInheritRoll = true;
	}

	// ===== ControlRotation を Actor 基準に再同期 =====
	if (m_PlayerController.IsValid() && m_Player)
	{
		FRotator ControlRot = m_Player->GetActorRotation();
		ControlRot.Pitch = -15.f;   // 通常の初期角
		ControlRot.Roll = 0.f;
		m_PlayerController->SetControlRotation(ControlRot);
	}

	// ===== キャラ回転は移動基準 =====
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
	}

	// ===== FOV =====
	if (m_Camera)
	{
		m_Camera->SetFieldOfView(PlayerParam.DefaultFOV);
	}

	// ===== 内部補間値同期 =====
	m_ClearCurrentArmLength = PlayerParam.CameraLength;
	m_ClearTargetArmLength = PlayerParam.CameraLength;

	CameraShakEnd();

	OnBossBattleEnd();
}


bool UPlayer_CameraComponent::IsLocationInCameraView(const FVector& WorldLocation) const
{
	if (!m_Camera) return false;

	FVector CameraLoc = m_Camera->GetComponentLocation();
	FVector CameraFwd = m_Camera->GetForwardVector();

	FVector ToTarget = WorldLocation - CameraLoc;
	ToTarget.Normalize();

	float Dot = FVector::DotProduct(CameraFwd, ToTarget);
	float HalfFOV = FMath::DegreesToRadians(m_Camera->FieldOfView * 0.5f);
	float Threshold = FMath::Cos(HalfFOV);

	return Dot >= Threshold;
}


void UPlayer_CameraComponent::OnBossBattleStart()
{
	if (BossHandler)
	{
		BossHandler->StartBossCamera();
	}
}

void UPlayer_CameraComponent::OnBossBattleEnd()
{
	if (BossHandler)
	{
		BossHandler->EndBossCamera();
	}
}

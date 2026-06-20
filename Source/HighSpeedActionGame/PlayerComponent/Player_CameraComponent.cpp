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

//===マジックナンバー排除用の定数定義===
namespace CameraConstants
{
	//追従処理の関連定数
	constexpr float DeadZoneRadius = 50.0f; //追従の遊びとなるXY平面の半径
	constexpr float ZFollowThreshold = 30.0f; //Z軸(高さ)の追従を開始する閾値
	constexpr float ManualInputCooldown = 0.1f; //手動カメラ操作後に自動追従を再開するまでの猶予時間

	//許容誤差の関連定数
	constexpr float TargetRotationTolerance = 2.0f; //目標の回転角度に到達したと判定する許容誤差
	constexpr float ZoomArrivalTolerance = 1.0f; //ズーム目標に到達したと判定する距離の許容誤差

	//クリア演出カメラの遷移待機時間
	constexpr float ClearCameraBackDelay = 1.0f; //背後カメラ演出へ移行するまでの時間
	constexpr float ClearCameraSideDelay = 3.0f; //側面カメラ演出へ移行するまでの時間
	constexpr float ClearCameraFrontDelay = 4.33f; //正面カメラ演出へ移行するまでの時間

	//カメラリセット機能の関連定数
	constexpr float ResetPitchAngle = -15.0f; //カメラリセット時に適用されるデフォルトのピッチ角
	constexpr float ResetAutoOrientDelay = 0.1f; //リセット後にキャラクターの自動旋回を復帰させるまでの時間

	//オープニング演出の関連定数
	constexpr float OpeningWaitTime = 0.0f; //オープニング演出の開始待機時間
	constexpr float OpeningDuration = 4.0f; //オープニング演出の完了にかかる時間
	constexpr float OpeningStartDistance = 100.0f; //オープニング演出開始時のカメラ距離(胸元付近)
	constexpr float OpeningRotationAngle = 180.0f; //オープニング演出でカメラが旋回する角度

	//カメラ追従時間(SmoothDampのSmoothTime)の関連定数
	constexpr float FollowTimeDash = 0.1f; //ダッシュ中の追従遅延時間
	constexpr float FollowTimeEvasive = 0.15f; //回避中の追従遅延時間
	constexpr float FollowTimeAirAttack = 0.15f; //空中攻撃中の追従遅延時間
	constexpr float FollowTimeAttack = 0.1f; //地上攻撃中の追従遅延時間
	constexpr float FollowTimeDefault = 0.05f; //通常移動時の追従遅延時間

	//SmoothDamp(バネモデル)の計算用数学定数
	constexpr float SmoothDampCoef1 = 0.48f;
	constexpr float SmoothDampCoef2 = 0.235f;
	constexpr float SmoothDampCoef3 = 2.0f;
}

// Sets default values for this component's properties
UPlayer_CameraComponent::UPlayer_CameraComponent()
{
	//コンポーネントがゲーム開始時に初期化され、毎フレームTick処理が呼ばれるように設定する
	PrimaryComponentTick.bCanEverTick = true;
	//変数の初期化はすべてヘッダー側のデフォルトメンバ初期化子に移動済み
}

// Called when the game starts
void UPlayer_CameraComponent::BeginPlay()
{
	Super::BeginPlay();

	//プレイヤーの設定を管理するサブシステムから感度や反転設定を取得して適用する
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UPlayerSettingsSubsystem* SettingsSubsystem = GameInstance->GetSubsystem<UPlayerSettingsSubsystem>())
		{
			//サブシステムに保存されているスライダーの値をそのまま代入
			m_PlayerParam.CameraSensitivity = SettingsSubsystem->GetCameraSensitivity();

			//リバース(操作反転)設定の適用
			m_bInvertCameraX = SettingsSubsystem->GetInvertCameraX();
			m_bInvertCameraY = SettingsSubsystem->GetInvertCameraY();
		}
	}

	//オーナーであるプレイヤーキャラクターを取得し各種コンポーネントをキャッシュする
	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player) return;

	m_MovementComponent = m_Player->FindComponentByClass<UPlayer_MovementComponent>();
	m_AttackComponent = m_Player->FindComponentByClass<UPlayer_AttackComponent>();
	m_EvasiveComponent = m_Player->FindComponentByClass<UPlayer_EvasiveComponent>();
	m_PlayerController = Cast<APlayerController>(m_Player->GetController());

	//PlayerのRootとしてCapsuleが存在するか検証
	USceneComponent* PlayerRoot = m_Player->GetRootComponent();
	if (!PlayerRoot) return;

	m_SpringArm = m_Player->m_SpringArm;
	m_Camera = m_Player->m_Camera;

	if (m_SpringArm)
	{
		m_SpringArm->TargetArmLength = m_PlayerParam.CameraLength;
		m_DefaultSocketOffset = m_SpringArm->SocketOffset;
		m_TargetSocketOffset = m_DefaultSocketOffset;
	}

	//SmoothDamp処理の初期値として現在のプレイヤー座標を設定する
	m_CameraFocusLocation = m_Player->GetActorLocation();
	m_CameraFocusVelocity = FVector::ZeroVector;

	//演出管理クラス(オーバーライドハンドラ)の生成と初期化
	m_OverrideHandler = NewObject<UCameraOverrideHandler>(this);
	m_OverrideHandler->Initialize(m_SpringArm, m_Camera);

	//オープニングハンドラの初期化
	if (m_OpeningHandler)
	{
		m_OpeningHandler->Initialize(m_SpringArm, m_Camera, m_PlayerController.Get());
	}

	//死亡時カメラハンドラの初期化
	if (m_DieHandler)
	{
		m_DieHandler->Initialize(m_SpringArm, m_PlayerController.Get());
	}

	//ボス戦用カメラハンドラの初期化
	if (m_BossHandler)
	{
		m_BossHandler->Initialize(m_SpringArm, m_Camera);
	}

	m_IsOpeningCamera = false;

	//アクション用カメラハンドラの生成と初期化
	m_ActionHandler = NewObject<UCameraActionHandler>(this);
	if (m_ActionHandler)
	{
		//必要なコンポーネントのポインタをハンドラに渡す
		m_ActionHandler->Initialize(m_Player, m_PlayerController.Get(), m_SpringArm, m_Camera, m_MovementComponent);

		//Blueprint等で設定されたパラメータの値をハンドラへ反映する
		m_ActionHandler->SetupParams(m_PlayerParam.CameraLength, m_PlayerParam.DefaultFOV, m_PlayerParam.DashStartArmLength, m_PlayerParam.DashStartFOV, m_PlayerParam.DashMidArmLength, m_PlayerParam.DashMidFOV);
	}

	//イベントのバインド処理
	if (UWorld* World = GetWorld())
	{
		//イベント管理サブシステムからボス討伐などのイベントを検知して関数を紐付ける
		if (UGameplayAreaEventManager* EventManager = World->GetSubsystem<UGameplayAreaEventManager>())
		{
			//ボス死亡時にカメラ演出を終了させる
			EventManager->OnBossDead.AddDynamic(this, &UPlayer_CameraComponent::OnBossBattleEnd);
		}

		//シーケンス管理サブシステムからボス戦開始などのイベントを検知して関数を紐付ける
		if (USequenceWorldSubsystem* SequenceSub = World->GetSubsystem<USequenceWorldSubsystem>())
		{
			SequenceSub->OnBossSequenceFinished.AddDynamic(this, &UPlayer_CameraComponent::OnBossBattleStart);
		}
	}
}

// Called every frame
void UPlayer_CameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//プレイヤー死亡時は他のカメラ操作をすべて無効化する
	if (m_DieHandler && m_DieHandler->IsActive())
	{
		return;
	}

	//ゲームクリア演出中は専用のズーム処理のみを実行して終了する
	if (m_IsClearCamera)
	{
		_updateClearCameraZoom(DeltaTime);
		return;
	}

	//オープニング演出中の処理
	if (m_IsOpeningCamera)
	{
		if (m_OpeningHandler)
		{
			//演出を毎フレーム更新する
			bool bIsStillRunning = m_OpeningHandler->UpdateOpening(DeltaTime);

			//演出が終了した瞬間の処理
			if (!bIsStillRunning)
			{
				m_IsOpeningCamera = false;

				if (m_Player)
				{
					//プレイヤーの操作入力を解禁する
					m_Player->EnableControl();
				}
			}
		}
		else
		{
			//エラー対策：ハンドラが存在しない場合も進行不能を回避するため操作を解禁する
			m_IsOpeningCamera = false;
			if (m_Player)
			{
				m_Player->EnableControl();
			}
		}
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//敵の方向を向く自動補正処理
	_updateEnemyDirectionCamera(DeltaTime);

	//通常プレイ時はデッドゾーンを用いたカメラ焦点の追従処理を実行する
	if (!m_IsClearCamera)
	{
		_updateCameraFocus(DeltaTime);
		m_SpringArm->SetWorldLocation(m_CameraFocusLocation);
	}

	//ボス戦専用のカメラ挙動が有効な場合の処理
	if (m_BossHandler && m_BossHandler->IsActive())
	{
		m_BossHandler->UpdateBossCamera(DeltaTime);
		return;
	}

	bool bIsSkillActive = (m_SkillCameraPhase != ESkillCameraPhase::None);

	//敵方向補正、クリア演出、スキル演出のいずれにも該当しない場合のみアクションカメラを適用
	if (!m_IsEnemyDirectionLooking && !m_IsClearCamera && !bIsSkillActive)
	{
		if (m_ActionHandler)
		{
			m_ActionHandler->UpdateActionCamera(DeltaTime);
		}
	}

	//強攻撃チャージ時のカメラ制御が介入した場合は、他の距離制御処理をスキップする
	if (_updateHeavyChargeCamera(DeltaTime))
	{
		return;
	}

	//スキル使用時のカメラ制御が介入した場合はここで処理を終了する
	if (_updateSkillCamera(DeltaTime))
	{
		return;
	}

	//アニメーション連動などのカメラオーバーライドが有効な場合の処理
	if (m_OverrideHandler && m_OverrideHandler->UpdateOverride(DeltaTime))
	{
		return;
	}

	//カメラのソケットオフセット(立ち位置のズレなど)を毎フレーム滑らかに補間して適用する
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

	//プレイヤーがデッドゾーン内に収まっている場合はカメラを動かさない(画面酔い防止)
	if (ToPlayer.Size2D() < CameraConstants::DeadZoneRadius && FMath::Abs(ToPlayer.Z) < CameraConstants::ZFollowThreshold)
	{
		return;
	}

	//デッドゾーンからのはみ出し分だけ目標位置をずらす
	FVector Target = PlayerLocation - ToPlayer.GetSafeNormal2D() * CameraConstants::DeadZoneRadius;

	//バネモデルに基づく滑らかな曲線補間処理(SmoothDamp)を実行してカメラの焦点座標を更新する
	m_CameraFocusLocation = SmoothDamp(
		m_CameraFocusLocation,
		Target,
		m_CameraFocusVelocity,
		GetCameraFollowTime(),
		DeltaTime
	);
}

//ロックオン対象の敵の方向へカメラを自動で向ける処理
void UPlayer_CameraComponent::_updateEnemyDirectionCamera(float DeltaTime)
{
	if (!m_IsEnemyDirectionLooking || !m_EnemyTarget.IsValid()) return;

	//プレイヤーによる手動カメラ入力が直近であった場合は自動回転処理を中断して操作を優先する
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - m_LastCamerainputTime < CameraConstants::ManualInputCooldown)
	{
		m_IsEnemyDirectionLooking = false;
		m_EnemyTarget = nullptr;
		return;
	}

	FVector CamLocation = m_Camera->GetComponentLocation();
	FVector TargetLocation = m_EnemyTarget->GetActorLocation();
	//ピッチ(上下)が過剰に振れないようにZ座標はプレイヤーの高さに合わせる
	TargetLocation.Z = m_Player->GetActorLocation().Z;

	FRotator TargetRotation = (TargetLocation - CamLocation).Rotation();
	FRotator NewRotation = FMath::RInterpTo(
		m_PlayerController->GetControlRotation(),
		TargetRotation,
		DeltaTime,
		m_EnemyDirectionInterpSpeed
	);

	m_PlayerController->SetControlRotation(NewRotation);

	//目標の回転角度とほぼ一致した段階で自動旋回を終了する
	if (NewRotation.Equals(TargetRotation, CameraConstants::TargetRotationTolerance))
	{
		m_IsEnemyDirectionLooking = false;
		m_EnemyTarget = nullptr;
	}
}

bool UPlayer_CameraComponent::_updateHeavyChargeCamera(float DeltaTime)
{
	if (!m_AttackComponent) return false;

	//強攻撃チャージ中でない場合は何もせずfalseを返す
	if (!m_AttackComponent->GetIsHeavyCharging())
	{
		return false;
	}

	//チャージ中は徐々にカメラをキャラクターに寄せる
	m_SpringArm->TargetArmLength = FMath::FInterpTo(m_SpringArm->TargetArmLength, m_HeavyChargeArmLength, DeltaTime, m_HeavyChargeInterpSpeed);
	return true;
}

bool UPlayer_CameraComponent::_updateSkillCamera(float DeltaTime)
{
	switch (m_SkillCameraPhase)
	{
	case ESkillCameraPhase::ZoomIn:
	{
		//スキル発動直後は勢いよくズームインする
		m_SpringArm->TargetArmLength = FMath::FInterpTo(m_SpringArm->TargetArmLength, m_SkillZoomInArmLength, DeltaTime, m_SkillZoomInSpeed);

		//目標距離まで寄り切ったらズームアウトのフェーズへ移行する
		if (FMath::IsNearlyEqual(m_SpringArm->TargetArmLength, m_SkillZoomInArmLength, CameraConstants::ZoomArrivalTolerance))
		{
			m_SkillCameraPhase = ESkillCameraPhase::ZoomOut;
		}
		return true;
	}

	case ESkillCameraPhase::ZoomOut:
	{
		//ズームイン後は少しゆっくりとカメラを引く演出を行う
		m_SpringArm->TargetArmLength = FMath::FInterpTo(m_SpringArm->TargetArmLength, m_SkillZoomOutArmLength, DeltaTime, m_SkillZoomOutSpeed);
		return true;
	}

	default:
		return false;
	}
}

void UPlayer_CameraComponent::_updateClearCameraZoom(float DeltaTime)
{
	//クリア演出中の滑らかなズームイン処理
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
	//スキル演出のズームインフェーズを開始する
	m_SkillCameraPhase = ESkillCameraPhase::ZoomIn;
}

void UPlayer_CameraComponent::EndSkillCamera()
{
	//スキル演出を終了し、カメラの目標距離を通常時に戻す
	m_SkillCameraPhase = ESkillCameraPhase::None;
	m_SpringArm->TargetArmLength = m_PlayerParam.CameraLength;
}

void UPlayer_CameraComponent::CancellationLockOn()
{
	//必要に応じてロックオン解除の具体的な処理を実装
}

void UPlayer_CameraComponent::CameraShakEnd()
{
	//現在再生されているすべてのカメラシェイク(振動演出)を即座に停止する
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
	//処理をオーバーライド管理ハンドラへ委譲する
	if (m_OverrideHandler)
	{
		m_OverrideHandler->StartOverride(TargetLength, TargetFOV, Speed);
	}
}

void UPlayer_CameraComponent::EndAnimCameraOverride()
{
	//処理をオーバーライド管理ハンドラへ委譲する
	if (m_OverrideHandler)
	{
		m_OverrideHandler->EndOverride();
	}
}

//----------入力コールバック------------

//カメラのピッチ(上下)回転入力処理
void UPlayer_CameraComponent::Input_CameraRotatePitch(const FInputActionValue& Value)
{
	if (m_IsClearCamera) return;
	if (m_IsEnemyDirectionLooking) return;

	//コントローラーやマウスからの入力値を取得
	float PitchAxis = Value.Get<float>();
	if (FMath::IsNearlyZero(PitchAxis)) return;

	//リバース設定が有効なら入力を反転
	if (m_bInvertCameraY) {
		PitchAxis *= -1.0f;
	}

	//手動入力があった時間を記録(自動旋回を防ぐため)
	m_LastCamerainputTime = GetWorld()->GetTimeSeconds();

	//現在の回転を取得して入力値と感度を乗算し加算する
	FRotator ControlRotate = m_PlayerController->GetControlRotation();
	ControlRotate.Pitch += PitchAxis * m_PlayerParam.CameraSensitivity;

	//カメラが縦方向に裏返らないよう可動域に制限をかける
	ControlRotate.Pitch = UKismetMathLibrary::ClampAngle(ControlRotate.Pitch, m_MinPitch, m_MaxPitch);

	//計算結果をコントローラーに適用する
	m_PlayerController->SetControlRotation(ControlRotate);
}

//カメラのヨー(左右)回転入力処理
void UPlayer_CameraComponent::Input_CameraRotateYaw(const FInputActionValue& Value)
{
	if (m_IsClearCamera) return;
	if (m_IsEnemyDirectionLooking) return;

	//コントローラーやマウスからの入力値を取得
	float YawAxis = Value.Get<float>();
	if (FMath::IsNearlyZero(YawAxis)) return;

	//リバース設定が有効なら入力を反転
	if (m_bInvertCameraX) {
		YawAxis *= -1.0f;
	}

	//手動入力があった時間を記録
	m_LastCamerainputTime = GetWorld()->GetTimeSeconds();

	//プレイヤー側に回転入力を流し込む
	m_Player->AddControllerYawInput(YawAxis * m_PlayerParam.CameraSensitivity);
}

//カメラ位置のリセット入力処理(キャラクターの背後にカメラを回す)
void UPlayer_CameraComponent::Input_CameraReset(const FInputActionValue& Value)
{
	if (m_IsClearCamera) return;
	if (!m_PlayerController.IsValid()) return;

	//キャラクタームーブメントコンポーネントを取得
	UCharacterMovementComponent* MoveComponent = m_Player->GetCharacterMovement();
	if (!MoveComponent) return;

	//カメラリセット時にキャラクターが勝手に振り向かないよう自動旋回を一時停止する
	bool bPrevOrient = MoveComponent->bOrientRotationToMovement;
	MoveComponent->bOrientRotationToMovement = false;

	//キャラクターの現在の向きを基準にリセット時の回転角を計算する
	FRotator TargetRotation = m_Player->GetActorRotation();
	TargetRotation.Pitch = CameraConstants::ResetPitchAngle;
	TargetRotation.Roll = 0.f;

	//計算した方向へカメラを強制設定する
	m_PlayerController->SetControlRotation(TargetRotation);

	//わずかに遅延させてからキャラクターの自動旋回設定を元に戻す
	FTimerHandle TimerHandle;
	m_Player->GetWorldTimerManager().SetTimer(TimerHandle, [MoveComponent, bPrevOrient]()
		{
			MoveComponent->bOrientRotationToMovement = bPrevOrient;
		}, CameraConstants::ResetAutoOrientDelay, false);
}

//ジャスト回避成功時などに特定の対象へカメラを向けるようセットする
void UPlayer_CameraComponent::OnJEnemyDirection(const AActor* Target, bool _LowAngle)
{
	if (!Target) return;

	m_EnemyTarget = Target;
	m_IsEnemyDirectionLooking = true;
}

void UPlayer_CameraComponent::ClearCamera()
{
	if (!m_PlayerController.IsValid()) return;

	m_IsClearCamera = true;
	m_ClearCameraPhase = EClearCameraPhase::Side;

	//キャラクターの自動回転を停止し、スプリングアームがコントローラーの回転に依存するように変更する
	m_Player->GetCharacterMovement()->bOrientRotationToMovement = false;
	m_SpringArm->bUsePawnControlRotation = true;

	//背面からのクリア演出カメラへ移行するタイマーをセット
	GetWorld()->GetTimerManager().SetTimer(
		m_BackClearTimer,
		this,
		&UPlayer_CameraComponent::SetBackClearCamera,
		CameraConstants::ClearCameraBackDelay,
		false
	);

	//真横からのクリア演出カメラへ移行するタイマーをセット
	GetWorld()->GetTimerManager().SetTimer(
		m_SideClearTimer,
		this,
		&UPlayer_CameraComponent::SetSideClearCamera,
		CameraConstants::ClearCameraSideDelay,
		false
	);

	//斜め前方からのクリア演出カメラへ移行するタイマーをセット
	GetWorld()->GetTimerManager().SetTimer(
		m_FrontClearTimer,
		this,
		&UPlayer_CameraComponent::SetFrontClearCamera,
		CameraConstants::ClearCameraFrontDelay,
		false
	);

	m_ClearCurrentArmLength = m_SpringArm->TargetArmLength;
	m_ClearTargetArmLength = m_ClearCurrentArmLength;
}

void UPlayer_CameraComponent::SetBackClearCamera()
{
	if (!m_SpringArm || !m_PlayerController.IsValid()) return;

	//キャラクターの背面基準で回転パラメータを加算し適用する
	FRotator BackRot = m_Player->GetActorRotation();
	BackRot.Yaw += m_BackClearParam.AddYaw;
	BackRot.Pitch = m_BackClearParam.Pitch;

	m_PlayerController->SetControlRotation(BackRot);

	m_SpringArm->TargetArmLength = m_BackClearParam.TargetArmLength;

	//カメラの近寄る目標距離とソケットオフセットをパラメータから反映する
	m_ClearTargetArmLength = m_ClearZoomLength;
	m_SpringArm->SocketOffset = m_BackClearParam.SocketOffset;
}

void UPlayer_CameraComponent::SetSideClearCamera()
{
	if (!m_SpringArm || !m_PlayerController.IsValid()) return;

	//キャラクターの側面基準で回転パラメータを加算し適用する
	FRotator SideRot = m_Player->GetActorRotation();
	SideRot.Yaw += m_SideClearParam.AddYaw;
	SideRot.Pitch = m_SideClearParam.Pitch;

	m_PlayerController->SetControlRotation(SideRot);

	//カメラの近寄る目標距離とソケットオフセットをパラメータから反映する
	m_ClearTargetArmLength = m_ClearZoomLength;
	m_SpringArm->SocketOffset = m_SideClearParam.SocketOffset;
}

void UPlayer_CameraComponent::SetFrontClearCamera()
{
	if (!m_SpringArm || !m_PlayerController.IsValid()) return;

	m_ClearCameraPhase = EClearCameraPhase::FrontAngle;

	//キャラクターの正面基準で回転パラメータを加算し適用する
	FRotator FrontRot = m_Player->GetActorRotation();
	FrontRot.Yaw += m_FrontClearParam.AddYaw;
	FrontRot.Pitch = m_FrontClearParam.Pitch;

	m_PlayerController->SetControlRotation(FrontRot);

	//カメラの近寄る目標距離とソケットオフセットをパラメータから反映する
	m_ClearTargetArmLength = m_ClearZoomLength;
	m_SpringArm->SocketOffset = m_FrontClearParam.SocketOffset;
}

void UPlayer_CameraComponent::PlayInGameOpening()
{
	if (m_OpeningHandler)
	{
		m_IsOpeningCamera = true;

		//演出中の操作を防ぐ
		m_Player->DisableControl();

		//オープニング演出のパラメータを渡して開始する
		m_OpeningHandler->StartOpening(
			CameraConstants::OpeningWaitTime,
			CameraConstants::OpeningDuration,
			CameraConstants::OpeningStartDistance,
			CameraConstants::OpeningRotationAngle
		);
	}
}

void UPlayer_CameraComponent::SetCameraSideOffset(bool bIsSideView, float OffsetAmount)
{
	if (bIsSideView)
	{
		//スプリングアームの向きに対して右側(Y軸)にオフセットを加える
		m_TargetSocketOffset = m_DefaultSocketOffset + FVector(0.f, OffsetAmount, 0.f);
	}
	else
	{
		//オフセットを元の状態に戻す
		m_TargetSocketOffset = m_DefaultSocketOffset;
	}
}

void UPlayer_CameraComponent::OnPlayerDie()
{
	if (m_DieHandler)
	{
		m_DieHandler->StartDieCamera();
	}
}

FVector UPlayer_CameraComponent::SmoothDamp(FVector Current, FVector Target, FVector& Velocity, float SmoothTime, float DeltaTime)
{
	//角振動数(ばねの強さ)を算出する(SmoothTimeが小さいほどOmegaが大きくなり、速く追従する)
	const float Omega = CameraConstants::SmoothDampCoef3 / SmoothTime;

	//1フレームで進む割合を算出する
	const float X = Omega * DeltaTime;

	//値が小さいほど強く減衰し、大きいほど慣性が残るように指数成分の近似を算出する
	const float Exp = 1.f / (1.f + X + CameraConstants::SmoothDampCoef1 * X * X + CameraConstants::SmoothDampCoef2 * X * X * X);

	//目標からのズレ量を基に速度と座標の補間結果を計算して返す
	FVector Change = Current - Target;
	FVector Temp = (Velocity + Omega * Change) * DeltaTime;

	Velocity = (Velocity - Omega * Temp) * Exp;
	return Target + (Change + Temp) * Exp;
}

float UPlayer_CameraComponent::GetCameraFollowTime() const
{
	//プレイヤーのアクション状態に応じて適した追従時間を返す
	if (m_MovementComponent->GetIsDash())
	{
		return CameraConstants::FollowTimeDash;
	}
	if (m_EvasiveComponent->GetIsEvasive())
	{
		return CameraConstants::FollowTimeEvasive;
	}
	if (m_AttackComponent->GetIsAirAttackStart())
	{
		return CameraConstants::FollowTimeAirAttack;
	}
	if (m_AttackComponent->GetIsAttack())
	{
		return CameraConstants::FollowTimeAttack;
	}

	return CameraConstants::FollowTimeDefault; //通常移動時
}

void UPlayer_CameraComponent::ResetCamera()
{
	//===タイマーの停止===
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(m_SideClearTimer);
		World->GetTimerManager().ClearTimer(m_FrontClearTimer);
		World->GetTimerManager().ClearTimer(m_BackClearTimer);
	}

	//===クリア演出の終了===
	m_IsClearCamera = false;
	m_ClearCameraPhase = EClearCameraPhase::None;

	if (m_DieHandler)
	{
		m_DieHandler->EndDieCamera();
	}

	//===その他のカメラ状態をすべて停止===
	m_IsEnemyDirectionLooking = false;
	m_EnemyTarget = nullptr;
	m_SkillCameraPhase = ESkillCameraPhase::None;

	CancellationLockOn();

	//===回転責務を通常プレイの状態へ強制復帰===
	if (m_SpringArm)
	{
		//通常プレイではコントローラーの回転制御(ControllerRotation)を使用する
		m_SpringArm->bUsePawnControlRotation = true;
		m_SpringArm->TargetArmLength = m_PlayerParam.CameraLength;
		m_SpringArm->SocketOffset = m_PlayerParam.CameraSocketOffset;

		m_SpringArm->bInheritPitch = true;
		m_SpringArm->bInheritYaw = true;
		m_SpringArm->bInheritRoll = true;
	}

	//===ControlRotationをキャラクター基準に再同期===
	if (m_PlayerController.IsValid() && m_Player)
	{
		FRotator ControlRot = m_Player->GetActorRotation();
		ControlRot.Pitch = CameraConstants::ResetPitchAngle; //通常の初期角度を適用
		ControlRot.Roll = 0.f;
		m_PlayerController->SetControlRotation(ControlRot);
	}

	//===キャラクターの回転設定を移動基準に戻す===
	if (UCharacterMovementComponent* MoveComp = m_Player->GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
	}

	//===FOV(視野角)の復帰===
	if (m_Camera)
	{
		m_Camera->SetFieldOfView(m_PlayerParam.DefaultFOV);
	}

	//===内部補間値の同期===
	m_ClearCurrentArmLength = m_PlayerParam.CameraLength;
	m_ClearTargetArmLength = m_PlayerParam.CameraLength;

	CameraShakEnd();
	OnBossBattleEnd();
}

bool UPlayer_CameraComponent::IsLocationInCameraView(const FVector& WorldLocation) const
{
	if (!m_Camera) return false;

	FVector CameraLoc = m_Camera->GetComponentLocation();
	FVector CameraFwd = m_Camera->GetForwardVector();

	//カメラから対象への方向ベクトルを正規化して算出
	FVector ToTarget = WorldLocation - CameraLoc;
	ToTarget.Normalize();

	//カメラ正面と対象方向の内積(Dot)を計算する
	float Dot = FVector::DotProduct(CameraFwd, ToTarget);

	//視野角(FOV)の半分をラジアンに変換して閾値を設定し、視界内か判定する
	float HalfFOV = FMath::DegreesToRadians(m_Camera->FieldOfView * 0.5f);
	float Threshold = FMath::Cos(HalfFOV);

	return Dot >= Threshold;
}

void UPlayer_CameraComponent::OnBossBattleStart()
{
	if (m_BossHandler)
	{
		m_BossHandler->StartBossCamera();
	}
}

void UPlayer_CameraComponent::OnBossBattleEnd()
{
	if (m_BossHandler)
	{
		m_BossHandler->EndBossCamera();
	}
}
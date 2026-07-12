#include "CameraRigComponent.h"
#include "Camera/CameraLayer.h"
#include "Camera/CameraLayer_Action.h"
#include "Camera/CameraLayer_BossBattle.h"
#include "Camera/CameraLayer_ClearCamera.h"
#include "Camera/CameraLayer_Lead.h"
#include "Camera/CameraLayer_LookDown.h"
#include "Camera/CameraLayer_SoftClamp.h"
#include "Camera/CameraLayer_OpeningCamera.h"
#include "Camera/CameraLayer_DieCamera.h"
#include "../DataAssets/Player/PlayerCameraParameter.h"
#include "../Event/GameplayAreaEventManager/GameplayAreaEventManager.h"
#include "../SequenceWorldSubsystem/SequenceWorldSubsystem.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	//手動カメラ操作中とみなす時間
	constexpr float CameraInputActiveTime = 0.2f;
}

//初期化
UCameraRigComponent::UCameraRigComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

//開始時
void UCameraRigComponent::BeginPlay()
{
	Super::BeginPlay();

	//カメラ制御に必要なComponent参照を取得
	CacheComponents();

	if (!m_SpringArmRef || !m_CameraRef) return;
	if (!m_CameraParam) return;

	//基本設定、レイヤー、イベントを初期化
	InitializeSpringArm();
	ApplyPlayerCameraLimit();
	BuildCameraPipeline();
	BindWorldEvents();

	m_bIsFirstTick = true;
}

//参照を取得
void UCameraRigComponent::CacheComponents()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	m_SpringArmRef = OwnerActor->FindComponentByClass<USpringArmComponent>();
	m_CameraRef = OwnerActor->FindComponentByClass<UCameraComponent>();
}

//SpringArmを初期化
void UCameraRigComponent::InitializeSpringArm()
{
	if (!m_SpringArmRef) return;
	if (!m_CameraParam) return;

	//リセット時に戻すSpringArmの初期Transformを保存
	if (!m_bHasDefaultSpringArmTransform)
	{
		m_DefaultSpringArmRelativeLocation = m_SpringArmRef->GetRelativeLocation();
		m_DefaultSpringArmRelativeRotation = m_SpringArmRef->GetRelativeRotation();
		m_bHasDefaultSpringArmTransform = true;
	}

	//PlayerControllerの回転に追従する基本設定を適用
	m_SpringArmRef->bUsePawnControlRotation = true;
	m_SpringArmRef->bInheritPitch = true;
	m_SpringArmRef->bInheritYaw = true;
	m_SpringArmRef->bInheritRoll = false;

	//通常カメラの追従設定を適用
	m_SpringArmRef->bEnableCameraLag = true;
	m_SpringArmRef->bEnableCameraRotationLag = false;

	m_SpringArmRef->TargetArmLength = m_CameraParam->BaseArmLength;
	m_SpringArmRef->SocketOffset = m_CameraParam->BaseSocketOffset;
	m_SpringArmRef->TargetOffset = m_CameraParam->BaseTargetOffset;
	m_SpringArmRef->CameraLagSpeed = m_CameraParam->BaseLagSpeed;
	m_SpringArmRef->CameraRotationLagSpeed = m_CameraParam->BaseRotationLagSpeed;
}

//PlayerControllerのカメラ制限を設定
void UCameraRigComponent::ApplyPlayerCameraLimit()
{
	if (!m_CameraParam) return;

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	APlayerController* PC = OwnerActor->GetInstigatorController<APlayerController>();
	if (!PC) return;

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
	if (!CameraManager) return;

	CameraManager->ViewPitchMin = m_CameraParam->PitchMin;
	CameraManager->ViewPitchMax = m_CameraParam->PitchMax;
}

//カメラパイプラインを構築
void UCameraRigComponent::BuildCameraPipeline()
{
	m_CameraPipeline.Reset();

	//Boss戦専用カメラ
	m_CameraPipeline.Add(NewObject<UCameraLayer_BossBattle>(this));

	//移動と回転の先読み
	m_CameraPipeline.Add(NewObject<UCameraLayer_Lead>(this));

	//落下中の見下ろし補正
	m_CameraPipeline.Add(NewObject<UCameraLayer_LookDown>(this));

	//プレイヤー行動に反応するカメラ
	m_CameraPipeline.Add(NewObject<UCameraLayer_Action>(this));

	//画面外へ出すぎないように補正
	m_CameraPipeline.Add(NewObject<UCameraLayer_SoftClamp>(this));

	//オープニング演出カメラ
	m_CameraPipeline.Add(NewObject<UCameraLayer_OpeningCamera>(this));

	//死亡演出カメラ
	m_CameraPipeline.Add(NewObject<UCameraLayer_DieCamera>(this));

	//クリア演出カメラ
	m_CameraPipeline.Add(NewObject<UCameraLayer_ClearCamera>(this));
}

//ワールドイベントを登録
void UCameraRigComponent::BindWorldEvents()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameplayAreaEventManager* EventManager = World->GetSubsystem<UGameplayAreaEventManager>())
	{
		//Boss死亡時にBoss戦カメラを終了
		EventManager->OnBossDead.AddDynamic(this, &UCameraRigComponent::OnBossBattleEnded);
	}

	if (USequenceWorldSubsystem* SequenceSubsystem = World->GetSubsystem<USequenceWorldSubsystem>())
	{
		//Boss開始シーケンス終了後にBoss戦カメラを開始
		SequenceSubsystem->OnBossSequenceFinished.AddDynamic(this, &UCameraRigComponent::OnBossBattleStarted);
	}
}

//Bossターゲットを自動取得
void UCameraRigComponent::TryCacheBossTarget()
{
	if (!m_bAutoFindBossTarget) return;
	if (m_BossTarget.IsValid()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> FoundActors;

	//指定タグを持つActorの先頭をBossターゲットとして使用
	UGameplayStatics::GetAllActorsWithTag(World, m_BossActorTag, FoundActors);

	if (FoundActors.Num() <= 0) return;

	m_BossTarget = FoundActors[0];
}

//毎フレーム更新
void UCameraRigComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_SpringArmRef || !m_CameraRef) return;
	if (!m_CameraParam) return;

	//タイムスローとフレーム時間急増を補正したDeltaTimeを取得
	const float SafeDeltaTime = GetSafeDeltaTime(DeltaTime);

	m_TimeSinceLastCameraInput += SafeDeltaTime;

	const bool bJustEvasiveLookRequest = m_CameraRequest.bIsActive && m_CameraRequest.RequestType == ECameraRequestType::JustEvasiveLook;

	//Just回避カメラだけはスローの実時間で管理する
	const float RequestDeltaTime = bJustEvasiveLookRequest ? DeltaTime : SafeDeltaTime;

	UpdateCameraRequest(RequestDeltaTime);

	FCameraContext Context = BuildCameraContext(SafeDeltaTime);

	//各カメラレイヤーを順番に適用して最終値を反映
	ExecuteCameraPipeline(Context);
	ApplyCameraContext(Context);
	RecoverCameraRelativeLocation(SafeDeltaTime);

	if (!Context.HardClampOffset.IsNearlyZero())
	{
		m_CameraRef->AddWorldOffset(Context.HardClampOffset);
	}

	ClearFinishedCameraRequest();

	m_bIsFirstTick = false;
}

//安全なDeltaTimeを取得
float UCameraRigComponent::GetSafeDeltaTime(float DeltaTime) const
{
	if (!m_CameraParam) return DeltaTime;

	float UnscaledDeltaTime = DeltaTime;

	if (const UWorld* World = GetWorld())
	{
		//タイムスロー中もカメラ補間速度を維持するため実時間へ戻す
		const float TimeDilation = FMath::Max(m_CameraParam->MinTimeDilation, UGameplayStatics::GetGlobalTimeDilation(World));

		UnscaledDeltaTime = DeltaTime / TimeDilation;
	}

	//大きすぎるDeltaTimeはカメラの瞬間移動を防ぐため標準値へ置換
	if (UnscaledDeltaTime > m_CameraParam->MaxDeltaTimeThreshold)
	{
		return m_CameraParam->StandardFrameTimeDelta;
	}

	return UnscaledDeltaTime;
}

//カメラContextを作成
FCameraContext UCameraRigComponent::BuildCameraContext(float DeltaTime) const
{
	FCameraContext Context;

	//全レイヤーで共通使用する参照と状態を設定
	Context.World = GetWorld();
	Context.OwnerActor = GetOwner();
	Context.DeltaTime = DeltaTime;
	Context.CameraParam = m_CameraParam;
	Context.CurrentMode = m_CurrentMode;

	Context.CameraRotation = m_SpringArmRef ? m_SpringArmRef->GetComponentRotation() : FRotator::ZeroRotator;

	Context.CameraLocation = m_CameraRef ? m_CameraRef->GetComponentLocation() : FVector::ZeroVector;

	Context.SpringArm = m_SpringArmRef;
	Context.Camera = m_CameraRef;

	Context.bIsFirstTick = m_bIsFirstTick;
	Context.bIsPlayerMovingCamera = m_TimeSinceLastCameraInput < CameraInputActiveTime;

	Context.PlayerVelocity = GetOwner() ? GetOwner()->GetVelocity() : FVector::ZeroVector;

	Context.bIsBossBattle = m_bIsBossBattle;
	Context.bHasBossTarget = m_BossTarget.IsValid();

	Context.BossLocation = m_BossTarget.IsValid() ? m_BossTarget->GetActorLocation() : FVector::ZeroVector;

	Context.CameraRequest = m_CameraRequest;

	const bool bDirectionLookRequest = Context.CameraRequest.bIsActive && Context.CameraRequest.RequestType == ECameraRequestType::JustEvasiveLook;

	if (
		bDirectionLookRequest &&
		m_SpringArmRef &&
		m_CameraRef
		)
	{
		//敵注視中は現在のカメラ位置と設定をそのまま維持する
		Context.TargetArmLength = m_SpringArmRef->TargetArmLength;

		Context.CumulativeSocketOffset = m_SpringArmRef->SocketOffset;

		Context.CumulativeTargetOffset = m_SpringArmRef->TargetOffset;

		Context.FinalFOV = m_CameraRef->FieldOfView;

		Context.CameraLagSpeed = m_SpringArmRef->CameraLagSpeed;
	}
	else
	{
		//通常時はBase値から各Layerを積み重ねる
		Context.TargetArmLength = m_CameraParam->BaseArmLength;

		Context.CumulativeSocketOffset = m_CameraParam->BaseSocketOffset;

		Context.CumulativeTargetOffset = m_CameraParam->BaseTargetOffset;

		Context.FinalFOV = m_CameraParam->BaseFOV;

		Context.CameraLagSpeed = m_CameraParam->BaseLagSpeed;
	}

	Context.CumulativeRotationOffset = FRotator::ZeroRotator;

	Context.HardClampOffset = FVector::ZeroVector;

	return Context;
}

//カメラパイプラインを実行
void UCameraRigComponent::ExecuteCameraPipeline(FCameraContext& Context)
{
	const bool bClearRequest =
		Context.CameraRequest.bIsActive &&
		Context.CameraRequest.RequestType == ECameraRequestType::Clear;

	const bool bJustLook =
		Context.CameraRequest.bIsActive &&
		Context.CameraRequest.RequestType == ECameraRequestType::JustEvasiveLook;

	//クリア中はクリアカメラだけを実行
	if (bClearRequest)
	{
		for (UCameraLayer* Layer : m_CameraPipeline)
		{
			if (!Layer) continue;
			if (!Layer->IsA(UCameraLayer_ClearCamera::StaticClass())) continue;

			Layer->ApplyLayer(Context);
			return;
		}

		return;
	}

	//登録順に各レイヤーの補正を積み重ねる
	for (UCameraLayer* Layer : m_CameraPipeline)
	{
		if (!Layer) continue;

		if (bJustLook && !Layer->IsA(UCameraLayer_Action::StaticClass()))
		{
			continue;
		}

		Layer->ApplyLayer(Context);
	}
}


//最終カメラ値を適用
void UCameraRigComponent::ApplyCameraContext(const FCameraContext& Context)
{
	if (!m_SpringArmRef || !m_CameraRef) return;
	if (!m_CameraParam) return;

	m_SpringArmRef->TargetArmLength = Context.TargetArmLength;
	m_SpringArmRef->SocketOffset = Context.CumulativeSocketOffset;
	m_SpringArmRef->TargetOffset = Context.CumulativeTargetOffset;
	m_SpringArmRef->CameraLagSpeed = Context.CameraLagSpeed;

	const float BasePitch = m_SpringArmRef->GetComponentRotation().Pitch;
	const float MaxLocalPitch = m_CameraParam->SafePitchMax - BasePitch;
	const float MinLocalPitch = m_CameraParam->SafePitchMin - BasePitch;

	//レイヤー加算後も安全Pitch範囲を超えないよう制限
	FRotator SafeRotation = Context.CumulativeRotationOffset;
	SafeRotation.Pitch = FMath::Clamp(SafeRotation.Pitch, MinLocalPitch, MaxLocalPitch);
	SafeRotation.Roll = 0.f;

	m_CameraRef->SetRelativeRotation(SafeRotation);
	m_CameraRef->SetFieldOfView(Context.FinalFOV);
}

//カメラ相対位置を復帰
void UCameraRigComponent::RecoverCameraRelativeLocation(float DeltaTime)
{
	if (!m_CameraRef || !m_CameraParam) return;

	const FVector CurrentRelativeLocation = m_CameraRef->GetRelativeLocation();

	if (CurrentRelativeLocation.IsNearlyZero(m_CameraParam->RelativeLocationZeroTolerance))
	{
		return;
	}

	//HardClampなどで移動したCameraComponentを原点へ戻す
	const FVector RecoveryLocation = FMath::VInterpTo(CurrentRelativeLocation, FVector::ZeroVector, DeltaTime, m_CameraParam->CameraRecoveryInterpSpeed);

	m_CameraRef->SetRelativeLocation(RecoveryLocation);
}

//カメラモードを設定
void UCameraRigComponent::SetCameraMode(ECameraMode NewMode)
{
	if (m_CurrentMode == NewMode) return;

	m_CurrentMode = NewMode;
}

//Boss戦状態を設定
void UCameraRigComponent::SetBossBattleMode(bool bInBossBattle)
{
	m_bIsBossBattle = bInBossBattle;
	m_CurrentMode = bInBossBattle ? ECameraMode::BossBattle : ECameraMode::Normal;
}

//Bossターゲットを設定
void UCameraRigComponent::SetBossTarget(AActor* BossActor)
{
	m_BossTarget = BossActor;
}

//Bossターゲットを解除
void UCameraRigComponent::ClearBossTarget()
{
	m_BossTarget = nullptr;
}

//オープニングカメラを要求
void UCameraRigComponent::RequestOpeningCamera()
{
	if (!StartCameraRequest(ECameraRequestType::Opening)) return;

	if (!m_CameraParam) return;

	m_CameraRequest.Duration = m_CameraParam->OpeningWaitTime + m_CameraParam->OpeningMoveDuration;
}

//クリアカメラを要求
void UCameraRigComponent::RequestClearCamera()
{
	if (!StartCameraRequest(ECameraRequestType::Clear)) return;

	//クリアカメラは自動終了しない
	m_CameraRequest.Duration = 0.f;
}

//死亡カメラを要求
void UCameraRigComponent::RequestDieCamera()
{
	if (!StartCameraRequest(ECameraRequestType::Die)) return;


	//死亡カメラはリスポーンまで継続
	m_CameraRequest.Duration = 0.f;
}

//スキルカメラを要求
void UCameraRigComponent::RequestSkillCamera()
{
	if (!StartCameraRequest(ECameraRequestType::Skill)) return;


	//スキルカメラはSkill終了通知で解除する
	m_CameraRequest.Duration = 0.f;
}

//ジャスト回避後の敵注視カメラを要求
void UCameraRigComponent::RequestJustEvasiveLookCamera(AActor* TargetActor, bool bLowAngle)
{
	if (!TargetActor) return;

	if (!StartCameraRequest(ECameraRequestType::JustEvasiveLook)) return;


	m_CameraRequest.Duration = m_CameraParam ? m_CameraParam->JustEvasiveLookDuration : 0.25f;

	m_CameraRequest.TargetInfo.SetTarget(TargetActor, bLowAngle);
}

//カメラ要求を終了
void UCameraRigComponent::ClearCameraRequest()
{
	m_CameraRequest.Reset();

	if (m_CurrentMode == ECameraMode::Action)
	{
		m_CurrentMode = m_bIsBossBattle ? ECameraMode::BossBattle : ECameraMode::Normal;
	}
}


void UCameraRigComponent::ClearActionCameraRequest()
{
	ClearCameraRequest();
}

//Spline移動カメラを要求
void UCameraRigComponent::RequestSplineMoveCamera()
{
	if (!StartCameraRequest(ECameraRequestType::SplineMove)) return;

	//Spline移動終了通知で解除する
	m_CameraRequest.Duration = 0.f;
}

//攻撃時に敵を見るカメラを要求
void UCameraRigComponent::RequestAttackLookCamera(AActor* TargetActor)
{
	if (!TargetActor) return;

	RequestJustEvasiveLookCamera(TargetActor, false);
}

//カメラ要求を開始
bool UCameraRigComponent::StartCameraRequest(ECameraRequestType RequestType)
{
	const bool bClearCameraActive =
		m_CameraRequest.bIsActive &&
		m_CameraRequest.RequestType == ECameraRequestType::Clear;

	//クリア中は他のカメラ要求で上書きしない
	if (bClearCameraActive && RequestType != ECameraRequestType::Clear)
	{
		return false;
	}

	m_CameraRequest.Reset();
	m_CameraRequest.Start(RequestType);

	m_CurrentMode = ECameraMode::Action;

	return true;
}

//カメラ要求を更新
void UCameraRigComponent::UpdateCameraRequest(float DeltaTime)
{
	if (!m_CameraRequest.bIsActive) return;

	m_CameraRequest.Tick(DeltaTime);

	if (m_CameraRequest.TargetInfo.bHasTarget)
	{
		m_CameraRequest.TargetInfo.UpdateTargetLocation();
	}
}

//終了済みのカメラ要求を解除
void UCameraRigComponent::ClearFinishedCameraRequest()
{
	if (!m_CameraRequest.bIsActive) return;
	if (!m_CameraRequest.IsFinished()) return;

	ClearCameraRequest();
}

//Boss戦開始時の処理
void UCameraRigComponent::OnBossBattleStarted()
{
	TryCacheBossTarget();

	SetBossBattleMode(true);
}

//Boss戦終了時の処理
void UCameraRigComponent::OnBossBattleEnded()
{
	SetBossBattleMode(false);
	ClearBossTarget();
}

//特定レイヤーの有効状態を設定
void UCameraRigComponent::SetCameraLayerActive(TSubclassOf<UCameraLayer> LayerClass, bool bActive)
{
	if (!LayerClass) return;

	//登録順に各レイヤーの補正を積み重ねる
	for (UCameraLayer* Layer : m_CameraPipeline)
	{
		if (!Layer) continue;
		if (!Layer->IsA(LayerClass)) continue;

		Layer->SetLayerActive(bActive);
	}
}

//カメラ入力があったことを通知
void UCameraRigComponent::NotifyCameraInput()
{
	m_TimeSinceLastCameraInput = 0.f;
}

//カメラ状態をリセット
void UCameraRigComponent::ResetCameraState()
{
	//カメラ要求とBoss戦状態を通常状態へ初期化
	m_CurrentMode = ECameraMode::Normal;
	m_bIsBossBattle = false;
	m_BossTarget = nullptr;
	m_CameraRequest.Reset();

	m_TimeSinceLastCameraInput = CameraInputActiveTime;
	m_bIsFirstTick = true;

	AActor* OwnerActor = GetOwner();

	if (APlayerController* PC =
		OwnerActor
		? OwnerActor->GetInstigatorController<APlayerController>()
		: nullptr)
	{
		const float OwnerYaw = OwnerActor ? OwnerActor->GetActorRotation().Yaw : PC->GetControlRotation().Yaw;

		PC->SetControlRotation(FRotator(0.f, OwnerYaw, 0.f));

		if (PC->PlayerCameraManager && m_CameraParam)
		{
			PC->PlayerCameraManager->ViewPitchMin = m_CameraParam->PitchMin;
			PC->PlayerCameraManager->ViewPitchMax = m_CameraParam->PitchMax;
		}
	}

	if (m_SpringArmRef && m_CameraParam)
	{
		//死亡カメラでAbsolute化されたSpringArmを通常状態へ戻す
		m_SpringArmRef->SetUsingAbsoluteLocation(false);
		m_SpringArmRef->SetUsingAbsoluteRotation(false);

		if (m_bHasDefaultSpringArmTransform)
		{
			m_SpringArmRef->SetRelativeLocation(m_DefaultSpringArmRelativeLocation);
			m_SpringArmRef->SetRelativeRotation(m_DefaultSpringArmRelativeRotation);
		}
		else
		{
			m_SpringArmRef->SetRelativeRotation(FRotator::ZeroRotator);
		}

		m_SpringArmRef->bUsePawnControlRotation = true;
		m_SpringArmRef->bInheritPitch = true;
		m_SpringArmRef->bInheritYaw = true;
		m_SpringArmRef->bInheritRoll = false;

		m_SpringArmRef->bDoCollisionTest = true;
		m_SpringArmRef->bEnableCameraLag = true;
		m_SpringArmRef->bEnableCameraRotationLag = false;

		m_SpringArmRef->TargetArmLength = m_CameraParam->BaseArmLength;
		m_SpringArmRef->SocketOffset = m_CameraParam->BaseSocketOffset;
		m_SpringArmRef->TargetOffset = m_CameraParam->BaseTargetOffset;
		m_SpringArmRef->CameraLagSpeed = m_CameraParam->BaseLagSpeed;
		m_SpringArmRef->CameraRotationLagSpeed = m_CameraParam->BaseRotationLagSpeed;
	}

	if (m_CameraRef && m_CameraParam)
	{
		m_CameraRef->SetRelativeLocation(FVector::ZeroVector);
		m_CameraRef->SetRelativeRotation(FRotator::ZeroRotator);
		m_CameraRef->SetFieldOfView(m_CameraParam->BaseFOV);
	}

	//各レイヤーの内部ブレンド値を完全破棄する
	BuildCameraPipeline();

}
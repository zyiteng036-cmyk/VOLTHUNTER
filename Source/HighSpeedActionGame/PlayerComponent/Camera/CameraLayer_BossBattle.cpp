#include "CameraLayer_BossBattle.h"
#include "../../DataAssets/Player/PlayerCameraParameter.h"

//初期化
UCameraLayer_BossBattle::UCameraLayer_BossBattle()
{
}

//レイヤー処理を適用
void UCameraLayer_BossBattle::ApplyLayer(FCameraContext& Context)
{
	if (!Context.CameraParam) return;
	if (!CanApplyLayer(Context)) return;
	if (!ShouldUpdateBossBattleCamera(Context)) return;

	const bool bCanUseBossCamera =
		Context.bIsBossBattle &&
		Context.bHasBossTarget;

	const float TargetBossBlendAlpha =
		bCanUseBossCamera ? 1.f : 0.f;

	m_CurrentBossBlendAlpha =
		FMath::FInterpTo(
			m_CurrentBossBlendAlpha,
			TargetBossBlendAlpha,
			Context.DeltaTime,
			Context.CameraParam->BossBattleBlendSpeed
		);

	if (m_CurrentBossBlendAlpha <= KINDA_SMALL_NUMBER && !bCanUseBossCamera)
	{
		ResetBossCameraRuntime();
		return;
	}

	if (!bCanUseBossCamera)
	{
		Context.TargetArmLength =
			FMath::Lerp(
				Context.TargetArmLength,
				m_CurrentBossArmLength,
				m_CurrentBossBlendAlpha
			);

		Context.FinalFOV =
			FMath::Lerp(
				Context.FinalFOV,
				m_CurrentBossFOV,
				m_CurrentBossBlendAlpha
			);

		Context.CumulativeSocketOffset +=
			FVector(0.f, 0.f, Context.CameraParam->BossFramingHeightOffset) *
			m_CurrentBossBlendAlpha;

		DebugBossCameraLog(
			Context,
			0.f,
			0.f,
			m_CurrentBossArmLength,
			m_CurrentBossFOV
		);

		return;
	}

	InitializeBossCameraRuntime(Context);

	const float RawDistanceToBoss =
		GetHorizontalDistanceToBoss(Context);

	m_CurrentBossDistance =
		FMath::FInterpTo(
			m_CurrentBossDistance,
			RawDistanceToBoss,
			Context.DeltaTime,
			Context.CameraParam->BossDistanceInterpSpeed
		);

	float DistanceAlpha =
		CalculateBossDistanceAlphaFromDistance(
			m_CurrentBossDistance,
			Context
		);

	//距離の線形変化を滑らかにする
	DistanceAlpha =
		DistanceAlpha * DistanceAlpha * (3.f - 2.f * DistanceAlpha);

	const float DesiredArmLength =
		CalculateBossArmLength(
			DistanceAlpha,
			Context
		);

	const float DesiredFOV =
		CalculateBossFOV(
			DistanceAlpha,
			Context
		);

	m_CurrentBossArmLength =
		FMath::FInterpTo(
			m_CurrentBossArmLength,
			DesiredArmLength,
			Context.DeltaTime,
			Context.CameraParam->BossArmLengthInterpSpeed
		);

	m_CurrentBossFOV =
		FMath::FInterpTo(
			m_CurrentBossFOV,
			DesiredFOV,
			Context.DeltaTime,
			Context.CameraParam->BossFOVInterpSpeed
		);

	Context.TargetArmLength =
		FMath::Lerp(
			Context.TargetArmLength,
			m_CurrentBossArmLength,
			m_CurrentBossBlendAlpha
		);

	Context.FinalFOV =
		FMath::Lerp(
			Context.FinalFOV,
			m_CurrentBossFOV,
			m_CurrentBossBlendAlpha
		);

	Context.CumulativeSocketOffset +=
		FVector(0.f, 0.f, Context.CameraParam->BossFramingHeightOffset) *
		m_CurrentBossBlendAlpha;

	DebugBossCameraLog(
		Context,
		RawDistanceToBoss,
		DistanceAlpha,
		DesiredArmLength,
		DesiredFOV
	);
}

//レイヤー名を取得
FName UCameraLayer_BossBattle::GetLayerName() const
{
	return TEXT("CameraLayer_BossBattle");
}

//Boss戦カメラを更新する必要があるか
bool UCameraLayer_BossBattle::ShouldUpdateBossBattleCamera(
	const FCameraContext& Context
) const
{
	if (Context.bIsBossBattle) return true;

	return m_CurrentBossBlendAlpha > KINDA_SMALL_NUMBER;
}

//Boss戦カメラの初期値を用意
void UCameraLayer_BossBattle::InitializeBossCameraRuntime(
	const FCameraContext& Context
)
{
	if (!m_bBossArmLengthInitialized)
	{
		m_CurrentBossArmLength = Context.TargetArmLength;
		m_bBossArmLengthInitialized = true;
	}

	if (!m_bBossFOVInitialized)
	{
		m_CurrentBossFOV = Context.FinalFOV;
		m_bBossFOVInitialized = true;
	}

	if (!m_bBossDistanceInitialized)
	{
		m_CurrentBossDistance = GetHorizontalDistanceToBoss(Context);
		m_bBossDistanceInitialized = true;
	}
}

//Boss戦カメラの実行時値をリセット
void UCameraLayer_BossBattle::ResetBossCameraRuntime()
{
	m_CurrentBossBlendAlpha = 0.f;
	m_CurrentBossArmLength = 0.f;
	m_CurrentBossFOV = 0.f;
	m_CurrentBossDistance = 0.f;

	m_bBossArmLengthInitialized = false;
	m_bBossFOVInitialized = false;
	m_bBossDistanceInitialized = false;
}

//Bossとの水平距離を取得
float UCameraLayer_BossBattle::GetHorizontalDistanceToBoss(
	const FCameraContext& Context
) const
{
	if (!Context.OwnerActor) return 0.f;
	if (!Context.bHasBossTarget) return 0.f;

	const FVector OwnerLocation =
		Context.OwnerActor->GetActorLocation();

	const FVector OwnerHorizontalLocation =
		FVector(OwnerLocation.X, OwnerLocation.Y, 0.f);

	const FVector BossHorizontalLocation =
		FVector(Context.BossLocation.X, Context.BossLocation.Y, 0.f);

	return FVector::Dist(
		OwnerHorizontalLocation,
		BossHorizontalLocation
	);
}

//Boss距離の割合を計算
float UCameraLayer_BossBattle::CalculateBossDistanceAlphaFromDistance(
	float DistanceToBoss,
	const FCameraContext& Context
) const
{
	if (!Context.CameraParam) return 0.f;

	const float NearDistance =
		Context.CameraParam->BossNearDistance;

	const float FarDistance =
		Context.CameraParam->BossFarDistance;

	if (FarDistance <= NearDistance)
	{
		return 0.f;
	}

	return FMath::Clamp(
		(DistanceToBoss - NearDistance) /
		(FarDistance - NearDistance),
		0.f,
		1.f
	);
}

//Boss距離からカメラ距離を計算
float UCameraLayer_BossBattle::CalculateBossArmLength(
	float DistanceAlpha,
	const FCameraContext& Context
) const
{
	if (!Context.CameraParam) return 0.f;

	return FMath::Lerp(
		Context.CameraParam->BossNearArmLength,
		Context.CameraParam->BossFarArmLength,
		DistanceAlpha
	);
}

//Boss距離からFOVを計算
float UCameraLayer_BossBattle::CalculateBossFOV(
	float DistanceAlpha,
	const FCameraContext& Context
) const
{
	if (!Context.CameraParam) return 0.f;

	return FMath::Lerp(
		Context.CameraParam->BaseFOV,
		Context.CameraParam->BossBattleFOV,
		DistanceAlpha
	);
}

//BossカメラDebugログを出す
void UCameraLayer_BossBattle::DebugBossCameraLog(
	const FCameraContext& Context,
	float RawDistanceToBoss,
	float DistanceAlpha,
	float DesiredArmLength,
	float DesiredFOV
) const
{
	if (!Context.CameraParam) return;
	if (!Context.CameraParam->bEnableBossCameraDebugLog) return;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossCamera] Boss:%s HasTarget:%s Blend:%f RawDist:%f SmoothDist:%f Alpha:%f DesiredArm:%f CurrentArm:%f ResultArm:%f DesiredFOV:%f CurrentFOV:%f ResultFOV:%f"),
		Context.bIsBossBattle ? TEXT("true") : TEXT("false"),
		Context.bHasBossTarget ? TEXT("true") : TEXT("false"),
		m_CurrentBossBlendAlpha,
		RawDistanceToBoss,
		m_CurrentBossDistance,
		DistanceAlpha,
		DesiredArmLength,
		m_CurrentBossArmLength,
		Context.TargetArmLength,
		DesiredFOV,
		m_CurrentBossFOV,
		Context.FinalFOV
	);
}
//担当
//伊藤直樹

//-----------------------------------------------------
//Boss戦カメラレイヤー
//
// Boss戦中のみ、Bossとの距離に応じてカメラ距離とFOVを調整する
// Bossに近い時は通常寄り、離れた時は引きと広いFOVの構図にする
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "CameraLayer.h"
#include "CameraLayer_BossBattle.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UCameraLayer_BossBattle : public UCameraLayer
{
	GENERATED_BODY()

public:
	//初期化
	UCameraLayer_BossBattle();

public:
	//レイヤー処理を適用
	virtual void ApplyLayer(FCameraContext& Context) override;

	//レイヤー名を取得
	virtual FName GetLayerName() const override;

private:
	//-----------------------------------------------------
	// 内部処理
	//-----------------------------------------------------

	//Boss戦カメラを更新する必要があるか
	bool ShouldUpdateBossBattleCamera(const FCameraContext& Context) const;

	//Boss戦カメラの初期値を用意
	void InitializeBossCameraRuntime(const FCameraContext& Context);

	//Boss戦カメラの実行時値をリセット
	void ResetBossCameraRuntime();

	//Bossとの水平距離を取得
	float GetHorizontalDistanceToBoss(const FCameraContext& Context) const;

	//Boss距離の割合を計算
	float CalculateBossDistanceAlphaFromDistance(
		float DistanceToBoss,
		const FCameraContext& Context
	) const;

	//Boss距離からカメラ距離を計算
	float CalculateBossArmLength(
		float DistanceAlpha,
		const FCameraContext& Context
	) const;

	//Boss距離からFOVを計算
	float CalculateBossFOV(
		float DistanceAlpha,
		const FCameraContext& Context
	) const;

	//BossカメラDebugログを出す
	void DebugBossCameraLog(
		const FCameraContext& Context,
		float RawDistanceToBoss,
		float DistanceAlpha,
		float DesiredArmLength,
		float DesiredFOV
	) const;

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//Boss戦カメラの現在ブレンド率
	UPROPERTY(VisibleAnywhere, Category = "Camera|BossBattle")
	float m_CurrentBossBlendAlpha = 0.f;

	//Boss戦中の現在カメラ距離
	UPROPERTY(VisibleAnywhere, Category = "Camera|BossBattle")
	float m_CurrentBossArmLength = 0.f;

	//Boss戦中の現在FOV
	UPROPERTY(VisibleAnywhere, Category = "Camera|BossBattle")
	float m_CurrentBossFOV = 0.f;

	//補間済みBoss距離
	UPROPERTY(VisibleAnywhere, Category = "Camera|BossBattle")
	float m_CurrentBossDistance = 0.f;

	//Bossカメラ距離を初期化済みか
	UPROPERTY(VisibleAnywhere, Category = "Camera|BossBattle")
	bool m_bBossArmLengthInitialized = false;

	//BossFOVを初期化済みか
	UPROPERTY(VisibleAnywhere, Category = "Camera|BossBattle")
	bool m_bBossFOVInitialized = false;

	//Boss距離を初期化済みか
	UPROPERTY(VisibleAnywhere, Category = "Camera|BossBattle")
	bool m_bBossDistanceInitialized = false;
};
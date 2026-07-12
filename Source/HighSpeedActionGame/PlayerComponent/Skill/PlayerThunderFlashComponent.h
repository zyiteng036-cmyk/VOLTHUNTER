//担当
//伊藤直樹

//-----------------------------------------------------
//ThunderFlashコンポーネント
//
// ThunderFlashのワープ待機、テレポート、コリジョン制御を担当する
// スキル選択、コスト消費、GAS起動はPlayerSkillComponentが担当する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Types/PlayerSkillTypes.h"
#include "PlayerThunderFlashComponent.generated.h"

class APlayerCharacter;
class UPlayerSkillParameter;

//ThunderFlashイベント通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerThunderFlashEventDelegate);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayerThunderFlashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//初期化
	UPlayerThunderFlashComponent();

protected:
	//開始時
	virtual void BeginPlay() override;

public:
	//毎フレーム更新
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

public:
	//-----------------------------------------------------
	// 初期化
	//-----------------------------------------------------

	//参照を初期化
	void InitializeThunderFlashComponent(
		APlayerCharacter* Player,
		UPlayerSkillParameter* SkillParameter
	);

	//スキルパラメータを設定
	void SetSkillParameter(UPlayerSkillParameter* SkillParameter)
	{
		m_SkillParameter = SkillParameter;
	}

public:
	//-----------------------------------------------------
	// ThunderFlash
	//-----------------------------------------------------

	//ThunderFlash開始
	void BeginThunderFlash();

	//ThunderFlash終了
	void EndThunderFlash();

	//ThunderFlashを強制リセット
	void ResetThunderFlash();

public:
	//-----------------------------------------------------
	// Getter
	//-----------------------------------------------------

	//ThunderFlash中か
	bool GetIsThunderFlashActive() const { return m_bIsThunderFlashActive; }

	//テレポート済みか
	bool GetHasTeleported() const { return m_RuntimeState.bTeleported; }

	//開始位置を取得
	const FVector& GetStartLocation() const { return m_RuntimeState.StartLocation; }

	//目標位置を取得
	const FVector& GetTargetLocation() const { return m_RuntimeState.TargetLocation; }

public:
	//-----------------------------------------------------
	// Event
	//-----------------------------------------------------

	//ThunderFlash開始通知
	UPROPERTY(BlueprintAssignable, Category = "Skill|ThunderFlash")
	FOnPlayerThunderFlashEventDelegate OnThunderFlashStarted;

	//ThunderFlashテレポート通知
	UPROPERTY(BlueprintAssignable, Category = "Skill|ThunderFlash")
	FOnPlayerThunderFlashEventDelegate OnThunderFlashTeleported;

	//ThunderFlash終了通知
	UPROPERTY(BlueprintAssignable, Category = "Skill|ThunderFlash")
	FOnPlayerThunderFlashEventDelegate OnThunderFlashEnded;

private:
	//-----------------------------------------------------
	// 内部処理
	//-----------------------------------------------------

	//ThunderFlash更新
	void UpdateThunderFlash(float DeltaTime);

	//目標位置を計算
	FVector CalculateTargetLocation() const;

	//テレポートを実行
	void ExecuteTeleport();

	//プレイヤーの見た目を設定
	void SetPlayerVisualVisible(bool bVisible);

	//開始時の共通状態を設定
	void ApplyStartState();

	//終了時の共通状態を戻す
	void RestoreEndState();

private:
	//-----------------------------------------------------
	// 参照
	//-----------------------------------------------------

	//プレイヤー
	UPROPERTY()
	TObjectPtr<APlayerCharacter> m_Player = nullptr;

	//スキルパラメータ
	UPROPERTY()
	TObjectPtr<UPlayerSkillParameter> m_SkillParameter = nullptr;

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//ThunderFlash中か
	UPROPERTY(VisibleAnywhere, Category = "Skill|ThunderFlash")
	bool m_bIsThunderFlashActive = false;

	//ThunderFlash中の一時状態
	UPROPERTY(VisibleAnywhere, Category = "Skill|ThunderFlash")
	FThunderFlashRuntimeState m_RuntimeState;
};
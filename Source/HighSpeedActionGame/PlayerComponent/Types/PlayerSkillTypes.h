//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤースキル共通定義
//
// スキル種別、スキル状態、スキル中の一時状態を定義する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "PlayerSkillTypes.generated.h"

//スキル種別
UENUM(BlueprintType)
enum class EPlayerSkillType : uint8
{
	//なし
	None,

	//雷閃
	ThunderFlash,

	//最大数
	MAX UMETA(Hidden)
};

//スキル状態
UENUM(BlueprintType)
enum class EPlayerSkillState : uint8
{
	//なし
	None,

	//選択中
	Selecting,

	//発動準備中
	Starting,

	//発動中
	Active,

	//終了中
	Ending,

	//使用不可
	Disabled
};

//スキル使用要求結果
USTRUCT(BlueprintType)
struct FPlayerSkillRequestResult
{
	GENERATED_BODY()

public:
	//受理されたか
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	bool bAccepted = false;

	//要求されたスキル
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	EPlayerSkillType SkillType = EPlayerSkillType::None;

	//失敗理由
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FName FailReason = NAME_None;
};

//スキル中の一時状態
USTRUCT(BlueprintType)
struct FPlayerSkillRuntimeState
{
	GENERATED_BODY()

public:
	//スキル発動可能か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	bool bCanSkillActive = true;

	//スキル発動中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	bool bIsSkillActive = false;

	//スキル選択入力をロック中か
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	bool bSelectLocked = false;

	//現在選択中のスキル
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	EPlayerSkillType SelectedSkill = EPlayerSkillType::ThunderFlash;

	//現在のスキル状態
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	EPlayerSkillState SkillState = EPlayerSkillState::None;

	//ヒットUIを表示済みか
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|UI")
	bool bHasShownHitUI = false;

public:
	//状態を初期化
	void Reset()
	{
		bCanSkillActive = true;
		bIsSkillActive = false;
		bSelectLocked = false;
		SelectedSkill = EPlayerSkillType::ThunderFlash;
		SkillState = EPlayerSkillState::None;
		bHasShownHitUI = false;
	}
};

//ThunderFlash中の一時状態
USTRUCT(BlueprintType)
struct FThunderFlashRuntimeState
{
	GENERATED_BODY()

public:
	//開始位置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|ThunderFlash")
	FVector StartLocation = FVector::ZeroVector;

	//目標位置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|ThunderFlash")
	FVector TargetLocation = FVector::ZeroVector;

	//遅延時間の経過量
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|ThunderFlash")
	float DelayElapsed = 0.f;

	//テレポート済みか
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|ThunderFlash")
	bool bTeleported = false;

public:
	//状態を初期化
	void Reset()
	{
		StartLocation = FVector::ZeroVector;
		TargetLocation = FVector::ZeroVector;
		DelayElapsed = 0.f;
		bTeleported = false;
	}
};

//スキル状態変更通知用
USTRUCT(BlueprintType)
struct FPlayerSkillStateChangeInfo
{
	GENERATED_BODY()

public:
	//変更前
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	EPlayerSkillState PreviousState = EPlayerSkillState::None;

	//変更後
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	EPlayerSkillState NewState = EPlayerSkillState::None;
};
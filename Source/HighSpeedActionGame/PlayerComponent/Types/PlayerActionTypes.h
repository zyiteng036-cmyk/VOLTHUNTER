//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤーアクション共通定義
//
// プレイヤー全体のアクション種別・状態を定義する
// 攻撃の細かい種類は PlayerAttackTypes.h 側で管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "PlayerActionTypes.generated.h"

//プレイヤー全体のアクション種別
UENUM(BlueprintType)
enum class EPlayerActionType : uint8
{
	//なし
	None,

	//移動
	Move,

	//ジャンプ
	Jump,

	//回避
	Evasive,

	//攻撃
	Attack,

	//スキル
	Skill,

	//ダメージ
	Damaged,

	//死亡
	Dead,

	//操作不能
	Disabled
};

//プレイヤー全体のアクション状態
UENUM(BlueprintType)
enum class EPlayerActionState : uint8
{
	//なし
	None,

	//移動
	Move,

	//ジャンプ
	Jump,

	//回避
	Evasive,

	//ジャスト回避反撃受付
	JustEvasiveCounter,

	//攻撃
	Attack,

	//スキル
	Skill,

	//ダメージ
	Damaged,

	//死亡
	Dead,

	//操作不能
	Disabled
};

//アクション状態変更通知用
USTRUCT(BlueprintType)
struct FPlayerActionStateChangeInfo
{
	GENERATED_BODY()

public:
	//変更前
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	EPlayerActionState PreviousState = EPlayerActionState::None;

	//変更後
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	EPlayerActionState NewState = EPlayerActionState::None;
};
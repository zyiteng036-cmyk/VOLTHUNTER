//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤーコンボテーブル
//
// 現在の攻撃タグ + 入力タグ から次に発動するGameplayAbilityを決定する
// X→X→Xだけでなく、X→Y→YやX→X→Yにも対応する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "PlayerComboTable.generated.h"

//コンボ派生1件分
USTRUCT(BlueprintType)
struct FPlayerComboLink
{
	GENERATED_BODY()

public:
	//現在の攻撃タグ
	//未設定なら攻撃開始用の入口として扱う
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FGameplayTag CurrentAttackTag;

	//入力タグ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FGameplayTag InputTag;

	//次に発動する攻撃Ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TSubclassOf<UGameplayAbility> NextAbilityClass = nullptr;

	//この派生に必要なタグ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FGameplayTagContainer RequiredTags;

	//この派生を禁止するタグ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FGameplayTagContainer BlockedTags;

	//次のAbility発動前にキャンセルするAbilityタグ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FGameplayTagContainer CancelAbilityTags;

	//現在の攻撃をキャンセルして次へ移るか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bCancelCurrentAttack = true;
};

UCLASS(BlueprintType)
class HIGHSPEEDACTIONGAME_API UPlayerComboTable : public UDataAsset
{
	GENERATED_BODY()

public:
	//コンボ派生一覧
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TArray<FPlayerComboLink> ComboLinks;

public:
	//現在攻撃タグと入力タグからコンボ派生を検索
	const FPlayerComboLink* FindComboLink(
		const FGameplayTag& CurrentAttackTag,
		const FGameplayTag& InputTag,
		const FGameplayTagContainer& OwnedTags
	) const
	{
		for (const FPlayerComboLink& Link : ComboLinks)
		{
			//入力タグが一致しない派生は除外
			if (!Link.InputTag.MatchesTagExact(InputTag))
			{
				continue;
			}

			//攻撃開始時は双方の攻撃タグが未設定の場合だけ一致させる
			const bool bCurrentAttackMatches =
				(!Link.CurrentAttackTag.IsValid() && !CurrentAttackTag.IsValid()) ||
				(Link.CurrentAttackTag.IsValid() && Link.CurrentAttackTag.MatchesTagExact(CurrentAttackTag));

			if (!bCurrentAttackMatches)
			{
				continue;
			}

			//必要な状態タグが不足している派生は使用しない
			if (!OwnedTags.HasAll(Link.RequiredTags))
			{
				continue;
			}

			//禁止状態タグを持っている派生は使用しない
			if (OwnedTags.HasAny(Link.BlockedTags))
			{
				continue;
			}

			//発動先が設定されていない不完全な派生を除外
			if (!Link.NextAbilityClass)
			{
				continue;
			}

			return &Link;
		}

		return nullptr;
	}
};
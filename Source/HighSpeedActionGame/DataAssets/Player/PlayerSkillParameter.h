//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤースキルパラメータ
//
//スキル選択と各スキル固有の調整値を管理する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerSkillParameter.generated.h"

class UUserWidget;

UCLASS(BlueprintType)
class HIGHSPEEDACTIONGAME_API UPlayerSkillParameter : public UDataAsset
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------
	//共通
	//-----------------------------------------------------

	//スキル選択入力を受け付けるしきい値
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Common")
	float SelectInputThreshold = 0.5f;

	//-----------------------------------------------------
	//ThunderFlash
	//-----------------------------------------------------

	//発動時に消費する電力
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|ThunderFlash")
	float ThunderFlashCost = 70.f;

	//発動時の移動距離
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|ThunderFlash")
	float ThunderFlashDistance = 600.f;

	//発動からワープするまでの待機時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|ThunderFlash")
	float ThunderFlashDelayTime = 0.4f;

	//攻撃命中時に表示するUI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|ThunderFlash|UI")
	TSubclassOf<UUserWidget> ThunderFlashHitUIClass = nullptr;
};
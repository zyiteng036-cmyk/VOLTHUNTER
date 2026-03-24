//佐々木奏太担当
//タイトルの英ジェットクラス
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleModeSelectWidge.generated.h"

UENUM(BlueprintType)
enum class ETitleMenuInputType : uint8
{
	MoveUpSelection UMETA(DisplayName = "MoveUpSelection"),
	MoveDownSelection UMETA(DisplayName = "MoveDownSelection"),
	Decision     UMETA(DisplayName = "Decision"),
	Cancel       UMETA(DisplayName = "Cancel")
};

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTitleMenuInput, ETitleMenuInputType, InputType);

UCLASS()
class HIGHSPEEDACTIONGAME_API UTitleModeSelectWidge : public UUserWidget
{
	GENERATED_BODY()

public:
	void NotifyInput(ETitleMenuInputType _inputType, int32 index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anim")
	bool GetIsAnimationPlaying()const { return IsAnimationPlaying; }

	UFUNCTION(BlueprintCallable, Category = "Anim")
	void SetIsAnimationPlaying(const bool _isAnimPlayung) { IsAnimationPlaying = _isAnimPlayung; }

protected:
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnTitleMenuInput OnTitleMenuInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	int32 CurrentIndex = 0;

	bool IsAnimationPlaying = false;//アニメーション再生したらイベントグラフ上でセットするフラグ
};

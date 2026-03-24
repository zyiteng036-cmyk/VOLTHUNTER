//佐々木奏太担当
//リザルトのウィジェットクラス

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultWidget.generated.h"

UENUM(BlueprintType)
enum class EResultMenuInputType : uint8
{
	MoveUpSelection UMETA(DisplayName = "MoveUpSelection"),
	MoveDownSelection UMETA(DisplayName = "MoveDownSelection"),
	Decision     UMETA(DisplayName = "Decision"),
};



/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResultMenuInput, EResultMenuInputType, InputType);

UCLASS()
class HIGHSPEEDACTIONGAME_API UResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void NotifyInput(const EResultMenuInputType _inputType,const int32 _index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anim")
	bool GetIsAnimationPlaying()const { return IsAnimationPlaying; }

	UFUNCTION(BlueprintCallable, Category = "Anim")
	void SetIsAnimationPlaying(const bool _isAnimPlayung) { IsAnimationPlaying = _isAnimPlayung; }
protected:
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnResultMenuInput OnResultMenuInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	int32 CurrentIndex = 0;

	bool IsAnimationPlaying = false;//アニメーション再生したらイベントグラフ上でセットするフラグ

};

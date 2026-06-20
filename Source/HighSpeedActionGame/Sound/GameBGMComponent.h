//担当
//伊藤直樹

//ゲーム内のサウンドを管理するクラス

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameBGMComponent.generated.h"

class USoundBase;
class UAudioComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HIGHSPEEDACTIONGAME_API UGameBGMComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UGameBGMComponent();


    // --- 呼び出し用 ---
    //通常インゲーム
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void PlayNormalBGM();
    //ボス戦
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void PlayBossBGM();
    //クリア時
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void PlayClearBGM();
    //曲を止める
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void StopBGM();

    // --- BPで設定する音源 ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM")
    USoundBase* NormalBGM;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM")
    USoundBase* BossBGM;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM")
    USoundBase* ClearBGM;

private:
    // 内部処理
    void PlayBGM(USoundBase* NewSound);

    UPROPERTY()
    UAudioComponent* CurrentAudioComponent;
};

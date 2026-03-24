//佐々木奏太担当
//ヒットストップ用コンポーネント

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitStopComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HIGHSPEEDACTIONGAME_API UHitStopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	// Sets default values for this component's properties
	UHitStopComponent();

    // ヒットストップを開始
    UFUNCTION(BlueprintCallable, Category = "HitStop")
    void StartHitStop(const float _customDuration = -1.f);

    // ヒットストップが現在アクティブかどうかを取得
    UFUNCTION(BlueprintCallable, Category = "HitStop")
    bool IsHitStopActive() const;


private:
    // ヒットストップ終了処理
    void EndHitStop();

public:
    // ヒットストップの持続時間（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitStop")
    float m_HitStopDuration = 0.05f;

    // ヒットストップ中の時間倍率（0に近いほど止まる）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitStop")
    float m_HitStopTimeDilation = 0.001f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;



private:
    // 元の時間倍率を保持
    float m_OriginalTimeDilation = 1.0f;

    // タイマー管理用のハンドル//停止、再開などできる
    UPROPERTY()
    FTimerHandle m_HitStopTimerHandle;

    //タイマー使用中かのstaticメンバー変数追加//同じフレームで衝突した場合IsTimerActiveだと上手く動作しなかったため
    static bool m_IsUseHitStop;
};

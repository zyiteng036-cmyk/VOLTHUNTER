//担当
//伊藤直樹

//-----------------------------------------------------
//ThunderTrailエフェクト
//
// ThunderFlash発動時にプレイヤーの移動軌跡を伸ばして表示する
// 終了時はその場に残し、細くしながらフェードアウトする
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderTrailEffect.generated.h"

class UNiagaraComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API AThunderTrailEffect : public AActor
{
	GENERATED_BODY()

public:
	//初期化
	AThunderTrailEffect();

protected:
	//開始時
	virtual void BeginPlay() override;

public:
	//毎フレーム更新
	virtual void Tick(float DeltaTime) override;

	//フェードアウト開始
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void BeginFadeOut();

private:
	//追従中の更新
	void UpdateTrailFollow();

	//フェードアウト中の更新
	void UpdateFadeOut(float DeltaTime);

	//固定回転を作成
	FRotator MakeFixedHorizontalRotation(const FRotator& SourceRotation) const;

private:
	//-----------------------------------------------------
	// Component
	//-----------------------------------------------------

	//Niagaraコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> m_NiagaraComponent = nullptr;

protected:
	//-----------------------------------------------------
	// Parameter
	//-----------------------------------------------------

	//スケールX計算の基準長さ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Setup")
	float m_BaseEffectLength = 10.f;

	//エフェクトの太さ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Setup")
	float m_ThicknessScale = 0.1f;

	//フェードアウト補間速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Fade")
	float m_FadeOutInterpSpeed = 10.f;

	//フェードアウト開始後の寿命
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Fade")
	float m_FadeOutLifeSpan = 0.5f;

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//フェードアウト中か
	UPROPERTY(VisibleAnywhere, Category = "Effect|Runtime")
	bool m_bIsFadeOut = false;

	//生成時の座標
	UPROPERTY(VisibleAnywhere, Category = "Effect|Runtime")
	FVector m_StartLocation = FVector::ZeroVector;

	//生成時の水平回転
	UPROPERTY(VisibleAnywhere, Category = "Effect|Runtime")
	FRotator m_FixedRotation = FRotator::ZeroRotator;
};
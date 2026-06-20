//担当
//伊藤直樹

//霹靂一閃風スキルのエフェクト管理クラス
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "ThunderTrailEffect.generated.h"

class UNiagaraComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API AThunderTrailEffect : public AActor
{
	GENERATED_BODY()

public:
	//コンストラクタ
	AThunderTrailEffect();

protected:
	//ゲーム開始時に呼ばれる処理
	virtual void BeginPlay() override;

public:
	//毎フレーム呼ばれる処理
	virtual void Tick(float DeltaTime) override;

	//エフェクトのフェードアウト
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void BeginFadeOut();

protected:
	//ナイアガラコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	UNiagaraComponent* m_NiagaraComp = nullptr;

	//エフェクト調整用パラメータ
	//スケールXを計算する際の基準となる長さ
	UPROPERTY(EditDefaultsOnly, Category = "Effect Setup")
	float m_BaseEffectLength = 10.0f;

	//エフェクトの太さ
	UPROPERTY(EditDefaultsOnly, Category = "Effect Setup")
	float m_ThicknessScale = 0.1f;

	//スキル終了時エフェクトが細くなって消えていく速度
	UPROPERTY(EditDefaultsOnly, Category = "Effect|Fade")
	float m_FadeOutInterpSpeed = 10.0f;

	//フェードアウト開始からActorが完全に削除されるまでの時間(秒)
	UPROPERTY(EditDefaultsOnly, Category = "Effect|Fade")
	float m_FadeOutLifeSpan = 0.5f;

private:
	//フェードアウト処理中か
	bool m_IsFinished = false;

	//始点座標
	FVector m_StartLocation = FVector::ZeroVector;

	//最初の向き
	FRotator m_FixedRotation = FRotator::ZeroRotator;
};
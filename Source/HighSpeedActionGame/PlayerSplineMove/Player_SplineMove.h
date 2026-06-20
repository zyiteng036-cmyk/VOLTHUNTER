//担当
//伊藤直樹

//スプライン曲線に沿ってプレイヤーアクターを走らせる、演出を兼ねた移動制御クラス
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player_SplineMove.generated.h"

class APlayerCharacter;
class USplineComponent;
class UBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UPlayer_EvasiveComponent;
class UPlayer_AttackComponent;
class UPlayer_SkillComponent;
class UWidgetComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API APlayer_SplineMove : public AActor
{
	GENERATED_BODY()

public:
	//コンストラクタ
	APlayer_SplineMove();

protected:
	//ゲーム開始時の初期化
	virtual void BeginPlay() override;

public:
	//毎フレームの更新処理
	virtual void Tick(float DeltaTime) override;

	//プレイヤー側から移動開始を要求
	void RequestStartSplineMove(AActor* RequestActor);

protected:
	//アクセス可能範囲に入った時
	UFUNCTION()
	void OnStartOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//アクセス可能範囲から出た時
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	//プレイヤーの参照
	TWeakObjectPtr<APlayerCharacter> m_Player = nullptr;

	//プレイヤーの各制御コンポーネント参照
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	TWeakObjectPtr<UPlayer_EvasiveComponent> m_EvasiveComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	TWeakObjectPtr<UPlayer_AttackComponent> m_AttackComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	TWeakObjectPtr<UPlayer_SkillComponent> m_SkillComponent = nullptr;

	//スプラインコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline", meta = (AllowPrivateAccess = "true"))
	USplineComponent* m_Spline = nullptr;

	//移動開始用のコリジョン
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* m_StartCollision = nullptr;

	//スプライン移動中のエフェクトコンポーネント
	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* m_SplineEffectComponent = nullptr;

	//適用するエフェクトアセット
	UPROPERTY(EditAnywhere, Category = "SplineMove|Effect")
	UNiagaraSystem* m_SplineMoveEffect = nullptr;

	//移動速度
	UPROPERTY(EditAnywhere, Category = "SplineMove")
	float m_MoveSpeed = 1500.0f;

	//UI表示用コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* m_InputPromptWidget = nullptr;

	//エディタからセットする移動中の効果音
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SplineMove|Sound")
	USoundBase* m_SplineMoveSound = nullptr;

	//音を再生・停止制御するためのコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SplineMove|Sound")
	UAudioComponent* m_MoveAudioComponent = nullptr;

private:
	//スプライン移動中かどうかのフラグ
	bool m_RunningSplineMove = false;

	//現在の進捗距離
	float m_CurrentDistance = 0.0f;

	//開始時の距離
	float m_StartDistance = 0.0f;

	//移動可能範囲内にいる候補アクター
	TWeakObjectPtr<AActor> m_CandidateActor = nullptr;

	//現在移動中のアクター
	TWeakObjectPtr<AActor> m_MovingActor = nullptr;
};
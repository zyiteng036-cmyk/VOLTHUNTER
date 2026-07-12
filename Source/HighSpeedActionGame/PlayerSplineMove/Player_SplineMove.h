//担当
//伊藤直樹

//-----------------------------------------------------
//Spline移動アクター
//
// プレイヤーが開始範囲内で入力した時、Splineに沿って自動移動させる
// 移動中は攻撃、回避、スキル、通常移動を停止し、終了時に復帰する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player_SplineMove.generated.h"

class APlayerCharacter;
class USplineComponent;
class UBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UWidgetComponent;
class UAudioComponent;
class USoundBase;
class UPlayerAttackComponent;
class UPlayerEvasiveComponent;
class UPlayerSkillComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API APlayer_SplineMove : public AActor
{
	GENERATED_BODY()

public:
	//初期化
	APlayer_SplineMove();

protected:
	//開始時
	virtual void BeginPlay() override;

public:
	//毎フレーム更新
	virtual void Tick(float DeltaTime) override;

public:
	//-----------------------------------------------------
	// SplineMove
	//-----------------------------------------------------

	//プレイヤー側からSpline移動開始を要求
	UFUNCTION(BlueprintCallable, Category = "SplineMove")
	void RequestStartSplineMove(AActor* RequestActor);

	//Spline移動中か
	UFUNCTION(BlueprintPure, Category = "SplineMove")
	bool IsRunningSplineMove() const { return m_bRunningSplineMove; }

	//開始候補アクターか
	UFUNCTION(BlueprintPure, Category = "SplineMove")
	bool IsCandidateActor(const AActor* Actor) const { return m_CandidateActor.Get() == Actor; }

protected:
	//-----------------------------------------------------
	// Overlap
	//-----------------------------------------------------

	//開始範囲に入った時
	UFUNCTION()
	void OnStartOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	//開始範囲から出た時
	UFUNCTION()
	void OnEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	//-----------------------------------------------------
	// 内部処理
	//-----------------------------------------------------

	//プレイヤー関連コンポーネントを取得
	bool CachePlayerComponents(APlayerCharacter* PlayerCharacter);

	//Spline移動を開始できるか
	bool CanStartSplineMove(AActor* RequestActor) const;

	//Spline移動を開始
	void BeginSplineMove(AActor* RequestActor);

	//Spline移動を更新
	void TickSplineMove(float DeltaTime);

	//Spline移動を終了
	void FinishSplineMove();

	//移動開始時のロックを適用
	void ApplySplineMoveLock(AActor* TargetActor);

	//移動終了時のロックを解除
	void RestoreSplineMoveLock(AActor* TargetActor);

	//攻撃、回避、スキルを停止
	void DisablePlayerActions();

	//攻撃、回避、スキルを復帰
	void RestorePlayerActions();

	//Spline演出を開始
	void StartSplineMoveEffect(const FVector& Location, const FRotator& Rotation);

	//Spline演出を停止
	void StopSplineMoveEffect();

	//移動音を開始
	void StartMoveSound(const FVector& Location);

	//移動音を停止
	void StopMoveSound();

	//入力案内UIを表示
	void ShowInputPrompt();

	//入力案内UIを非表示
	void HideInputPrompt();

private:
	//-----------------------------------------------------
	// Player
	//-----------------------------------------------------

	//候補プレイヤー
	UPROPERTY()
	TWeakObjectPtr<APlayerCharacter> m_Player = nullptr;

	//攻撃コンポーネント
	UPROPERTY()
	TWeakObjectPtr<UPlayerAttackComponent> m_AttackComponent = nullptr;

	//回避コンポーネント
	UPROPERTY()
	TWeakObjectPtr<UPlayerEvasiveComponent> m_EvasiveComponent = nullptr;

	//スキルコンポーネント
	UPROPERTY()
	TWeakObjectPtr<UPlayerSkillComponent> m_SkillComponent = nullptr;

private:
	//-----------------------------------------------------
	// Component
	//-----------------------------------------------------

	//移動用Spline
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SplineMove", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineComponent> m_Spline = nullptr;

	//開始判定コリジョン
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SplineMove", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> m_StartCollision = nullptr;

	//Spline移動エフェクト
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SplineMove|Effect", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> m_SplineEffectComponent = nullptr;

	//入力案内UI
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SplineMove|UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> m_InputPromptWidget = nullptr;

	//移動音コンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SplineMove|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> m_MoveAudioComponent = nullptr;

private:
	//-----------------------------------------------------
	// Parameter
	//-----------------------------------------------------

	//Spline移動速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SplineMove", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float m_MoveSpeed = 1500.f;

	//Splineの一番近い地点から開始するか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SplineMove", meta = (AllowPrivateAccess = "true"))
	bool m_bStartFromClosestPoint = false;

	//移動中にメッシュを非表示にするか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SplineMove", meta = (AllowPrivateAccess = "true"))
	bool m_bHideMovingActorMesh = true;

	//移動中に移動入力を無効化するか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SplineMove", meta = (AllowPrivateAccess = "true"))
	bool m_bDisableMoveInput = true;

	//Spline移動エフェクト
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SplineMove|Effect", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraSystem> m_SplineMoveEffect = nullptr;

	//Spline移動中の効果音
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SplineMove|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> m_SplineMoveSound = nullptr;

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//Spline移動中か
	UPROPERTY(VisibleAnywhere, Category = "SplineMove|Runtime")
	bool m_bRunningSplineMove = false;

	//現在のSpline距離
	UPROPERTY(VisibleAnywhere, Category = "SplineMove|Runtime")
	float m_CurrentDistance = 0.f;

	//開始時のSpline距離
	UPROPERTY(VisibleAnywhere, Category = "SplineMove|Runtime")
	float m_StartDistance = 0.f;

	//開始候補アクター
	UPROPERTY()
	TWeakObjectPtr<AActor> m_CandidateActor = nullptr;

	//移動中アクター
	UPROPERTY()
	TWeakObjectPtr<AActor> m_MovingActor = nullptr;
};
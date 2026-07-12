//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー回避コンポーネント
//
// 回避中のジャスト回避判定と成功時の演出・通知を担当する
// 回避移動そのものはCustomCharacterMovementComponentとGA_Evasiveへ委譲する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerEvasiveComponent.generated.h"

class APlayerCharacter;
class UCapsuleComponent;
class UUserWidget;
class UHitStopComponent;
class UPlayerElectroGaugeComponent;

//ジャスト回避成功通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlayerJustEvasiveSuccessDelegate,
	AActor*, Attacker
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayerEvasiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//初期化
	UPlayerEvasiveComponent();

protected:
	//開始時
	virtual void BeginPlay() override;

public:
	//毎フレーム更新
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

public:
	//回避開始時
	void BeginEvasive();

	//回避終了時
	void EndEvasive();

	//被ダメージ前にジャスト回避を試行
	bool TryHandleJustEvasiveBeforeDamage();

	//被ダメージ前にジャスト回避を試行
	bool TryHandleJustEvasiveBeforeDamage(AActor* Attacker);

	//ジャスト回避判定Collision生成
	UFUNCTION(BlueprintCallable, Category = "Evasive|Just")
	void CreateJustEvasiveCollision();

	//ジャスト回避判定Collision破棄
	UFUNCTION(BlueprintCallable, Category = "Evasive|Just")
	void DestroyJustEvasiveCollision();

	//回避可能状態を設定
	void SetCanEvasive(bool bCanEvasive) { m_bCanEvasive = bCanEvasive; }

	//回避中か設定
	void SetIsEvasive(bool bIsEvasive) { m_bIsEvasive = bIsEvasive; }

	//ジャスト回避中か設定
	void SetIsJustEvasive(bool bIsJustEvasive) { m_bIsJustEvasive = bIsJustEvasive; }

	//回避可能か
	bool GetCanEvasive() const { return m_bCanEvasive; }

	//回避中か
	bool GetIsEvasive() const { return m_bIsEvasive; }

	//ジャスト回避中か
	bool GetIsJustEvasive() const { return m_bIsJustEvasive; }

	//ジャスト回避受付中か
	bool IsJustEvasiveWindowOpen() const { return m_bJustEvasiveWindowOpen; }

	//回避状態を完全リセット
	void ResetEvasiveState();
public:
	//ジャスト回避成功通知
	UPROPERTY(BlueprintAssignable, Category = "Evasive|Event")
	FOnPlayerJustEvasiveSuccessDelegate OnJustEvasiveSuccess;

public:
	//ジャスト回避成功時に表示するUI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> JustEvasiveWidgetClass;

private:
	//ジャスト回避状態更新
	void UpdateJustEvasive(float DeltaTime);

	//スローモーション安全装置更新
	void UpdateSlowMotionSafety(float DeltaTime);

	//ジャスト回避成功処理
	void HandleJustEvasiveSuccess(AActor* Attacker);

	//ジャスト回避成功時に電力ゲージを加算
	void AddJustEvasiveGauge();

	//ジャスト回避UI表示
	void ShowJustEvasiveUI();

	//ジャスト回避UI非表示
	void HideJustEvasiveUI();

	//スロー演出開始
	void StartJustEvasiveSlowMotion();

	//スロー演出終了
	void RestoreGlobalTimeDilation();

	//ジャスト回避Overlap
	UFUNCTION()
	void OnJustEvasiveOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	//プレイヤー
	UPROPERTY()
	TObjectPtr<APlayerCharacter> m_Player = nullptr;

	//電力ゲージコンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerElectroGaugeComponent> m_ElectroGaugeComponent = nullptr;

	//ヒットストップコンポーネント
	UPROPERTY()
	TObjectPtr<UHitStopComponent> m_HitStopComponent = nullptr;

	//生成したWidgetインスタンス
	UPROPERTY()
	TObjectPtr<UUserWidget> m_JustEvasiveWidgetInstance = nullptr;

	//ジャスト回避判定Collision
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> m_JustEvasiveCollision = nullptr;

private:
	//回避中か
	UPROPERTY(VisibleAnywhere, Category = "Evasive")
	bool m_bIsEvasive = false;

	//ジャスト回避中か
	UPROPERTY(VisibleAnywhere, Category = "Evasive|Just")
	bool m_bIsJustEvasive = false;

	//回避可能か
	UPROPERTY(VisibleAnywhere, Category = "Evasive")
	bool m_bCanEvasive = true;

	//現在のジャスト回避時間
	UPROPERTY(VisibleAnywhere, Category = "Evasive|Just")
	float m_CurrentJustEvasiveTime = 0.f;

	//スローモーション監視用タイマー
	UPROPERTY(VisibleAnywhere, Category = "Evasive|Just")
	float m_CurrentSlowMotionWatchTime = 0.f;

private:
	//ジャスト回避の制限時間
	UPROPERTY(EditDefaultsOnly, Category = "Evasive|Just")
	float m_JustEvasiveLimitTime = 1.5f;

	//ジャスト回避成功時の電力ゲージ加算量
	UPROPERTY(EditDefaultsOnly, Category = "Evasive|Just")
	float m_JustEvasiveGaugeAddValue = 15.f;

	//ジャスト回避スロー倍率
	UPROPERTY(EditDefaultsOnly, Category = "Evasive|Just|Slow")
	float m_JustEvasiveSlowTimeScale = 0.2f;

	//ジャスト回避スロー時間
	UPROPERTY(EditDefaultsOnly, Category = "Evasive|Just|Slow")
	float m_JustEvasiveSlowDuration = 0.2f;

	//スローが残った時の安全解除時間
	UPROPERTY(EditDefaultsOnly, Category = "Evasive|Just|Slow")
	float m_SlowMotionSafetyLimit = 3.f;

	//ジャスト回避Collision半径
	UPROPERTY(EditDefaultsOnly, Category = "Evasive|Just|Collision")
	float m_JustEvasiveCollisionRadius = 80.f;

	//ジャスト回避Collision高さ
	UPROPERTY(EditDefaultsOnly, Category = "Evasive|Just|Collision")
	float m_JustEvasiveCollisionHalfHeight = 100.f;

	//ジャスト回避受付中か
	UPROPERTY(VisibleAnywhere, Category = "Evasive|Just")
	bool m_bJustEvasiveWindowOpen = false;
private:
	//スロー解除タイマー
	FTimerHandle m_SlowMotionTimerHandle;
};
//担当
//伊藤直樹
//プレイヤー回避クラス
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "../PlayerParam.h"
#include "Player_EvasiveComponent.generated.h"

class APlayerCharacter;
class UPlayer_MovementComponent;
class UPlayer_CameraComponent;
class UPlayer_AttackComponent;
class UPlayer_SkillComponent;
class UInputMappingContext;
class UInputAction;
class UCapsuleComponent;
class UPlayer_ElectroGaugeComponent;
class UHitStopComponent;
class UUserWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayer_EvasiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//コンストラクタ
	UPlayer_EvasiveComponent();

protected:
	//ゲーム開始時に呼ばれる処理
	virtual void BeginPlay() override;

public:
	//毎フレーム呼ばれる処理
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	//回避の終了や回避ストックの回復等を行う更新処理
	void _updateEvasive(float DeltaTime);

	//入力コールバック関連
	//回避スウェイ
	void Input_Evasive(const FInputActionValue& Value);

	//ジャスト回避成功時
	//回避コリジョン生成
	void CreateJustEvasiveCollision();

	//回避コリジョン消去
	void DestroyJustEvasiveCollision();

	//敵との攻撃判定
	//回避コリジョンと重なったら速度を遅く
	UFUNCTION()
	void OnJustEvasiveOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	//UIを消すためのヘルパー関数
	void HideJustEvasiveUI();

	//セッター
	void SetCanEvasive(bool CanEvasiveValue) { m_CanEvasive = CanEvasiveValue; }
	void SetIsEvasive(bool IsEvasiveValue) { m_IsEvasive = IsEvasiveValue; }
	void SetIsJustEvasive(bool IsJustEvasiveValue) { m_IsJustEvasive = IsJustEvasiveValue; }

	//ゲッター
	bool GetCanEvasive() const { return m_CanEvasive; }
	bool GetIsEvasive() const { return m_IsEvasive; }
	bool GetIsJustEvasive() const { return m_IsJustEvasive; }

private:
	//ジャスト回避が成功したとき電力ゲージを加算
	void OnJustEvasiveSuccess();

public:
	//ジャスト回避成功時に表示するUIクラスを設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> m_JustEvasiveWidgetClass = nullptr;

protected:
	//プレイヤーの参照
	UPROPERTY()
	APlayerCharacter* m_Player = nullptr;

	//プレイヤーのパラメーター
	FPlayerParam m_PlayerParam;

	//各コンポーネントの参照
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_MovementComponent* m_MovementComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_CameraComponent* m_CameraComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_AttackComponent* m_AttackComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_ElectroGaugeComponent* m_ElectroComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_SkillComponent* m_SkillComponent = nullptr;

private:
	//生成したWidgetインスタンス
	UPROPERTY()
	UUserWidget* m_JustEvasiveWidgetInstance = nullptr;

	//ジャスト回避のコリジョン
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> m_JustEvasiveCollision = nullptr;

	//===パラメータ===

	//回避スウェイ状態かどうか
	bool m_IsEvasive = false;

	//ジャスト回避中か
	bool m_IsJustEvasive = false;

	//回避の経過時間
	float m_EvasiveTime = 0.0f;

	//回避ストックの現在数(初期値は0、BeginPlay等でパラメータから取得)
	int32 m_CurrentEvasiveStock = 0;

	//1ストック回復に必要な時間
	float m_CurrentEvasiveRecoveryTime = 0.0f;

	//どの方向に回避するか
	FVector m_EvasiveDirection = FVector::ZeroVector;

	//現在のジャスト回避時間
	float m_CurrentJustEvasiveTime = 0.0f;

	//ジャスト回避の制限時間
	float m_JustEvasiveLimitTime = 1.5f;

	//回避できるか
	bool m_CanEvasive = true;

	//スローモーション監視用タイマー
	float m_CurrentSlowMotionWatchTime = 0.0f;
};
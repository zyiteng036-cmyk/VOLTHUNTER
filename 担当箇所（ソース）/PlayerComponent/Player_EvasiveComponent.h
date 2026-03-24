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

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayer_EvasiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayer_EvasiveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	//回避スウェイ
//回避の終了、回避ストックの回復
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

	void SetCanEvasive(bool _CanEvasive) { m_CanEvasive = _CanEvasive; }
	void SetIsEvasive(bool _IsEvasive) { m_IsEvasive = _IsEvasive; }
	void SetIsJustEvasive(bool IsJustEvasive) { m_IsJustEvasive = IsJustEvasive; }

	bool GetCanEvasive()const { return m_CanEvasive; }
	bool GetIsEvasive()const { return m_IsEvasive; }
	bool GetIsJustEvasive()const { return m_IsJustEvasive; }

private:
	//ジャスト回避が成功したとき電力ゲージを加算
	void OnJustEvasiveSuccess();



	//ジャスト回避のコリジョン
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> JustEvasiveCollision;

public:
	// UIを消すためのヘルパー関数
	void HideJustEvasiveUI();

	// ジャスト回避成功時に表示するUIクラスを設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> JustEvasiveWidgetClass;
protected:
	APlayerCharacter* m_Player;

	//プレイヤーのパラメーター
	FPlayerParam PlayerParam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_MovementComponent* m_MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_CameraComponent* m_CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_AttackComponent* m_AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_ElectroGaugeComponent* m_ElectroComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_SkillComponent* m_SkillComponent;

private:
	//回避スウェイ状態かどうか
	bool m_IsEvasive;

	//ジャスト回避中か
	bool m_IsJustEvasive;

	//回避の経過時間
	float m_EvasiveTime;

	//回避ストックの最大数
	int32 m_CurrentEvasiveStock;

	//1ストックに回復に必要な時間
	float m_CurrentEvasiveRecoveryTime;

	//どの方向に回避するか
	FVector m_EvasiveDirection;

	//現在のジャスト回避時間
	float m_CurrentJustEvasiveTime;
	//ジャスト回避の制限時間
	float m_JustEvasiveLimitTime;

	//回避できるか
	bool m_CanEvasive;

	// 生成したWidgetインスタンス
	UPROPERTY()
	UUserWidget* m_JustEvasiveWidgetInstance;

	// スローモーション監視用タイマー
	float m_CurrentSlowMotionWatchTime;

	// スローモーションがこの秒数（実時間）以上続いたら強制解除する
	const float SAFETY_SLOW_MOTION_LIMIT = 3.0f;
};

//担当 
//伊藤直樹

//カメラの管理クラス
#pragma once

#include "CoreMinimal.h"
#include "CameraComponent/ZoomInOut/CameraOverrideHandler.h"
#include "CameraComponent/OpeningCamera/CameraOpeningHandler.h"
#include "CameraComponent/CameraAction/CameraActionHandler.h"
#include "CameraComponent/CameraBoss/CameraBossHandler.h"
#include "CameraComponent/CameraDie/CameraDieHandler.h"
#include "Components/ActorComponent.h"
#include "../PlayerParam.h"
#include "CameraComponent/ClearCamera/ClearCameraParam/ClearCameraPhaseParam.h"
#include "InputActionValue.h"
#include "Player_CameraComponent.generated.h"

class APlayerCharacter;
class UInputMappingContext;
class UInputAction;
class UPlayer_MovementComponent;
class UPlayer_AttackComponent;
class UPlayer_EvasiveComponent;
class USpringArmComponent;
class UCameraComponent;
class AEnemyBase;

UENUM()
enum class ESkillCameraPhase : uint8
{
	None,
	ZoomIn,  //一瞬寄る
	ZoomOut  //引く
};

UENUM()
enum class EClearCameraPhase : uint8
{
	None,
	Side,
	FrontAngle
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayer_CameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayer_CameraComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	//追従の遅延を作るためのデッドゾーン(不感帯)判定処理
	void _updateCameraFocus(float DeltaTime);

	//ロックオン対象の敵の方向へカメラを自動で向ける処理
	void _updateEnemyDirectionCamera(float DeltaTime);

	//強攻撃チャージ中のズーム処理(距離制御をオーバーライドする場合はtrueを返す)
	bool _updateHeavyChargeCamera(float DeltaTime);

	//スキル使用時の演出用カメラ更新処理
	bool _updateSkillCamera(float DeltaTime);

	//ゲームクリア時のズームイン処理
	void _updateClearCameraZoom(float DeltaTime);

	//スキル発動に伴う演出カメラの開始通知
	void StartSkillCamera();

	//スキル演出用カメラの終了処理を実行し通常カメラへ戻す
	void EndSkillCamera();

	//ロックオン状態を解除する
	void CancellationLockOn();

	//現在再生中のカメラシェイクを強制終了する
	void CameraShakEnd();

	//指定したワールド座標がカメラの視界内に収まっているかを判定する
	bool IsLocationInCameraView(const FVector& WorldLocation) const;

	//===入力コールバック関連===

	//カメラのピッチ(上下)回転入力処理
	void Input_CameraRotatePitch(const FInputActionValue& Value);

	//カメラのヨー(左右)回転入力処理
	void Input_CameraRotateYaw(const FInputActionValue& Value);

	//カメラ位置の背面リセット入力処理
	void Input_CameraReset(const FInputActionValue& Value);

	//ジャスト回避成功時などに特定の対象へカメラを向けるようセットする
	void OnJEnemyDirection(const AActor* Target, bool _LowAngle = false);

	//ゲームクリア時の専用カメラ演出を開始する
	void ClearCamera();

	//カメラの各種状態やタイマーを初期状態にリセットする
	UFUNCTION(BlueprintCallable, Category = "Reset")
	void ResetCamera();

private:
	//背後からのクリア演出カメラへ切り替える
	void SetBackClearCamera();

	//真横からのクリア演出カメラへ切り替える
	void SetSideClearCamera();

	//斜め前方からのクリア演出カメラへ切り替える
	void SetFrontClearCamera();

public:
	//===アニメーション連動オーバーライド===

	//NotifyStateから呼び出されるカメラオーバーライドの開始処理
	void StartAnimCameraOverride(float TargetLength, float TargetFOV, float Speed);

	//NotifyStateから呼び出されるカメラオーバーライドの終了処理
	void EndAnimCameraOverride();

	//===オープニング関連===

	//ゲームモード等から呼び出されるインゲームオープニング演出の再生処理
	void PlayInGameOpening();

	//===汎用カメラ操作===

	//カメラの横位置(Y軸)にオフセットを適用する
	static constexpr float DefaultSideOffsetAmount = 150.0f; //横位置オフセットのデフォルト値
	void SetCameraSideOffset(bool bIsSideView, float OffsetAmount = DefaultSideOffsetAmount);

	//プレイヤー死亡時の専用カメラ処理を実行する
	void OnPlayerDie();

private:
	//バネモデルに基づく滑らかな曲線補間(SmoothDamp)の計算処理
	FVector SmoothDamp(FVector Current, FVector Target, FVector& Velocity, float SmoothTime, float DeltaTime);

	//プレイヤーの現在のアクション状態から適切なカメラ追従時間を取得する
	float GetCameraFollowTime() const;

	//ボス戦開始のイベント通知を受け取る
	UFUNCTION()
	void OnBossBattleStart();

	//ボス戦終了(討伐時など)のイベント通知を受け取る
	UFUNCTION()
	void OnBossBattleEnd();

protected:
	UPROPERTY()
	APlayerCharacter* m_Player = nullptr;

	UPROPERTY()
	FPlayerParam m_PlayerParam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_MovementComponent* m_MovementComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_AttackComponent* m_AttackComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_EvasiveComponent* m_EvasiveComponent = nullptr;

	UPROPERTY()
	EClearCameraPhase m_ClearCameraPhase = EClearCameraPhase::None;

	//背面クリアカメラ用設定パラメータ
	UPROPERTY(EditAnywhere, Category = "ClearCamera|Phase")
	FClearCameraPhaseParam m_BackClearParam;

	//側面クリアカメラ用設定パラメータ
	UPROPERTY(EditAnywhere, Category = "ClearCamera|Phase")
	FClearCameraPhaseParam m_SideClearParam;

	//正面クリアカメラ用設定パラメータ
	UPROPERTY(EditAnywhere, Category = "ClearCamera|Phase")
	FClearCameraPhaseParam m_FrontClearParam;

public:
	//オープニング演出管理ハンドラ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Camera Opening")
	UCameraOpeningHandler* m_OpeningHandler = nullptr;

	//オープニング演出中かどうかを示すフラグ
	bool m_IsOpeningCamera = false;

	//ボス戦カメラ演出管理ハンドラ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Boss Camera")
	UCameraBossHandler* m_BossHandler = nullptr;

	//死亡時カメラ演出管理ハンドラ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Die Camera")
	UCameraDieHandler* m_DieHandler = nullptr;

private:
	//一時的な演出によるカメラオーバーライド管理ハンドラ
	UPROPERTY()
	UCameraOverrideHandler* m_OverrideHandler = nullptr;

	//アクション連動カメラ管理ハンドラ
	UPROPERTY()
	UCameraActionHandler* m_ActionHandler = nullptr;

private:
	//===コンポーネント参照===

	//カメラを追従させるためのスプリングアーム
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> m_SpringArm = nullptr;

	//メインのカメラコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> m_Camera = nullptr;

	//プレイヤーの入力を管理するコントローラー
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerController, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<APlayerController> m_PlayerController = nullptr;

	//現在ロックオン対象となっている敵
	TWeakObjectPtr<const AActor> m_EnemyTarget = nullptr;

	//敵の方向を向く際の補間速度
	float m_EnemyDirectionInterpSpeed = 15.0f;

	//現在敵の方向へ自動で向いている最中かどうかのフラグ
	bool m_IsEnemyDirectionLooking = false;

	//ロックオン中かどうかのフラグ
	bool m_IsTargetLockOn = false;

	//カメラピッチ(上下)の最小可動域
	float m_MinPitch = -50.0f;

	//カメラピッチ(上下)の最大可動域
	float m_MaxPitch = 20.0f;

	//最後にプレイヤーから手動のカメラ操作入力があった時間
	float m_LastCamerainputTime = 0.0f;

	//手動操作後にカメラが自動追従へ戻るまでの待機時間
	float m_CameraAutoDelay = 1.0f;

	//カメラの自動追従時の補間速度
	float m_CameraReturnSpeed = 0.8f;

	//===強攻撃チャージ関連===

	//強攻撃チャージ時にカメラを近づける目標距離
	float m_HeavyChargeArmLength = 250.0f;

	//強攻撃チャージ時のカメラ接近補間速度
	float m_HeavyChargeInterpSpeed = 6.0f;

	//===スキルカメラ関連===

	//現在のスキルカメラの進行フェーズ
	ESkillCameraPhase m_SkillCameraPhase = ESkillCameraPhase::None;

	//スキル発動時に一瞬寄る際のカメラ距離
	float m_SkillZoomInArmLength = 200.0f;

	//スキル発動後にカメラを引く際の目標距離
	float m_SkillZoomOutArmLength = 800.0f;

	//スキル発動時に一瞬寄る際の補間速度(速め)
	float m_SkillZoomInSpeed = 10.0f;

	//スキル発動後にカメラを引く際の補間速度(少しゆっくり)
	float m_SkillZoomOutSpeed = 6.0f;

	//===クリアカメラ関連===

	//ゲームクリア演出用カメラが有効かどうかのフラグ
	bool m_IsClearCamera = false;

	//クリア演出の各フェーズ移行を管理するタイマーハンドル
	FTimerHandle m_SideClearTimer;
	FTimerHandle m_FrontClearTimer;
	FTimerHandle m_BackClearTimer;

	//クリア演出中の現在のカメラ距離
	float m_ClearCurrentArmLength = 0.0f;

	//クリア演出中の目標カメラ距離
	float m_ClearTargetArmLength = 0.0f;

	//クリア演出での最大ズーム長
	float m_ClearZoomLength = 300.0f;

	//クリア演出時のズーム補間速度
	float m_ClearZoomInterpSpeed = 0.3f;

	//===リセット復帰用保存値===

	float m_DefaultArmLength = 0.0f;
	FVector m_DefaultSocketOffset = FVector::ZeroVector;
	FRotator m_DefaultControlRotation = FRotator::ZeroRotator;
	bool m_DefaultUsePawnControlRotation = false;

	//===カメラ焦点管理===

	//SmoothDampで使用する現在の焦点座標
	FVector m_CameraFocusLocation = FVector::ZeroVector;

	//SmoothDampで使用する現在の焦点移動速度
	FVector m_CameraFocusVelocity = FVector::ZeroVector;

	//デッドゾーン判定用のカメラ目標座標
	FVector m_CameraTargetLocation = FVector::ZeroVector;

	//===オフセット補間===

	//スムーズに移行させるための目標ソケットオフセット
	FVector m_TargetSocketOffset = FVector::ZeroVector;

	//ソケットオフセットの補間速度
	UPROPERTY(EditAnywhere, Category = "Camera|Tutorial")
	float m_SideOffsetInterpSpeed = 5.0f;

	//X軸(左右)のカメラ操作を反転するかどうか
	bool m_bInvertCameraX = false;

	//Y軸(上下)のカメラ操作を反転するかどうか
	bool m_bInvertCameraY = false;
};
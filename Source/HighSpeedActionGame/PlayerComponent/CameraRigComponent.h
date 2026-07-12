//担当
//伊藤直樹

//-----------------------------------------------------
//カメラリグコンポーネント
//
// SpringArmとCameraを管理し、CameraLayerパイプラインを通して
// 最終的なカメラ距離、オフセット、FOV、回転補正を適用する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Types/CameraRigTypes.h"
#include "CameraRigComponent.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCameraParameter;
class UCameraLayer;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UCameraRigComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	//初期化
	UCameraRigComponent();

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
	//-----------------------------------------------------
	// Camera Mode
	//-----------------------------------------------------

	//カメラモードを設定
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(ECameraMode NewMode);

	//Boss戦状態を設定
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetBossBattleMode(bool bInBossBattle);

	//Bossターゲットを設定
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetBossTarget(AActor* BossActor);

	//Bossターゲットを解除
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ClearBossTarget();

public:
	//-----------------------------------------------------
	// Camera Request
	//-----------------------------------------------------


	//オープニングカメラを要求
	UFUNCTION(BlueprintCallable, Category = "Camera|Request")
	void RequestOpeningCamera();

	//クリアカメラを要求
	UFUNCTION(BlueprintCallable, Category = "Camera|Request")
	void RequestClearCamera();

	//死亡カメラを要求
	UFUNCTION(BlueprintCallable, Category = "Camera|Request")
	void RequestDieCamera();

	//スキルカメラを要求
	UFUNCTION(BlueprintCallable, Category = "Camera|Request")
	void RequestSkillCamera();

	//ジャスト回避後の敵注視カメラを要求
	UFUNCTION(BlueprintCallable, Category = "Camera|Request")
	void RequestJustEvasiveLookCamera(AActor* TargetActor, bool bLowAngle);

	//カメラ要求を終了
	UFUNCTION(BlueprintCallable, Category = "Camera|Request")
	void ClearCameraRequest();


	UFUNCTION(BlueprintCallable, Category = "Camera|Request")
	void ClearActionCameraRequest();

	//Spline移動カメラを要求
	UFUNCTION(BlueprintCallable, Category = "Camera|Request")
	void RequestSplineMoveCamera();

	//攻撃時に敵を見るカメラを要求
	UFUNCTION(BlueprintCallable, Category = "Camera|Request")
	void RequestAttackLookCamera(AActor* TargetActor);
public:
	//-----------------------------------------------------
	// Layer
	//-----------------------------------------------------

	//特定レイヤーの有効状態を設定
	UFUNCTION(BlueprintCallable, Category = "Camera|Layer")
	void SetCameraLayerActive(TSubclassOf<UCameraLayer> LayerClass, bool bActive);

public:
	//-----------------------------------------------------
	// Input
	//-----------------------------------------------------

	//カメラ入力があったことを通知
	void NotifyCameraInput();

public:
	//-----------------------------------------------------
	// Reset
	//-----------------------------------------------------

	//カメラ状態をリセット
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ResetCameraState();

public:
	//-----------------------------------------------------
	// Getter
	//-----------------------------------------------------

	//SpringArmを取得
	USpringArmComponent* GetSpringArm() const { return m_SpringArmRef; }

	//Cameraを取得
	UCameraComponent* GetCamera() const { return m_CameraRef; }

	//カメラパラメータを取得
	const UCameraParameter* GetCameraParameter() const { return m_CameraParam; }

	//Boss戦中か
	bool GetIsBossBattle() const { return m_bIsBossBattle; }

	//カメラ要求中か
	bool GetHasCameraRequest() const { return m_CameraRequest.bIsActive; }

	
	bool GetHasActionRequest() const { return GetHasCameraRequest(); }

private:
	//-----------------------------------------------------
	// 初期化
	//-----------------------------------------------------

	//参照を取得
	void CacheComponents();

	//SpringArmを初期化
	void InitializeSpringArm();

	//PlayerControllerのカメラ制限を設定
	void ApplyPlayerCameraLimit();

	//カメラパイプラインを構築
	void BuildCameraPipeline();

	//ワールドイベントを登録
	void BindWorldEvents();

	//Bossターゲットを自動取得
	void TryCacheBossTarget();

private:
	//-----------------------------------------------------
	// Tick
	//-----------------------------------------------------

	//安全なDeltaTimeを取得
	float GetSafeDeltaTime(float DeltaTime) const;

	//カメラContextを作成
	FCameraContext BuildCameraContext(float DeltaTime) const;

	//カメラパイプラインを実行
	void ExecuteCameraPipeline(FCameraContext& Context);

	//最終カメラ値を適用
	void ApplyCameraContext(const FCameraContext& Context);

	//カメラ相対位置を復帰
	void RecoverCameraRelativeLocation(float DeltaTime);


private:
	//-----------------------------------------------------
	// Request
	//-----------------------------------------------------

	//カメラ要求を開始
	bool StartCameraRequest(ECameraRequestType RequestType);

	//カメラ要求を更新
	void UpdateCameraRequest(float DeltaTime);

	//終了済みのカメラ要求を解除
	void ClearFinishedCameraRequest();

private:
	//-----------------------------------------------------
	// Event
	//-----------------------------------------------------

	//Boss戦開始時の処理
	UFUNCTION()
	void OnBossBattleStarted();

	//Boss戦終了時の処理
	UFUNCTION()
	void OnBossBattleEnded();

private:
	//-----------------------------------------------------
	// 参照
	//-----------------------------------------------------

	//SpringArm参照
	UPROPERTY()
	TObjectPtr<USpringArmComponent> m_SpringArmRef = nullptr;

	//Camera参照
	UPROPERTY()
	TObjectPtr<UCameraComponent> m_CameraRef = nullptr;

protected:
	//-----------------------------------------------------
	// DataAsset
	//-----------------------------------------------------

	//カメラパラメータ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Parameter")
	TObjectPtr<const UCameraParameter> m_CameraParam = nullptr;

protected:
	//-----------------------------------------------------
	// Boss
	//-----------------------------------------------------

	//Bossターゲットを自動検索するか
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Boss")
	bool m_bAutoFindBossTarget = true;

	//Boss検索に使うActorタグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Boss")
	FName m_BossActorTag = TEXT("Enemy.Boss");

	//初期SpringArm相対位置
	UPROPERTY(VisibleAnywhere, Category = "Camera|Runtime")
	FVector m_DefaultSpringArmRelativeLocation = FVector::ZeroVector;

	//初期SpringArm相対回転
	UPROPERTY(VisibleAnywhere, Category = "Camera|Runtime")
	FRotator m_DefaultSpringArmRelativeRotation = FRotator::ZeroRotator;

	//初期SpringArm相対Transformを保存済みか
	UPROPERTY(VisibleAnywhere, Category = "Camera|Runtime")
	bool m_bHasDefaultSpringArmTransform = false;
private:
	//-----------------------------------------------------
	// Pipeline
	//-----------------------------------------------------

	//カメラレイヤーパイプライン
	UPROPERTY()
	TArray<TObjectPtr<UCameraLayer>> m_CameraPipeline;

private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//現在のカメラモード
	UPROPERTY(VisibleAnywhere, Category = "Camera|Runtime")
	ECameraMode m_CurrentMode = ECameraMode::Normal;

	//Boss戦中か
	UPROPERTY(VisibleAnywhere, Category = "Camera|Runtime")
	bool m_bIsBossBattle = false;

	//Bossターゲット
	UPROPERTY()
	TWeakObjectPtr<AActor> m_BossTarget = nullptr;

	//カメラ要求
	UPROPERTY(VisibleAnywhere, Category = "Camera|Runtime")
	FCameraRequest m_CameraRequest;

	//初回Tickか
	UPROPERTY(VisibleAnywhere, Category = "Camera|Runtime")
	bool m_bIsFirstTick = true;

	//最後のカメラ入力からの経過時間
	UPROPERTY(VisibleAnywhere, Category = "Camera|Runtime")
	float m_TimeSinceLastCameraInput = 0.f;


};
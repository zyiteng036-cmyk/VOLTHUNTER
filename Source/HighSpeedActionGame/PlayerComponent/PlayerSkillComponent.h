//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤースキルコンポーネント
//
// スキル全体の窓口を担当する
// スキル選択、使用可否判定、GAS発動、開始/終了通知を管理する
// ThunderFlashなどの個別処理は専用Componentへ委譲する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "Types/PlayerSkillTypes.h"
#include "PlayerSkillComponent.generated.h"

class APlayerCharacter;
class UPlayerSkillParameter;
class UPlayerThunderFlashComponent;
class UPlayerAttackComponent;
class UPlayerElectroGaugeComponent;
class UAbilitySystemComponent;
class UGameplayAbility;
class UUserWidget;
class UCameraRigComponent;

//スキル使用可能状態変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlayerSkillUsableChangedDelegate,
	bool, bIsUsable
);

//スキル状態変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPlayerSkillStateChangedDelegate,
	EPlayerSkillState, PreviousState,
	EPlayerSkillState, NewState
);

//スキル開始通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPlayerSkillStartedDelegate,
	FGameplayTag, SkillTag,
	EPlayerSkillType, SkillType
);

//スキル終了通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPlayerSkillEndedDelegate,
	FGameplayTag, SkillTag,
	EPlayerSkillType, SkillType
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayerSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//初期化
	UPlayerSkillComponent();

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
	// 入力
	//-----------------------------------------------------

	//スキル選択入力
	void InputSkillSelect(const FInputActionValue& Value);

	//スキル選択入力解除
	void InputSkillReleased();

	//スキル発動入力
	void InputSkillActive();

	//選択中スキルの発動を要求
	bool RequestActivateSelectedSkill();

public:
	//-----------------------------------------------------
	// GAS Abilityから呼ばれる関数
	//-----------------------------------------------------

	//スキル開始可能か
	bool CanStartSkill(EPlayerSkillType SkillType) const;

	//スキル開始
	void BeginSkillByType(
		const FGameplayTag& SkillTag,
		EPlayerSkillType SkillType
	);

	//スキル終了
	void EndSkillByType(
		const FGameplayTag& SkillTag,
		EPlayerSkillType SkillType,
		bool bWasCancelled
	);

public:
	//-----------------------------------------------------
	// 外部制御
	//-----------------------------------------------------

	//スキル発動可能状態を設定
	void SetCanSkillActive(bool bCanSkillActive);

	//スキルを強制終了
	void EndSkill();

	//現在選択中のスキルが使用可能か
	bool CheckCurrentSkillUsable() const;

	//スキル状態を完全リセット
	void ResetSkill();

public:
	//-----------------------------------------------------
	// Getter
	//-----------------------------------------------------

	//スキル発動中か
	bool GetIsSkillActive() const { return m_RuntimeState.bIsSkillActive; }

	//スキル発動可能か
	bool GetCanSkillActive() const { return m_RuntimeState.bCanSkillActive; }

	//現在選択中のスキル
	EPlayerSkillType GetSelectedSkill() const { return m_RuntimeState.SelectedSkill; }

	//現在実行中のスキル
	EPlayerSkillType GetCurrentSkillType() const { return m_CurrentSkillType; }

	//現在のスキル状態
	EPlayerSkillState GetSkillState() const { return m_RuntimeState.SkillState; }

	//現在のスキルタグ
	const FGameplayTag& GetCurrentSkillTag() const { return m_CurrentSkillTag; }

	//現在のスキル使用可能状態を取得
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsCurrentSkillUsable() const;

	//現在のスキル使用可能状態を強制通知
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void RefreshSkillUsableNotify();

	//初期スキルAbilityを付与
	void GiveInitialSkillAbilities();
public:
	//-----------------------------------------------------
	// Event
	//-----------------------------------------------------

	//スキル使用可能状態変更通知
	UPROPERTY(BlueprintAssignable, Category = "Skill|Event")
	FOnPlayerSkillUsableChangedDelegate OnSkillUsableChanged;

	//スキル状態変更通知
	UPROPERTY(BlueprintAssignable, Category = "Skill|Event")
	FOnPlayerSkillStateChangedDelegate OnSkillStateChanged;

	//スキル開始通知
	UPROPERTY(BlueprintAssignable, Category = "Skill|Event")
	FOnPlayerSkillStartedDelegate OnSkillStarted;

	//スキル終了通知
	UPROPERTY(BlueprintAssignable, Category = "Skill|Event")
	FOnPlayerSkillEndedDelegate OnSkillEnded;

private:
	//-----------------------------------------------------
	// 初期化
	//-----------------------------------------------------

	//参照を取得
	void CacheComponents();

	//サブコンポーネントを初期化
	void InitializeSubComponents();


private:
	//-----------------------------------------------------
	// 選択処理
	//-----------------------------------------------------

	//次のスキルを選択
	void SelectNextSkill();

	//前のスキルを選択
	void SelectPrevSkill();

	//スキル選択を設定
	void SetSelectedSkill(EPlayerSkillType SkillType);

private:
	//-----------------------------------------------------
	// 使用可否
	//-----------------------------------------------------

	//ThunderFlashが使用可能か
	bool CanUseThunderFlash() const;

	//スキルコストを支払えるか
	bool CanPaySkillCost(EPlayerSkillType SkillType) const;

	//スキルコストを消費
	bool ConsumeSkillCost(EPlayerSkillType SkillType);

	//地上スキルを使えるか
	bool CanUseGroundSkill() const;

private:
	//-----------------------------------------------------
	// 状態管理
	//-----------------------------------------------------

	//スキル状態を設定
	void SetSkillState(EPlayerSkillState NewState);

	//スキル開始時の共通状態を設定
	void ApplySkillStartState(
		const FGameplayTag& SkillTag,
		EPlayerSkillType SkillType
	);

	//スキル終了時の共通状態を戻す
	void ResetSkillState();

	//スキル使用可能通知を更新
	void UpdateSkillUsableNotify();

private:
	//-----------------------------------------------------
	// GAS
	//-----------------------------------------------------

	//選択中スキルのAbilityを取得
	TSubclassOf<UGameplayAbility> GetSelectedSkillAbilityClass() const;

	//ASCを取得
	UAbilitySystemComponent* GetASC() const;

private:
	//-----------------------------------------------------
	// Camera
	//-----------------------------------------------------

	//スキルカメラを開始
	void StartSkillCamera();

	//スキルカメラを終了
	void EndSkillCamera();

private:
	//-----------------------------------------------------
	// UI
	//-----------------------------------------------------

	//ヒットUIを更新
	void UpdateSkillHitUI();

	//ヒットUIを表示
	void ShowSkillHitUI();

	//ヒットUIを非表示
	void HideSkillHitUI();

private:
	//-----------------------------------------------------
	// 参照
	//-----------------------------------------------------

	//プレイヤー
	UPROPERTY()
	TObjectPtr<APlayerCharacter> m_Player = nullptr;

	//ThunderFlashコンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerThunderFlashComponent> m_ThunderFlashComponent = nullptr;

	//攻撃コンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerAttackComponent> m_AttackComponent = nullptr;

	//電力ゲージコンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerElectroGaugeComponent> m_ElectroGaugeComponent = nullptr;

	//カメラリグコンポーネント
	UPROPERTY()
	TObjectPtr<UCameraRigComponent> m_CameraRigComponent = nullptr;

protected:
	//-----------------------------------------------------
	// DataAsset
	//-----------------------------------------------------

	//スキルパラメータ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Parameter")
	TObjectPtr<UPlayerSkillParameter> m_SkillParameter = nullptr;

protected:
	//-----------------------------------------------------
	// Ability
	//-----------------------------------------------------

	//ThunderFlash用Ability
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Ability")
	TSubclassOf<UGameplayAbility> m_ThunderFlashAbilityClass = nullptr;

	//スキル選択順
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Select", meta = (AllowPrivateAccess = "true"))
	TArray<EPlayerSkillType> m_SkillSelectOrder =
	{
		EPlayerSkillType::ThunderFlash
	};
private:
	//-----------------------------------------------------
	// Runtime
	//-----------------------------------------------------

	//スキル中の一時状態
	UPROPERTY(VisibleAnywhere, Category = "Skill")
	FPlayerSkillRuntimeState m_RuntimeState;

	//現在のスキルタグ
	UPROPERTY(VisibleAnywhere, Category = "Skill")
	FGameplayTag m_CurrentSkillTag;

	//現在実行中のスキル
	UPROPERTY(VisibleAnywhere, Category = "Skill")
	EPlayerSkillType m_CurrentSkillType = EPlayerSkillType::None;

	//前回の使用可能状態
	UPROPERTY(VisibleAnywhere, Category = "Skill")
	bool m_bLastUsableState = false;

	//現在表示中のヒットUI
	UPROPERTY()
	TObjectPtr<UUserWidget> m_ActiveHitUIInstance = nullptr;
};
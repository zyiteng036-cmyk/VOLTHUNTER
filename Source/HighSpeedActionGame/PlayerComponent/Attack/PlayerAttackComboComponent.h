//担当
//伊藤直樹

//-----------------------------------------------------
//プレイヤー攻撃コンボコンポーネント
//
// 攻撃入力タグ、入力予約、ComboTable検索、
// 次攻撃Abilityの発動を担当する
//-----------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PlayerAttackComboComponent.generated.h"

class APlayerCharacter;
class UPlayerAttackComponent;
class UPlayerComboTable;
class UAbilitySystemComponent;
struct FPlayerComboLink;

//コンボ予約受付変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlayerAttackComboReserveWindowChangedDelegate,
	bool, bOpen
);

//コンボ発動受付変更通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlayerAttackComboActivationWindowChangedDelegate,
	bool, bOpen
);

//攻撃入力予約通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlayerAttackInputBufferedDelegate,
	FGameplayTag, InputTag
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayerAttackComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//初期化
	UPlayerAttackComboComponent();

protected:
	//開始時
	virtual void BeginPlay() override;

public:
	//-----------------------------------------------------
	// 初期化
	//-----------------------------------------------------

	//参照を初期化
	void InitializeComboComponent(
		APlayerCharacter* Player,
		UPlayerAttackComponent* AttackComponent,
		UPlayerComboTable* ComboTable
	);

	//コンボテーブルを設定
	void SetComboTable(UPlayerComboTable* ComboTable);

public:
	//-----------------------------------------------------
	// 入力処理
	//-----------------------------------------------------

	//攻撃入力タグを処理
	bool HandleAttackInputByTag(const FGameplayTag& InputTag);

	//弱攻撃要求
	bool RequestLightAttack();

	//強攻撃押下要求
	bool RequestHeavyAttackPressed();

	//強攻撃離し要求
	bool RequestHeavyAttackReleased();

public:
	//-----------------------------------------------------
	// Combo NotifyStateから呼ばれる処理
	//-----------------------------------------------------

	//コンボ予約受付開始
	void OpenComboReserveWindow();

	//コンボ予約受付終了
	void CloseComboReserveWindow();

	//コンボ発動受付開始
	void OpenComboActivationWindow();

	//コンボ発動受付終了
	void CloseComboActivationWindow();

	//予約入力からコンボ発動
	bool TryActivateBufferedCombo();

	//予約入力をクリア
	void ClearBufferedInput();

	//ジャスト回避カウンター予約受付開始
	void OpenJustEvasiveCounterReserveWindow();

	//ジャスト回避カウンター予約受付終了
	void CloseJustEvasiveCounterReserveWindow();

	//ジャスト回避カウンター発動受付開始
	void OpenJustEvasiveCounterActivationWindow();

	//ジャスト回避カウンター発動受付終了
	void CloseJustEvasiveCounterActivationWindow();

	//予約入力からジャスト回避カウンター発動
	bool TryActivateBufferedJustEvasiveCounter();

public:
	//-----------------------------------------------------
	// Getter
	//-----------------------------------------------------

	//予約入力があるか
	bool HasBufferedInput() const { return m_bHasBufferedInput; }

	//予約入力タグを取得
	const FGameplayTag& GetBufferedInputTag() const { return m_BufferedInputTag; }

	//コンボ予約受付中か
	bool IsComboReserveWindowOpen() const { return m_bComboReserveWindowOpen; }

	//コンボ発動受付中か
	bool IsComboActivationWindowOpen() const { return m_bComboActivationWindowOpen; }

	//弱攻撃入力タグを取得
	const FGameplayTag& GetLightAttackInputTag() const { return m_LightAttackInputTag; }

	//強攻撃押下タグを取得
	const FGameplayTag& GetHeavyAttackPressedInputTag() const { return m_HeavyAttackPressedInputTag; }

	//強攻撃離しタグを取得
	const FGameplayTag& GetHeavyAttackReleasedInputTag() const { return m_HeavyAttackReleasedInputTag; }

	//ジャスト回避カウンター予約受付中か
	bool IsJustEvasiveCounterReserveWindowOpen() const { return m_bJustEvasiveCounterReserveWindowOpen; }

	//ジャスト回避カウンター発動受付中か
	bool IsJustEvasiveCounterActivationWindowOpen() const { return m_bJustEvasiveCounterActivationWindowOpen; }
public:
	//-----------------------------------------------------
	// Event
	//-----------------------------------------------------

	//コンボ予約受付変更通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Combo")
	FOnPlayerAttackComboReserveWindowChangedDelegate OnComboReserveWindowChanged;

	//コンボ発動受付変更通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Combo")
	FOnPlayerAttackComboActivationWindowChangedDelegate OnComboActivationWindowChanged;

	//攻撃入力予約通知
	UPROPERTY(BlueprintAssignable, Category = "Attack|Combo")
	FOnPlayerAttackInputBufferedDelegate OnAttackInputBuffered;

private:
	//-----------------------------------------------------
	// 内部処理
	//-----------------------------------------------------

	//入力タグを予約
	void BufferAttackInput(const FGameplayTag& InputTag);

	//攻撃未開始状態から攻撃を開始
	bool TryStartAttackFromInput(const FGameplayTag& InputTag);

	//現在攻撃タグと入力タグからコンボ派生を探す
	const FPlayerComboLink* FindComboLinkFromCurrentState(
		const FGameplayTag& InputTag
	) const;

	//コンボ派生を発動
	bool TryActivateComboLink(const FPlayerComboLink& ComboLink);

	//AbilityタグでAbilityをキャンセル
	void CancelAbilitiesByTags(const FGameplayTagContainer& CancelTags);

	//ASCを取得
	UAbilitySystemComponent* GetASC() const;

	//攻撃開始入力を受け付けられるか
	bool CanStartAttackFromInput() const;

	//コンボ遷移前に受付窓を閉じる
	void CloseComboWindowsForTransition();

private:
	//-----------------------------------------------------
	// 参照
	//-----------------------------------------------------

	//プレイヤー
	UPROPERTY()
	TObjectPtr<APlayerCharacter> m_Player = nullptr;

	//攻撃コンポーネント
	UPROPERTY()
	TObjectPtr<UPlayerAttackComponent> m_AttackComponent = nullptr;

	//コンボテーブル
	UPROPERTY()
	TObjectPtr<UPlayerComboTable> m_ComboTable = nullptr;

private:
	//-----------------------------------------------------
	// 入力タグ
	//-----------------------------------------------------

	//弱攻撃入力タグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Tag|Input", meta = (AllowPrivateAccess = "true"))
	FGameplayTag m_LightAttackInputTag;

	//強攻撃押下タグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Tag|Input", meta = (AllowPrivateAccess = "true"))
	FGameplayTag m_HeavyAttackPressedInputTag;

	//強攻撃離しタグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Tag|Input", meta = (AllowPrivateAccess = "true"))
	FGameplayTag m_HeavyAttackReleasedInputTag;

private:
	//-----------------------------------------------------
	// 状態タグ
	//-----------------------------------------------------

	//コンボ予約受付中タグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Tag|State", meta = (AllowPrivateAccess = "true"))
	FGameplayTag m_ComboReserveWindowStateTag;

	//コンボ発動受付中タグ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Tag|State", meta = (AllowPrivateAccess = "true"))
	FGameplayTag m_ComboActivationWindowStateTag;

private:
	//-----------------------------------------------------
	// 入力予約状態
	//-----------------------------------------------------

	//予約された入力タグ
	UPROPERTY(VisibleAnywhere, Category = "Attack|Combo")
	FGameplayTag m_BufferedInputTag;

	//入力予約があるか
	UPROPERTY(VisibleAnywhere, Category = "Attack|Combo")
	bool m_bHasBufferedInput = false;

	//コンボ予約受付中か
	UPROPERTY(VisibleAnywhere, Category = "Attack|Combo")
	bool m_bComboReserveWindowOpen = false;

	//コンボ発動受付中か
	UPROPERTY(VisibleAnywhere, Category = "Attack|Combo")
	bool m_bComboActivationWindowOpen = false;

	//ジャスト回避カウンター予約受付中か
	UPROPERTY(VisibleAnywhere, Category = "Attack|JustEvasive")
	bool m_bJustEvasiveCounterReserveWindowOpen = false;

	//ジャスト回避カウンター発動受付中か
	UPROPERTY(VisibleAnywhere, Category = "Attack|JustEvasive")
	bool m_bJustEvasiveCounterActivationWindowOpen = false;

	//コンボ遷移処理中か
	UPROPERTY(VisibleAnywhere, Category = "Attack|Combo")
	bool m_bComboTransitionInProgress = false;

	//ジャスト回避強攻撃を予約後に入力を離したか
	UPROPERTY(VisibleAnywhere, Category = "Attack|JustEvasive")
	bool m_bBufferedJustEvasiveHeavyReleased = false;
};
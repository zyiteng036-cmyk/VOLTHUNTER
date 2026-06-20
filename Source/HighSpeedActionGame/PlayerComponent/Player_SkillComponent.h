//担当
//伊藤直樹
//スキルクラス
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "../PlayerCharacter.h"
#include "Player_SkillComponent.generated.h"

class UPostProcess;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUsableChanged, bool, bIsUsable);

UENUM()
enum class EPlayerSkill : uint8
{
	ThunderFlash UMETA(DisplayName = "ThunderFlash"),

	MAX UMETA(Hidden)
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HIGHSPEEDACTIONGAME_API UPlayer_SkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//コンストラクタ
	UPlayer_SkillComponent();

protected:
	//ゲーム開始時に呼ばれる処理
	virtual void BeginPlay() override;

public:
	//毎フレーム呼ばれる処理
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//===入力===
	//スキル選択入力
	void Input_SkillSelect(const FInputActionValue& Value);
	//スキル選択入力の解除
	void Input_SkillReleased();
	//スキル発動入力
	void Input_SkillActive();

	//スキルの終了処理
	void EndSkill();

	//雷閃(ThunderFlash)が使用可能か判定
	bool CanUseThunderFlash() const;

	//スキルが発動中か取得
	bool GetIsSkillActive() const;

private:
	//===スキルの選択===
	//次のスキルを選択
	void SelectNextSkill();
	//前のスキルを選択
	void SelectPrevSkill();

	//スキル1(雷閃)の開始
	void StartThunderFlash();
	//スキル1(雷閃)の更新処理
	void _updateThunderFlash(float DeltaTime);

public:
	//スキル発動可能状態の設定
	void SetCanSkillActive(bool CanSkillActive);

	//現在選択中のスキルが使用可能かチェック
	bool CheckCurrentSkillUsable() const;

public:
	//スキル使用可能状態が変化した際のイベント
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkillUsableChanged m_OnSkillUsableChanged;

protected:
	//プレイヤーへの参照
	UPROPERTY()
	APlayerCharacter* m_Player = nullptr;

	//移動コンポーネントへの参照
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_MovementComponent* m_MovementComponent = nullptr;

	//攻撃コンポーネントへの参照
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_AttackComponent* m_AttackComponent = nullptr;

	//電力ゲージコンポーネントへの参照
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_ElectroGaugeComponent* m_ElectroGaugeComponent = nullptr;

	//スキルヒット時に表示するUIクラス
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|UI")
	TSubclassOf<UUserWidget> m_SkillHitUIClass = nullptr;

private:
	//現在使用中のアビリティスキル
	TSubclassOf<UGameplayAbility> m_CurrentAbilitySkill = nullptr;

	//スキル発動可能フラグ
	bool m_CanSkillActive = true;

	//雷閃(ThunderFlash)の消費コスト
	float m_ThunderFlashCost = 70.0f;
	//スキル1の移動距離
	float m_Skill01Distance = 600.0f;

	//現在選択されているスキル
	EPlayerSkill m_SelectedSkill = EPlayerSkill::ThunderFlash;
	//スキル選択のロック状態
	bool m_SelectLocked = false;

	//スキルが実行中かどうかのフラグ
	bool m_IsSkillActive = false;

	//霹靂一閃の移動開始位置
	FVector m_StartLocation = FVector::ZeroVector;
	//霹靂一閃の移動目標位置
	FVector m_TargetLocation = FVector::ZeroVector;
	//遅延時間の経過量
	float m_DelayElapsed = 0.0f;
	//消えるまでの待機時間
	float m_DelayTime = 0.4f;
	//テレポートが完了したかどうかのフラグ
	bool m_Teleported = false;

	//前フレームのスキル使用可能状態を保存
	bool m_LastUsableState = false;

	//現在表示中のヒットUIインスタンス
	UPROPERTY()
	UUserWidget* m_ActiveHitUIInstance = nullptr;

	//ヒットUIを表示済みかどうかのフラグ
	bool m_HasShownHitUI = false;
};
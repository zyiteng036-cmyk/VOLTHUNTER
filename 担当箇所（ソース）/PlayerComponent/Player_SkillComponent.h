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


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HIGHSPEEDACTIONGAME_API UPlayer_SkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayer_SkillComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//===入力===
	void Input_SkillSelect(const FInputActionValue& Value);
	void Input_SkillReleased();
	void Input_SkillActive();

	//スキルの終了
	void EndSkill();

	bool CanUseThunderFlash() const;

	bool GetIsSkillActive()const;
private:
	//===スキルの選択===
	void SelectNextSkill();
	void SelectPrevSkill();

	//スキル1
	void StartThunderFlash();
	void _updateThunderFlash(float DeltaTime);

public:
	void SetCanSkillActive(bool _CanSkillActive);

	//現在選択中のスキルが使用可能かチェック
	bool CheckCurrentSkillUsable() const;

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkillUsableChanged OnSkillUsableChanged;
protected:
	UPROPERTY()
	APlayerCharacter* m_Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_MovementComponent* m_MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_AttackComponent* m_AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component")
	UPlayer_ElectroGaugeComponent* m_ElectroGaugeComponent;

	//UPROPERTY()
	//UPostProcess* m_PostProcessActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|UI")
	TSubclassOf<UUserWidget> SkillHitUIClass;
private:

	TSubclassOf<UGameplayAbility> m_CurrentAbilitySkill;

	//スキル発動可能か
	bool m_CanSkillActive;

	float m_ThunderFlashCost;
	float m_Skill01Distance;

	// スキル選択
	EPlayerSkill m_SelectedSkill;
	bool m_SelectLocked;

	// スキル実行状態
	bool m_IsSkillActive;

	// 霹靂一閃 移動用
	FVector m_StartLocation;
	FVector m_TargetLocation;
	float m_DelayElapsed;
	// 消えるまでの時間
	float m_DelayTime;     
	bool  m_Teleported;

	//前フレームの状態を保存する
	bool m_bLastUsableState;

	UPROPERTY()
	UUserWidget* m_ActiveHitUIInstance;

	bool m_bHasShownHitUI;
};

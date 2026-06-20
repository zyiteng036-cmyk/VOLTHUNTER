// Fill out your copyright notice in the Description page of Project Settings.


#include "ComboActiveNotifyState.h"
#include "../../PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "../../PlayerComponent/Player_AttackComponent.h"
#include "../../PlayerComponent/Player_SkillComponent.h"


void UComboActiveNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    //コンボが発動したかどうかのフラグをリセット
    m_Activated = false;
    //この受付期間中に一度でも入力があったかどうかのフラグをリセット
    m_InputEverRequested = false;
}

void UComboActiveNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	//一度発動したら、この区間ではもう何もしない
	if (m_Activated || !MeshComp) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return;

	UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>();
	if (!AttackComp) return;

	//コンボ受付が許可されており、かつ先行入力が予約されているかチェック
	if (AttackComp->GetCanBufferAttack() && AttackComp->GetNextAttackRequested())
	{
		m_InputEverRequested = true;

		//予約されていた次の攻撃アビリティ（GAS）を発動
		UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
		bool bSuccess = ASC->TryActivateAbilityByClass(PlayerCharacter->GetBufferedNextAbility());

		if (bSuccess)
		{
			//発動に成功したら、予約状態をクリアして次へ繋ぐ準備をする
			AttackComp->SetNextAttackRequested(false);
			PlayerCharacter->SetBufferedNextAbility(nullptr);
			m_Activated = true;

		}
	}
}

void UComboActiveNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return;

	if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
	{
		//攻撃状態の解除
		AttackComp->AirAttackEnd();
		AttackComp->SetIsAttack(false);

		//攻撃が完全に終了したため、スキルの発動を再度許可する
		if (UPlayer_SkillComponent* SkillComp = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>())
		{
			SkillComp->SetCanSkillActive(true);
		}
	}

	//この受付期間中に一度も先行入力（コンボ継続）が行われなかった場合
	if (!m_InputEverRequested)
	{
		//強化攻撃状態などをリセットし、完全にニュートラル状態に戻す
		PlayerCharacter->SetIsEnhancedAttack(false);
		m_InputEverRequested = false;
	}

	//念のためのフラグリセット
	m_Activated = false;
}

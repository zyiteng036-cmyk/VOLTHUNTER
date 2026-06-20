// Fill out your copyright notice in the Description page of Project Settings.


#include "ComboNotifyState.h"
#include "../../PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "../../PlayerComponent/Player_AttackComponent.h"
#include"../../PlayerComponent/Player_EvasiveComponent.h"
#include"../../PlayerComponent/Player_SkillComponent.h"



//受付開始
void UComboNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    //早期リターンでネストを浅くする
    if (!MeshComp) return;

    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
    if (!PlayerCharacter) return;

    UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>();
    if (!AttackComp) return;

    //攻撃入力の先行予約を許可
    AttackComp->SetCanBufferAttack(true);
    AttackComp->SetIsAttack(true);

    //攻撃中はスキル発動を制限する
    if (UPlayer_SkillComponent* SkillComp = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>())
    {
        SkillComp->SetCanSkillActive(false);
    }
}

//終了
void UComboNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter) return;

	//入力予約の受付を終了
	if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
	{
		AttackComp->SetCanBufferAttack(false);
		AttackComp->SetIsAttack(false);
	}

	//攻撃受付が終わったので回避を再許可
	if (UPlayer_EvasiveComponent* EvasiveComp = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>())
	{
		EvasiveComp->SetCanEvasive(true);
	}
}


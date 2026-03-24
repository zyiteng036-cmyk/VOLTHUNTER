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
    if (!MeshComp) return;
    //所有者チェック
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        UE_LOG(LogTemp, Warning, TEXT("ReadyAttackState Begin"));
        //攻撃入力を予約可
        if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
        {
            AttackComp->SetCanBufferAttack(true);
            AttackComp->SetIsAttack(true);
            if (UPlayer_SkillComponent* SkillComp = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>())
            {
                SkillComp->SetCanSkillActive(false);
            }
        }
    }
}

//終了
void UComboNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        UE_LOG(LogTemp, Warning, TEXT("ReadyAttackState End"));
        //入力予約の受付を終了
        if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
        {
            AttackComp->SetCanBufferAttack(false);
            AttackComp->SetIsAttack(false);
        }

        if (UPlayer_EvasiveComponent* EvasiveComp = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>())
        {
            EvasiveComp->SetCanEvasive(true);
        }

    }
}


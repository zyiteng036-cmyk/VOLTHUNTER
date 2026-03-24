// Fill out your copyright notice in the Description page of Project Settings.

//空中攻撃

#include "ConboNotify.h"
#include "../../PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "../../PlayerComponent/Player_AttackComponent.h"

void UConboNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;
    //所有者チェック
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        UE_LOG(LogTemp, Warning, TEXT("BranchAttackNotify fired"));
        if (!PlayerCharacter->GetAbilitySystemComponent()) return;
        if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
        {
            if (AttackComp->GetNextAttackRequested() && AttackComp->GetCanBufferAttack())
            {
                bool bActivated = PlayerCharacter->GetAbilitySystemComponent()->TryActivateAbilityByClass(PlayerCharacter->m_BufferedNextAbility);
                if (bActivated)
                {
                    AttackComp->SetNextAttackRequested(false);
                    PlayerCharacter->m_BufferedNextAbility = nullptr;
                    UE_LOG(LogTemp, Warning, TEXT("Buffered ability activated"));
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Buffered ability failed to activate"));
                }
            }
            else
            {
                AttackComp->AirAttackEnd();
                AttackComp->SetIsAttack(false);
            }
        }
    }
}
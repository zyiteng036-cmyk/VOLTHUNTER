// Fill out your copyright notice in the Description page of Project Settings.


#include "ComboActiveNotifyState.h"
#include "../../PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "../../PlayerComponent/Player_AttackComponent.h"
#include "../../PlayerComponent/Player_SkillComponent.h"


void UComboActiveNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	m_Activated = false;
    m_InputEverRequested = false;
}

void UComboActiveNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
    if (m_Activated)
        return; // 一度発動したら無視

    if (!MeshComp) return;
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
        {
            if (AttackComp->GetCanBufferAttack() && AttackComp->GetNextAttackRequested())
            {
                m_InputEverRequested = true;
                bool bSuccess = PlayerCharacter->GetAbilitySystemComponent()->TryActivateAbilityByClass(PlayerCharacter->m_BufferedNextAbility);
                if (bSuccess)
                {
                    AttackComp->SetNextAttackRequested(false);
                    PlayerCharacter->m_BufferedNextAbility = nullptr;
                    m_Activated = true;
                    UE_LOG(LogTemp, Warning, TEXT("発動NotifyState：Tickでコンボ発動"));
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("発動NotifyState：発動失敗"));
                }
            }
        }
    }
}

void UComboActiveNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        if (UPlayer_AttackComponent* AttackComp = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>())
        {
            AttackComp->AirAttackEnd();
            AttackComp->SetIsAttack(false);
            if (UPlayer_SkillComponent* SkillComp = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>())
            {
                SkillComp->SetCanSkillActive(true);
            }
            UE_LOG(LogTemp, Warning, TEXT("発動NotifyState：NotifyEndで攻撃終了処理"));
        }

        if (!m_InputEverRequested) {
        
            PlayerCharacter->SetIsEnhancedAttack(false);
            m_InputEverRequested = false;
        }
    }
    m_Activated = false;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_ActivateAttackAbility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"

UBTT_ActivateAttackAbility::UBTT_ActivateAttackAbility() {
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_ActivateAttackAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	CachedASC = Pawn->FindComponentByClass<UAbilitySystemComponent>();
	if (!CachedASC || !AttackAbility)
	{
		return EBTNodeResult::Failed;
	}

	// 既にAbilityが付与済みかチェック
	FGameplayAbilitySpec* FoundSpec = CachedASC->FindAbilitySpecFromClass(AttackAbility);
	if (FoundSpec)
	{
		ActivatedAbilityHandle = FoundSpec->Handle;
	}
	else
	{
		FGameplayAbilitySpec NewSpec(AttackAbility, 1, INDEX_NONE);
		ActivatedAbilityHandle = CachedASC->GiveAbility(NewSpec);
	}

	// Delegate重複解除
	if (AbilityEndedDelegateHandle.IsValid())
	{
		CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		AbilityEndedDelegateHandle.Reset();
	}

	// Delegate登録
	AbilityEndedDelegateHandle = CachedASC->OnAbilityEnded.AddUObject(this, &UBTT_ActivateAttackAbility::OnAbilityEnded);

	// OwnerCompキャッシュ
	CachedOwnerComp = &OwnerComp;

	const bool bActivated = CachedASC->TryActivateAbility(ActivatedAbilityHandle);
	if (!bActivated)
	{
		// Delegate解除
		CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		AbilityEndedDelegateHandle.Reset();
		CachedOwnerComp = nullptr;
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;

}

void UBTT_ActivateAttackAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) {
	// キャッシュクリア
	CachedOwnerComp = nullptr;
	CachedASC = nullptr;
}

void UBTT_ActivateAttackAbility::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData) {
	if (AbilityEndedData.AbilitySpecHandle == ActivatedAbilityHandle && CachedASC && CachedOwnerComp)
	{
		// Delegate解除
		CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		AbilityEndedDelegateHandle.Reset();

		// タスク終了通知
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}
}
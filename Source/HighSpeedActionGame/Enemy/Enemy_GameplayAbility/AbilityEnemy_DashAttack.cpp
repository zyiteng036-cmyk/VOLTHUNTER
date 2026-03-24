// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityEnemy_DashAttack.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_WaitCancel.h"
#include "GameFramework/Character.h"

UAbilityEnemy_DashAttack::UAbilityEnemy_DashAttack()
	:AttackMontage(nullptr)
	, DashVector(FVector::ZeroVector)
	, DashDuration(2.f)
	, DashStrength(1200.f)
{

}


void UAbilityEnemy_DashAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {
	// AbilityのAvatarとなるCharacter取得
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());

	// DashMontageが設定されていない場合は中断
	if (!Character || !AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	DashVector = Character->GetActorForwardVector();

	// 壁判定（ダッシュ開始時）
	const float WallCheckDistance = 300.f;
	FVector TraceStart = Character->GetActorLocation();
	FVector TraceEnd = TraceStart + DashVector.GetSafeNormal() * WallCheckDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	bool bHitWall = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	if (bHitWall)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);

	MontageTask->OnCompleted.AddDynamic(this, &UAbilityEnemy_DashAttack::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UAbilityEnemy_DashAttack::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UAbilityEnemy_DashAttack::OnMontageEnded);
	MontageTask->ReadyForActivation();

	//Root Motionを使ってキャラクター移動を制御
	RootMotionTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this,                  // 設定するアビリティ
		NAME_None,             // タスクの名前
		DashVector.GetSafeNormal(),  // 方向
		DashStrength,          // 速度
		DashDuration,          // 時間
		false,                 // 他RootMotionに対し加算的に適用するか
		nullptr,               // 力の大きさを時間で変化させるか
		ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity,  // Root Motion終了時の速度制御モード
		FVector::ZeroVector,   // 終了時のベロシティ
		0.f,
		false                          // 重力を有効にするか
	);
	RootMotionTask->ReadyForActivation();

	CancelTask = UAbilityTask_WaitCancel::WaitCancel(this);
	CancelTask->OnCancel.AddDynamic(this, &UAbilityEnemy_DashAttack::OnAbilityCancelled);
	CancelTask->ReadyForActivation();

	// 壁判定を定期チェックするタイマーを設定
	GetWorld()->GetTimerManager().SetTimer(
		DashWallCheckTimerHandle,
		this,
		&UAbilityEnemy_DashAttack::CheckWallDuringDash,
		0.05f,
		true
	);
}
void UAbilityEnemy_DashAttack::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) {
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	OnAbilityCancelled();
}

void UAbilityEnemy_DashAttack::OnMontageEnded() {
	// モンタージュが完了・割り込み・キャンセルされたらAbilityを終了
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	GetWorld()->GetTimerManager().ClearTimer(DashWallCheckTimerHandle);
}

void UAbilityEnemy_DashAttack::OnAbilityCancelled() {
	GetWorld()->GetTimerManager().ClearTimer(DashWallCheckTimerHandle);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAbilityEnemy_DashAttack::CheckWallDuringDash()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;

	const float WallCheckDistance = 300.f;
	FVector TraceStart = Character->GetActorLocation();
	FVector TraceEnd = TraceStart + DashVector.GetSafeNormal() * WallCheckDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	bool bHitWall = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHitWall)
	{
		GetWorld()->GetTimerManager().ClearTimer(DashWallCheckTimerHandle);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}


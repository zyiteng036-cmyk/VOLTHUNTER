// Fill out your copyright notice in the Description page of Project Settings.


#include "../Enemy/EnemyBase.h"
#include "../PlayerCharacter.h"
#include "../GameUtility/GameUtility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Kismet//KismetMathLibrary.h"
#include "../HitStopComponent/HitStopComponent.h"
#include "../HitJudgmentComponent/HitJudgmentComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/MeshComponent.h"
#include "EnemyManager/EnemyManager.h"
#include "../Event/GameplayAreaEventManager/GameplayAreaEventManager.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "../Enemy/EnemyNavigationManager/EnemyNavigationManager.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "../SequenceWorldSubsystem/SequenceWorldSubsystem.h"

// Sets default values
AEnemyBase::AEnemyBase()
	: m_EnemyParam()
	, m_EnemyTypeFlags()
	, m_HitStopComponent(nullptr)
	, m_DamageMontage(nullptr)
	, m_HitJudgmentComponent(nullptr)
	, m_MaterialInstanceDynamics()
	, m_PlayerChara(nullptr)
	, m_SplinePatrolActor(nullptr)
	, m_Opacity(1.f)
	, m_DistanceToTarget(70.f)
	, m_WarpOffsetDistance(150.f)
	, m_MoveTargetLocation(FVector::ZeroVector)
	, m_IsActive(true)
	, m_IsDeath(false)
	, m_IsDying(false)
	, m_IsTakeDamage(true)
	, m_IsTakingDamage(false)
	, m_CanSeeTarget(false)
	, m_DamageTime(1.f)
	, m_DamageTimeTimer(0.f)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// AddUniqueでタグが重複しないようになる
	Tags.AddUnique(FName(TEXT("Enemy")));

	m_HitStopComponent = CreateDefaultSubobject<UHitStopComponent>(TEXT("HitStopComponent"));

	m_HitJudgmentComponent = CreateDefaultSubobject<UHitJudgmentComponent>(TEXT("HitJudgmentComponent"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	m_PlayerChara = Cast<APlayerCharacter>(CGameUtility::GetActorFromTag(this->GetWorld(), "Player"));


	USkeletalMeshComponent* SkelComp = FindComponentByClass<USkeletalMeshComponent>();
	if (!SkelComp) return;

	//マテリアルの数取得
	int32 MaterialNum = SkelComp->GetNumMaterials();

	m_MaterialInstanceDynamics.Empty();

	for (int32 i = 0; i < MaterialNum; ++i) {
		UMaterialInterface* BaseMat = SkelComp->GetMaterial(i);
		if (!BaseMat) continue;
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, this);

		if (MID)
		{
			SkelComp->SetMaterial(i, MID);
			m_MaterialInstanceDynamics.Add(MID);
		}
	}

	//if (m_AbilitySystemComponent)
	//{
	//	if (DamageAbilityClass) {
	//		m_AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DamageAbilityClass, 1, INDEX_NONE, this));
	//	}

	//}
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	if (!m_IsActive)return;

	Super::Tick(DeltaTime);

	//死亡時の消える処理
	if (m_IsDeath) {
		m_Opacity -= DeltaTime;

		SetOpacity(m_Opacity);

		if (m_Opacity <= 0.f) {

			SetActive(false);
		}
	}

	if (m_IsTakingDamage && !m_IsDying) {
		m_DamageTimeTimer += DeltaTime;

		if (m_DamageTimeTimer > m_DamageTime) {
			m_DamageTimeTimer = 0.f;
			m_IsTakingDamage = false;
			OnDamageEnd();
		}
	}

	{
		FVector Velocity = GetVelocity();
		float Speed = UKismetMathLibrary::VSize(Velocity);
	}

}

void AEnemyBase::SetActive(const bool _isActive) {
	m_IsActive = _isActive;
	SetActorHiddenInGame(!_isActive);       // 描画の有効無効//falseで隠す
	SetActorEnableCollision(_isActive);     // 衝突判定の有効無効
	SetActorTickEnabled(_isActive);         // Tickの有効無効
	SetEnemyActiveMoveComponent(_isActive);//MoveCompを無効にしておく
	SetAIControllerIsActive(_isActive);
	if (_isActive) {
		ResetParam();
	}
}
void AEnemyBase::SetAIControllerActive(const bool _isActive) {
	AAIController* AIController = Cast<AAIController>(UAIBlueprintHelperLibrary::GetAIController(this));
	if (AIController)
	{
		if (_isActive)
		{
			if (AIController->GetPawn() != this)
			{
				AIController->Possess(Cast<APawn>(this));
			}
			AIController->SetActorTickEnabled(true);
		}
		else
		{
			AIController->UnPossess();
			AIController->SetActorTickEnabled(false);
		}
	}
}
void AEnemyBase::SetAIControllerIsActive(const bool _isActive) {
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();

		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool("IsActive", _isActive); //BBのTakingDamageTrueにする
		}
	}
}
void AEnemyBase::SetEnemyActiveMoveComponent(const bool _isActive) {

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	if (MoveComp)
	{
		MoveComp->SetComponentTickEnabled(_isActive);
		if (_isActive)
		{
			if (!MoveComp->IsRegistered())
			{
				//Actor Component をゲームの更新ループ・物理・レンダリング処理に登録
				MoveComp->RegisterComponent();
			}
		}
		else
		{
			// キャラクターの移動を停止
			MoveComp->StopMovementImmediately();
		}
	}
}

bool AEnemyBase::GetBoolBBIsTakingDamage()const {
	AAIController* AIController = Cast<AAIController>(GetController());

	if (AIController)return false;

	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
	if (!BlackboardComp)return false;

	// 取得したいBool型のブラックボードキー名
	const FName BoolKeyName = TEXT("IsTakingDamage");

	bool bValue = BlackboardComp->GetValueAsBool(BoolKeyName);

	return bValue;
}

void AEnemyBase::SetEnemyParam(FEnemyDataBase _enemyData) {
	m_EnemyParam.m_Hp = _enemyData.HP;
	m_EnemyParam.m_MaxHp = _enemyData.HP;
	m_EnemyParam.AttackPower = _enemyData.AttackPower;
	m_EnemyParam.m_WalkSpeed = _enemyData.WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = _enemyData.WalkSpeed;
	m_EnemyParam.m_DashSpeed = _enemyData.DashSpeed;
	m_EnemyParam.m_RotationSpeed = _enemyData.RotationSpeed;
}

void AEnemyBase::TakeDamage(const FDamageInfo& _damageInfo) {
	if (!m_HitStopComponent || !m_IsTakeDamage)return;

	UseDamageInformation(_damageInfo);

	if (m_EnemyParam.m_Hp > 0)
	{
		if (!m_DamageMontage)return;

		if (_damageInfo.IsEnhancedAttack || (!m_EnemyTypeFlags.bIsEventEnemy && !m_CanSeeTarget)) {
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(m_DamageMontage);
			}
			m_IsTakingDamage = true;
			m_DamageTimeTimer = 0.f;
			HandleDamageTakenAI();
		}
	}
	else {
		// アニメモンタージュの停止
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			{
				AnimInstance->Montage_Stop(0.f, nullptr);
			}
		}

		OnDying();//瀕死処理呼び出す
		HandleDamageTakenAI();
	}

	FacePlayerDirection();

	if (!m_EnemyTypeFlags.bIsEventEnemy) {
		OnSeeTarget();
	}
}

void AEnemyBase::HandleDamageTakenAI() {
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		//UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIController->GetComponentByClass(UBehaviorTreeComponent::StaticClass()));

		//if (BTComp && !GetBoolBBIsTakingDamage())
		//{
		//	BTComp->RestartTree();
		//}
		SetBBIsTakingDamage(true);
	}
}

void AEnemyBase::SetBBMoveToLocation(const FVector _moveToLocation, FName _keyName = "MoveLocation") {
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsVector(_keyName, _moveToLocation);
		}
	}
}

void AEnemyBase::Speed​​ChangeDash() {
	GetCharacterMovement()->MaxWalkSpeed = m_EnemyParam.m_DashSpeed;
}
void AEnemyBase::SpeedChangeWalk() {
	GetCharacterMovement()->MaxWalkSpeed = m_EnemyParam.m_WalkSpeed;
}



void AEnemyBase::OnDamageEnd() {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("OnDamageEnd"));

	SetBBIsTakingDamage(false);
}

void AEnemyBase::OnDeath() {
	UEnemyManager* EnemyMng = GetWorld()->GetSubsystem<UEnemyManager>();
	if (EnemyMng)
	{
		EnemyMng->RemoveFromActiveEnemies(this);
	}

	m_IsDeath = true;
}

void AEnemyBase::OnDying() {
	m_IsTakeDamage = false;
	m_IsDying = true;

	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		AIController->StopMovement();//移動止める
	}

	if (m_EnemyTypeFlags.bIsEventEnemy) {
		UWorld* World = GetWorld();
		if (World)
		{
			{
				UGameplayAreaEventManager* Subsystem = World->GetSubsystem<UGameplayAreaEventManager>();
				if (m_EnemyTypeFlags.bIsBoss) {
					Subsystem->EventBossEnemyDeath();
					return;
				}
				if (Subsystem)
				{
					Subsystem->EventEnemyDeath();
				}
			}

			{
				UEnemyNavigationManager* Subsystem = World->GetSubsystem<UEnemyNavigationManager>();
				if (Subsystem) {
					Subsystem->RemoveChasingEnemy(this);
				}
			}

		}
	}
}
void AEnemyBase::BlowEnemy(const FVector& _blowVector, const float _blowScale) {
	FVector BlowVector = _blowVector.GetSafeNormal() * _blowScale;

	//キャラクター吹っ飛ばす処理//第一引数吹っ飛ばしベクトル//第二引数水平方向に速度を上書きするが//第三引数垂直方向に速度上書きするか
	LaunchCharacter(BlowVector, true, true);
}

void AEnemyBase::SetOpacity(const float alpha) {
	for (int32 i = 0; i < m_MaterialInstanceDynamics.Num(); ++i) {
		if (!m_MaterialInstanceDynamics[i])continue;
		m_MaterialInstanceDynamics[i]->SetScalarParameterValue(FName("Opacity"), alpha);
	}



}
void AEnemyBase::ResetParam() {
	m_EnemyParam.m_Hp = m_EnemyParam.m_MaxHp;
	m_Opacity = 1.f;
	SetOpacity(m_Opacity);
	m_IsTakeDamage = true;
	m_IsDeath = false;
	m_IsDying = false;
	SetBBIsTakingDamage(false);
	SetIsEventEnemy(false);
}

void AEnemyBase::UseDamageInformation(const FDamageInfo& _damageInfo) {
	//ヒットストップ
	m_HitStopComponent->StartHitStop(_damageInfo.HitStopTime);

	m_EnemyParam.m_Hp -= CalculateEnemyDamage(_damageInfo);

	if (m_EnemyParam.m_Hp > 0) {
		//吹っ飛ばし
		BlowEnemy(_damageInfo.KnockbackDirection, _damageInfo.KnockbackScale);
	}
}


void AEnemyBase::FacePlayerDirection() {
	//プレイヤーの方向を向く処理
	FVector ThisEnemyPos = GetActorLocation();
	FVector PlayerPos = m_PlayerChara->GetActorLocation();
	FRotator ThisEnemyRot = GetActorRotation();

	// プレイヤー方向の回転を計算
	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(ThisEnemyPos, PlayerPos);

	// Yawのみ即時適用（Pitch, Rollは維持）
	SetActorRotation(FRotator(ThisEnemyRot.Pitch, TargetRot.Yaw, ThisEnemyRot.Roll));
}

void AEnemyBase::SetBBIsTakingDamage(const bool _isTakingDamage) {
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool("IsTakingDamage", _isTakingDamage); //BBのTakingDamageTrueにする
		}
	}

}

void AEnemyBase::ResetEnemyTypeFlags() {
	m_EnemyTypeFlags.bIsBoss = false;
	m_EnemyTypeFlags.bIsEventEnemy = false;
}

void AEnemyBase::SetIsEventEnemy(const bool _isEventEnemy) {
	m_EnemyTypeFlags.bIsEventEnemy = _isEventEnemy;

	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool("IsEventEnemy", _isEventEnemy); //BBのIsEventEnemyをTrueにする
		}
	}

}

void AEnemyBase::OnEnterEventMode() {
	m_EnemyTypeFlags.bIsEventEnemy = true;
}

float AEnemyBase::CalculateEnemyDamage(const FDamageInfo& _damageInfo) {
	float Damage = _damageInfo.Damage;

	if (!_damageInfo.IsEnhancedAttack) {
		Damage *= 0.5f;
	}

	return Damage;
}

uint8 AEnemyBase::GetBBCurrentState(const FName _keyName) {
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)return -1;

	UBlackboardComponent* BBComp = AIController->GetBlackboardComponent();
	if (!BBComp)return-1;

	uint8 RawValue = BBComp->GetValueAsEnum(_keyName);

	return RawValue;
}

void AEnemyBase::OnSeeTarget() {
	UEnemyNavigationManager* Subsystem = GetWorld()->GetSubsystem<UEnemyNavigationManager>();
	if (!Subsystem)return;

	Subsystem->AddChasingEnemy(this);

	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)return;
	m_CanSeeTarget = true;
	if (AIController)
	{
		AIController->StopMovement();
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool("IsPlayerInSight", true); //BBのIsPlayerInSightTrueにする
		}
	}
}

void AEnemyBase::PauseEnemy(bool _isPouse) {
	//移動停止・再開
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		if (_isPouse)
		{
			MoveComp->StopMovementImmediately();
			MoveComp->SetMovementMode(MOVE_None);
			MoveComp->SetComponentTickEnabled(false);
		}
		else
		{
			MoveComp->SetMovementMode(MOVE_Walking); // 必要に応じて元のモードへ
			MoveComp->SetComponentTickEnabled(true);
		}
	}

	//AIコントローラーの停止・再開
	SetAIControllerIsActive(_isPouse);

	//ビヘイビアツリー停止
	if (_isPouse)
	{
		AAIController* AIController = Cast<AAIController>(GetController());
		if (AIController)
		{
			UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(AIController->GetBrainComponent());
			if (BTComponent)
			{
				BTComponent->StopLogic(TEXT("Paused by enemy base"));//ビヘイビアツリー停止
			}
		}
	}
	//アニメーションモンタージュ一時停止・再開
	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh)
	{
		UAnimInstance* AnimInstance = SkelMesh->GetAnimInstance();
		if (AnimInstance)
		{
			if (_isPouse)
			{
				AnimInstance->Montage_Pause(nullptr);   // 全モンタージュ一時停止
			}
			else
			{
				AnimInstance->Montage_Resume(nullptr);  // モンタージュ再開
			}
		}

		//Animation Blueprint通常アニメーションも一時停止・再開
		SkelMesh->GlobalAnimRateScale = _isPouse ? 0.0f : 1.0f;
	}

	//敵有効フラグ制御（例: ダメージ受付停止）
	m_IsTakeDamage = !_isPouse;

	//Actor Tick制御
	SetActorTickEnabled(!_isPouse);
}

void AEnemyBase::HandleBossCase() {
	if (USequenceWorldSubsystem* Manager = GetWorld()->GetSubsystem<USequenceWorldSubsystem>())
	{
		Manager->OnBossSequenceFinished.AddDynamic(this, &AEnemyBase::OnBossSequenceFinished);
	}

	SetAIControllerIsActive(false);
}

void AEnemyBase::OnBossSequenceFinished() {
	SetAIControllerIsActive(true);
}
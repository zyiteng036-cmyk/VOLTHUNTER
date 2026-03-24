// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_EvasiveComponent.h"
#include "Player_MovementComponent.h"
#include "Player_AttackComponent.h"
#include "Player_CameraComponent.h"
#include "Camera/CameraComponent.h"
#include "Player_ElectroGaugeComponent.h"
#include "Player_SkillComponent.h"
#include "../HitStopComponent/HitStopComponent.h"
#include "../PlayerCharacter.h"
#include "../PlayerNotifySubSystem/PlayerNotifySubsystem.h"
#include "../CollisionChannelName.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "../AttackCollisionDetection/AttackCollisionDetection.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

// Sets default values for this component's properties
UPlayer_EvasiveComponent::UPlayer_EvasiveComponent()
	:m_Player(nullptr)
	, m_MovementComponent(nullptr)
	, m_CameraComponent(nullptr)
	, m_AttackComponent(nullptr)
	, m_ElectroComponent(nullptr)
	, m_SkillComponent(nullptr)
	, JustEvasiveCollision(nullptr)
	, m_IsEvasive(false)
	, m_IsJustEvasive(false)
	, m_EvasiveTime(0.f)
	, m_CurrentEvasiveStock(PlayerParam.EvasiveStock)
	, m_CurrentEvasiveRecoveryTime(0.f)
	, m_EvasiveDirection(FVector::ZeroVector)
	, m_CurrentJustEvasiveTime(0.f)
	, m_JustEvasiveLimitTime(1.5f)
	, m_CanEvasive(true)
	, m_CurrentSlowMotionWatchTime(0.f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayer_EvasiveComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player)		return;

	m_MovementComponent = m_Player->FindComponentByClass<UPlayer_MovementComponent>();
	if (!m_MovementComponent)return;

	m_CameraComponent = m_Player->FindComponentByClass<UPlayer_CameraComponent>();
	if (!m_CameraComponent)return;

	m_AttackComponent = m_Player->FindComponentByClass<UPlayer_AttackComponent>();
	if (!m_AttackComponent)return;

	m_ElectroComponent = m_Player->FindComponentByClass<UPlayer_ElectroGaugeComponent>();
	if (!m_ElectroComponent)return;

	m_SkillComponent = m_Player->FindComponentByClass<UPlayer_SkillComponent>();
	if (!m_SkillComponent)return;

	// ...
}

// Called every frame
void UPlayer_EvasiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	//状態を更新
	_updateEvasive(DeltaTime);

	float CurrentTimeDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());

	if (CurrentTimeDilation < 0.95f && CurrentTimeDilation > 0.05f)
	{
		//スロー中は DeltaTime も小さくなっているため、実時間に変換して加算する
		//0除算防止のため、CurrentTimeDilation が極端に小さい場合は計算しない
		if (CurrentTimeDilation > KINDA_SMALL_NUMBER)
		{
			m_CurrentSlowMotionWatchTime += (DeltaTime / CurrentTimeDilation);
		}
		else
		{
			//時間がほぼ停止している場合は DeltaTime をそのまま使うか、固定値を足す
			m_CurrentSlowMotionWatchTime += DeltaTime;
		}

		//制限時間を超えたら強制リセット
		if (m_CurrentSlowMotionWatchTime >= SAFETY_SLOW_MOTION_LIMIT)
		{
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
			m_CurrentSlowMotionWatchTime = 0.f;

			UE_LOG(LogTemp, Warning, TEXT("TimeDilation was stuck! Force reset to 1.0f."));
		}
	}
	else
	{
		//スロー中ではないならタイマーをリセット
		m_CurrentSlowMotionWatchTime = 0.f;
	}
}


//回避スウェイ
void UPlayer_EvasiveComponent::_updateEvasive(float DeltaTime)
{

	if (m_IsJustEvasive)
	{
		m_CurrentJustEvasiveTime += DeltaTime;

		if (m_CurrentJustEvasiveTime >= m_JustEvasiveLimitTime)
		{
			m_IsJustEvasive = false;
			m_CurrentJustEvasiveTime = 0.f;

			m_Player->SetInvincible(false);

			HideJustEvasiveUI();
			if (m_SkillComponent)
			{
				m_SkillComponent->SetCanSkillActive(true);
			}

		}
	}

}

//回避スウェイ入力
void UPlayer_EvasiveComponent::Input_Evasive(const FInputActionValue& Value)
{
	if (m_IsEvasive)return;


	//空中
	if (!m_Player->GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	//回避可能中ではなければ回避不可
	if (!GetCanEvasive())
	{
		return;
	}

	m_Player->OnActionCommitted(EPlayerActionCommit::Evasive);


	m_AttackComponent->SetIsAttack(false);
	m_AttackComponent->SetCanBufferAttack(false);
	m_AttackComponent->SetNextAttackRequested(false);

	//ダッシュ中であれば終了させる
	if (m_MovementComponent->GetIsDash())
	{
		m_MovementComponent->EndDash();
	}
	m_MovementComponent->SetCanMovement(false);
	////使用したらストック消費
	//m_CurrentEvasiveStock--;

	m_IsEvasive = true;
	m_EvasiveTime = 0.f;

	//スキル使用不可
	m_SkillComponent->SetCanSkillActive(false);

	//回避方向
	FRotator EvasiveRotation;
	//ロックオン中
	if (m_CameraComponent->GetIsTargetLockedOn() && m_CameraComponent->GetLockOnCamera())
	{
		//ロックオンカメラの水平方向
		FRotator CameraRotation = m_CameraComponent->GetLockOnCamera()->GetComponentRotation();
		EvasiveRotation = FRotator(0.f, CameraRotation.Yaw, 0.f);
	}
	//通常時
	else
	{
		EvasiveRotation = m_MovementComponent->GetControlRotationFlat();
	}

	m_Player->DeleteCollision();


	//前方向ベクトルと右方向ベクトルの取得
	FVector Forward = FRotationMatrix(EvasiveRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(EvasiveRotation).GetUnitAxis(EAxis::Y);

	//移動入力があればその方向に
	if (m_MovementComponent->GetCurrentMoveInput().SizeSquared() > 0.1f)
	{
		m_EvasiveDirection = (Forward * m_MovementComponent->GetCurrentMoveInput().X + Right * m_MovementComponent->GetCurrentMoveInput().Y).GetSafeNormal();
	}
	//入力がなければ後ろに
	else
	{
		m_EvasiveDirection = -m_Player->GetActorForwardVector().GetSafeNormal();
	}

	//回避の速度を与える
	//LaunchCharacterで瞬間的な速度を得る
	FVector Launch = m_EvasiveDirection * PlayerParam.EvasiveSpeed;
	Launch.Z += PlayerParam.VerticalSpeed;
	m_Player->LaunchCharacter(Launch, true, true);

	//回避アビリティ
	if (UAbilitySystemComponent* AbilitySystemComp = m_Player->GetAbilitySystemComponent())
	{
		if (m_MovementComponent->GetCurrentMoveInput().SizeSquared() > 0.1f && m_Player->m_AbilityPlayer_Evasive)
		{
			AbilitySystemComp->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_Evasive);
		}
		//入力がなければ後ろに
		else if (m_Player->m_AbilityPlayer_BackEvasive)
		{
			AbilitySystemComp->TryActivateAbilityByClass(m_Player->m_AbilityPlayer_BackEvasive);
		}
	}
}

//回避用コリジョン生成
void UPlayer_EvasiveComponent::CreateJustEvasiveCollision()
{
	if (JustEvasiveCollision)return;

	//回避コリジョン生成
	JustEvasiveCollision = NewObject<UCapsuleComponent>(this);


	// まずプレイヤーの位置に合わせて配置（+高さ調整）
	JustEvasiveCollision->SetWorldLocation(m_Player->GetActorLocation());
	//ワールドに登録
	JustEvasiveCollision->RegisterComponent();
	//半径
	JustEvasiveCollision->InitCapsuleSize(60.f, 94.f);
	//コリジョン判定する
	JustEvasiveCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//ワールド上でどのカテゴリーに属する物体か
	JustEvasiveCollision->SetCollisionObjectType(JustEvasive_Collision);
	//無視する
	JustEvasiveCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	//何がOverlapと反応するか
	JustEvasiveCollision->SetCollisionResponseToChannel(AttackDetection_Collision, ECR_Overlap);
	//表示,可視化
	//JustEvasiveCollision->SetHiddenInGame(false);
	JustEvasiveCollision->SetGenerateOverlapEvents(true);

	//判定を即時反映
	JustEvasiveCollision->UpdateOverlaps();

	JustEvasiveCollision->OnComponentBeginOverlap.AddDynamic(this, &UPlayer_EvasiveComponent::OnJustEvasiveOverlap);


}

//回避コリジョン破棄
void UPlayer_EvasiveComponent::DestroyJustEvasiveCollision()
{
	if (!JustEvasiveCollision)return;

	JustEvasiveCollision->DestroyComponent();
	JustEvasiveCollision = nullptr;
}

//ジャスト回避成功時
void UPlayer_EvasiveComponent::OnJustEvasiveOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (!OtherActor || OtherActor == m_Player) return;
	if (m_IsJustEvasive)return;
	if (m_Player->GetIsDamage())return;

	if (UHitStopComponent* HitStopComp = m_Player->FindComponentByClass<UHitStopComponent>())
	{
		if (HitStopComp->IsHitStopActive())return;
	}

	//攻撃コリジョンと重なったか？
	if (OtherComp && OtherComp->GetCollisionObjectType() == AttackDetection_Collision)
	{
		m_IsJustEvasive = true;
		m_Player->SetInvincible(true);
		m_Player->SetIsEnhancedAttack(true);


		UE_LOG(LogTemp, Warning, TEXT("EvasiveCollisionkasanatta"));

		if (JustEvasiveWidgetClass && m_Player->GetController())
		{
			//既に表示されていたら一旦消す
			if (m_JustEvasiveWidgetInstance)
			{
				m_JustEvasiveWidgetInstance->RemoveFromParent();
				m_JustEvasiveWidgetInstance = nullptr;
			}

			//APlayerControllerを取得
			if (APlayerController* PC = Cast<APlayerController>(m_Player->GetController()))
			{
				//Widget生成
				m_JustEvasiveWidgetInstance = CreateWidget<UUserWidget>(PC, JustEvasiveWidgetClass);
				if (m_JustEvasiveWidgetInstance)
				{
					m_JustEvasiveWidgetInstance->AddToViewport();

				}
			}
		}

		//スロー演出開始時に監視タイマーをリセットしておく
		m_CurrentSlowMotionWatchTime = 0.f;

		//スロー演出
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);

		FTimerHandle SlowMoTimer;
		GetWorld()->GetTimerManager().SetTimer(
			SlowMoTimer,
			[this]()
			{
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
			},
			0.2f, // スローの実時間
			false
		);

		//ジャスト回避成功時電力ゲージ加算
		OnJustEvasiveSuccess();

		if (!IsValid(m_Player->GetJustEvasive_Attacker()))return;
		m_MovementComponent->OnJustEvasive(m_Player->GetJustEvasive_Attacker());
		m_CameraComponent->OnJEnemyDirection(m_Player->GetJustEvasive_Attacker(), true);

		if (m_Player)
		{
			m_Player->BroadcastJustEvasiveSuccess(m_Player->GetJustEvasive_Attacker());
		}
	}

}

void UPlayer_EvasiveComponent::OnJustEvasiveSuccess()
{
	if (!m_Player) return;

	m_ElectroComponent->AddElectroGauge(15.0f);
	m_ElectroComponent->OnJustEvasiveSuccess();
}

void UPlayer_EvasiveComponent::HideJustEvasiveUI()
{
	if (m_JustEvasiveWidgetInstance)
	{
		m_JustEvasiveWidgetInstance->RemoveFromParent();
		m_JustEvasiveWidgetInstance = nullptr;
	}
}

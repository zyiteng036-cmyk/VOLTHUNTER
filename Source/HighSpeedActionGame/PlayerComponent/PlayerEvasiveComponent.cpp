#include "PlayerEvasiveComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "Attack/PlayerAttackComboComponent.h"
#include "PlayerAttackComponent.h"
#include "PlayerElectroGaugeComponent.h"
#include "../HitStopComponent/HitStopComponent.h"
#include "../PlayerNotifySubSystem/PlayerNotifySubsystem.h"
#include "../CollisionChannelName.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"

//初期化
UPlayerEvasiveComponent::UPlayerEvasiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

//開始時
void UPlayerEvasiveComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player) return;

	m_ElectroGaugeComponent = m_Player->FindComponentByClass<UPlayerElectroGaugeComponent>();
	m_HitStopComponent = m_Player->FindComponentByClass<UHitStopComponent>();
}

//毎フレーム更新
void UPlayerEvasiveComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateJustEvasive(DeltaTime);
	UpdateSlowMotionSafety(DeltaTime);
}

//回避開始時
void UPlayerEvasiveComponent::BeginEvasive()
{
	if (!m_Player) return;
	if (m_bIsEvasive) return;
	if (!m_bCanEvasive) return;

	m_bIsEvasive = true;

	
	//回避中はプレイヤー同士/敵とのPawn衝突を一時的に無効化する
	m_Player->DeleteCollision();
}

//回避終了時
void UPlayerEvasiveComponent::EndEvasive()
{
	if (!m_Player) return;

	m_bIsEvasive = false;

	//回避終了時に通常Collisionへ戻す
	m_Player->RevivalCollision();

	//残っているジャスト回避Collisionを破棄
	DestroyJustEvasiveCollision();

}


//被ダメージ前にジャスト回避を試行
bool UPlayerEvasiveComponent::TryHandleJustEvasiveBeforeDamage()
{
	if (!m_Player) return false;
	if (!m_bIsEvasive) return false;
	if (m_bIsJustEvasive) return true;
	if (!m_JustEvasiveCollision) return false;

	if (m_HitStopComponent && m_HitStopComponent->IsHitStopActive())
	{
		return false;
	}

	//回避でプレイヤーが移動しても判定が置き去りにならないようにする
	m_JustEvasiveCollision->SetWorldLocation(m_Player->GetActorLocation());
	m_JustEvasiveCollision->UpdateOverlaps();

	TArray<UPrimitiveComponent*> OverlappingComponents;
	m_JustEvasiveCollision->GetOverlappingComponents(OverlappingComponents);

	for (UPrimitiveComponent* OverlappingComponent : OverlappingComponents)
	{
		if (!OverlappingComponent) continue;

		if (OverlappingComponent->GetCollisionObjectType() != AttackDetection_Collision)
		{
			continue;
		}

		AActor* Attacker =
			const_cast<AActor*>(m_Player->GetJustEvasive_Attacker());

		if (!IsValid(Attacker))
		{
			Attacker = OverlappingComponent->GetOwner();
		}

		HandleJustEvasiveSuccess(Attacker);
		return true;
	}

	return false;
}
//被ダメージ前にジャスト回避を試行
bool UPlayerEvasiveComponent::TryHandleJustEvasiveBeforeDamage(AActor* Attacker)
{
	if (!m_Player)
	{
		return false;
	}

	if (!m_bIsEvasive)
	{
		return false;
	}

	if (m_bIsJustEvasive)
	{
		return true;
	}

	if (!m_bJustEvasiveWindowOpen)
	{
		return false;
	}

	if (m_HitStopComponent && m_HitStopComponent->IsHitStopActive())
	{
		return false;
	}

	if (IsValid(Attacker))
	{

		HandleJustEvasiveSuccess(Attacker);
		return true;
	}

	return TryHandleJustEvasiveBeforeDamage();
}

//回避状態を完全リセット
void UPlayerEvasiveComponent::ResetEvasiveState()
{
	m_bIsEvasive = false;
	m_bIsJustEvasive = false;
	m_bCanEvasive = true;
	m_bJustEvasiveWindowOpen = false;

	m_CurrentJustEvasiveTime = 0.f;
	m_CurrentSlowMotionWatchTime = 0.f;

	DestroyJustEvasiveCollision();
	HideJustEvasiveUI();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(m_SlowMotionTimerHandle);
		UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
	}

	if (m_Player)
	{
		m_Player->RevivalCollision();
		m_Player->SetInvincible(false);
		m_Player->SetIsEnhancedAttack(false);
	}

}
//ジャスト回避状態更新
void UPlayerEvasiveComponent::UpdateJustEvasive(float DeltaTime)
{
	if (!m_bIsJustEvasive) return;

	m_CurrentJustEvasiveTime += DeltaTime;

	if (m_CurrentJustEvasiveTime < m_JustEvasiveLimitTime) return;

	m_bIsJustEvasive = false;
	m_CurrentJustEvasiveTime = 0.f;

	if (m_Player)
	{
		m_Player->SetInvincible(false);
	}

	HideJustEvasiveUI();
}

//スローモーション安全装置更新
void UPlayerEvasiveComponent::UpdateSlowMotionSafety(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World) return;

	const float CurrentTimeDilation = UGameplayStatics::GetGlobalTimeDilation(World);

	if (CurrentTimeDilation < 0.95f && CurrentTimeDilation > 0.05f)
	{
		if (CurrentTimeDilation > KINDA_SMALL_NUMBER)
		{
			m_CurrentSlowMotionWatchTime += DeltaTime / CurrentTimeDilation;
		}
		else
		{
			m_CurrentSlowMotionWatchTime += DeltaTime;
		}

		if (m_CurrentSlowMotionWatchTime >= m_SlowMotionSafetyLimit)
		{
			UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
			m_CurrentSlowMotionWatchTime = 0.f;

		}

		return;
	}

	m_CurrentSlowMotionWatchTime = 0.f;
}

//ジャスト回避判定Collision生成
void UPlayerEvasiveComponent::CreateJustEvasiveCollision()
{
	if (!m_Player) return;

	m_bJustEvasiveWindowOpen = true;

	if (m_JustEvasiveCollision) return;

	m_JustEvasiveCollision = NewObject<UCapsuleComponent>(this);
	if (!m_JustEvasiveCollision)
	{
		m_bJustEvasiveWindowOpen = false;
		return;
	}

	m_JustEvasiveCollision->SetWorldLocation(m_Player->GetActorLocation());
	m_JustEvasiveCollision->RegisterComponent();

	m_JustEvasiveCollision->InitCapsuleSize(
		m_JustEvasiveCollisionRadius,
		m_JustEvasiveCollisionHalfHeight
	);

	m_JustEvasiveCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	m_JustEvasiveCollision->SetCollisionObjectType(JustEvasive_Collision);
	m_JustEvasiveCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	m_JustEvasiveCollision->SetCollisionResponseToChannel(AttackDetection_Collision, ECR_Overlap);
	m_JustEvasiveCollision->SetGenerateOverlapEvents(true);
	m_JustEvasiveCollision->SetHiddenInGame(false);
	m_JustEvasiveCollision->SetVisibility(true);

	m_JustEvasiveCollision->OnComponentBeginOverlap.AddDynamic(
		this,
		&UPlayerEvasiveComponent::OnJustEvasiveOverlap
	);

	m_JustEvasiveCollision->UpdateOverlaps();
}


//ジャスト回避判定Collision破棄
void UPlayerEvasiveComponent::DestroyJustEvasiveCollision()
{
	m_bJustEvasiveWindowOpen = false;

	if (!m_JustEvasiveCollision) return;

	m_JustEvasiveCollision->OnComponentBeginOverlap.RemoveDynamic(
		this,
		&UPlayerEvasiveComponent::OnJustEvasiveOverlap
	);

	m_JustEvasiveCollision->DestroyComponent();
	m_JustEvasiveCollision = nullptr;
}


//ジャスト回避Overlap
void UPlayerEvasiveComponent::OnJustEvasiveOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!m_Player) return;
	if (!OtherActor || OtherActor == m_Player) return;
	if (!OtherComp) return;

	if (OtherComp->GetCollisionObjectType() != AttackDetection_Collision)
	{
		return;
	}

	//TakeDamage側で最終判定するため、ここでは成功処理を呼ばない
}

//ジャスト回避成功処理
void UPlayerEvasiveComponent::HandleJustEvasiveSuccess(AActor* Attacker)
{
	if (!m_Player) return;

	m_bIsJustEvasive = true;
	m_CurrentJustEvasiveTime = 0.f;

	m_Player->SetInvincible(true);
	m_Player->StartJustEvasiveInvincible();
	m_Player->SetIsEnhancedAttack(true);

	if (UPlayerAttackComponent* AttackComponent =
		m_Player->FindComponentByClass<UPlayerAttackComponent>())
	{
		AttackComponent->SetJustEvasiveCounterReady(true);

		//ジャスト回避成功直後から攻撃入力を予約可能にする
		AttackComponent->OpenJustEvasiveCounterReserveWindow();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[JustCounter] Ready:true Reserve:true")
		);
	}

	ShowJustEvasiveUI();

	m_CurrentSlowMotionWatchTime = 0.f;
	StartJustEvasiveSlowMotion();

	AddJustEvasiveGauge();

	if (IsValid(Attacker))
	{
		m_Player->BroadcastJustEvasiveSuccess(Attacker);
		OnJustEvasiveSuccess.Broadcast(Attacker);
	}
}

//ジャスト回避成功時に電力ゲージを加算
void UPlayerEvasiveComponent::AddJustEvasiveGauge()
{
	if (!m_ElectroGaugeComponent) return;

	m_ElectroGaugeComponent->AddElectroGauge(m_JustEvasiveGaugeAddValue);
	m_ElectroGaugeComponent->OnJustEvasiveSuccess();
}

//ジャスト回避UI表示
void UPlayerEvasiveComponent::ShowJustEvasiveUI()
{
	if (!m_Player) return;
	if (!JustEvasiveWidgetClass) return;

	if (m_JustEvasiveWidgetInstance)
	{
		m_JustEvasiveWidgetInstance->RemoveFromParent();
		m_JustEvasiveWidgetInstance = nullptr;
	}

	APlayerController* PC = Cast<APlayerController>(m_Player->GetController());
	if (!PC) return;

	m_JustEvasiveWidgetInstance = CreateWidget<UUserWidget>(PC, JustEvasiveWidgetClass);
	if (!m_JustEvasiveWidgetInstance) return;

	m_JustEvasiveWidgetInstance->AddToViewport();
}

//ジャスト回避UI非表示
void UPlayerEvasiveComponent::HideJustEvasiveUI()
{
	if (!m_JustEvasiveWidgetInstance) return;

	m_JustEvasiveWidgetInstance->RemoveFromParent();
	m_JustEvasiveWidgetInstance = nullptr;
}

//スロー演出開始
void UPlayerEvasiveComponent::StartJustEvasiveSlowMotion()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SetGlobalTimeDilation(World, m_JustEvasiveSlowTimeScale);

	World->GetTimerManager().ClearTimer(m_SlowMotionTimerHandle);

	World->GetTimerManager().SetTimer(
		m_SlowMotionTimerHandle,
		this,
		&UPlayerEvasiveComponent::RestoreGlobalTimeDilation,
		m_JustEvasiveSlowDuration,
		false
	);
}

//スロー演出終了
void UPlayerEvasiveComponent::RestoreGlobalTimeDilation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
	m_CurrentSlowMotionWatchTime = 0.f;
}
#include "PlayerThunderFlashComponent.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../../DataAssets/Player/PlayerSkillParameter.h"
#include "Components/SkeletalMeshComponent.h"

//初期化
UPlayerThunderFlashComponent::UPlayerThunderFlashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

//開始時
void UPlayerThunderFlashComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
}

//毎フレーム更新
void UPlayerThunderFlashComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateThunderFlash(DeltaTime);
}

//参照を初期化
void UPlayerThunderFlashComponent::InitializeThunderFlashComponent(
	APlayerCharacter* Player,
	UPlayerSkillParameter* SkillParameter
)
{
	m_Player = Player;
	m_SkillParameter = SkillParameter;
}

//ThunderFlash開始
void UPlayerThunderFlashComponent::BeginThunderFlash()
{
	if (!m_Player) return;
	if (!m_SkillParameter) return;
	if (m_bIsThunderFlashActive) return;

	m_RuntimeState.Reset();

	m_RuntimeState.StartLocation = m_Player->GetActorLocation();
	m_RuntimeState.TargetLocation = CalculateTargetLocation();

	m_bIsThunderFlashActive = true;

	ApplyStartState();

	OnThunderFlashStarted.Broadcast();
}

//ThunderFlash終了
void UPlayerThunderFlashComponent::EndThunderFlash()
{
	if (!m_bIsThunderFlashActive) return;

	RestoreEndState();

	m_bIsThunderFlashActive = false;
	m_RuntimeState.Reset();

	OnThunderFlashEnded.Broadcast();
}

//ThunderFlashを強制リセット
void UPlayerThunderFlashComponent::ResetThunderFlash()
{
	RestoreEndState();

	m_bIsThunderFlashActive = false;
	m_RuntimeState.Reset();
}

//ThunderFlash更新
void UPlayerThunderFlashComponent::UpdateThunderFlash(float DeltaTime)
{
	if (!m_bIsThunderFlashActive) return;
	if (!m_Player) return;
	if (!m_SkillParameter) return;

	m_RuntimeState.DelayElapsed += DeltaTime;

	if (m_RuntimeState.DelayElapsed < m_SkillParameter->ThunderFlashDelayTime)
	{
		return;
	}

	if (m_RuntimeState.bTeleported)
	{
		return;
	}

	ExecuteTeleport();
}

//目標位置を計算
FVector UPlayerThunderFlashComponent::CalculateTargetLocation() const
{
	if (!m_Player) return FVector::ZeroVector;
	if (!m_SkillParameter) return FVector::ZeroVector;

	return
		m_Player->GetActorLocation() +
		m_Player->GetActorForwardVector() *
		m_SkillParameter->ThunderFlashDistance;
}

//テレポートを実行
void UPlayerThunderFlashComponent::ExecuteTeleport()
{
	if (!m_Player) return;

	SetPlayerVisualVisible(false);

	m_Player->SetActorLocation(
		m_RuntimeState.TargetLocation,
		true
	);

	SetPlayerVisualVisible(true);

	m_RuntimeState.bTeleported = true;

	OnThunderFlashTeleported.Broadcast();
}

//プレイヤーの見た目を設定
void UPlayerThunderFlashComponent::SetPlayerVisualVisible(bool bVisible)
{
	if (!m_Player) return;

	USkeletalMeshComponent* Mesh = m_Player->GetMesh();
	if (!Mesh) return;

	Mesh->SetVisibility(bVisible, true);
}

//開始時の共通状態を設定
void UPlayerThunderFlashComponent::ApplyStartState()
{
	if (!m_Player) return;

	m_Player->DeleteCollision();
	m_Player->SetInputLocked(true);
}

//終了時の共通状態を戻す
void UPlayerThunderFlashComponent::RestoreEndState()
{
	if (!m_Player) return;

	SetPlayerVisualVisible(true);

	m_Player->RevivalCollision();
	m_Player->SetInputLocked(false);
}
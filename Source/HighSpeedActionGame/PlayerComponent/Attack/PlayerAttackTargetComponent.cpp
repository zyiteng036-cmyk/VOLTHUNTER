#include "PlayerAttackTargetComponent.h"
#include "../../DataAssets/Player/PlayerAttackParameter.h"
#include "../../Enemy/EnemyBase.h"
#include "../../Enemy/EnemyManager/EnemyManager.h"
#include "../../PlayerCharacter/PlayerCharacter.h"
#include "../Solver/PlayerAttackSolver.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

//初期化
UPlayerAttackTargetComponent::UPlayerAttackTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//所有者をプレイヤーとして取得
void UPlayerAttackTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<APlayerCharacter>(GetOwner());
}

//使用する参照を初期化
void UPlayerAttackTargetComponent::InitializeTargetComponent(APlayerCharacter* Player, UPlayerAttackParameter* AttackParameter)
{
	m_Player = Player;
	m_AttackParameter = AttackParameter;
}

//攻撃対象を検索して固定
AEnemyBase* UPlayerAttackTargetComponent::FindAttackTarget(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput)
{
	if (!m_Player || !m_AttackParameter) return nullptr;

	//移動入力がある場合は前回の固定対象を使用しない
	if (bHasMoveInput)
	{
		ClearLockedAttackTarget();
	}

	//入力がない場合は前回命中した敵を優先
	if (!bHasMoveInput && HasLockedAttackTarget())
	{
		AEnemyBase* LockedEnemy = GetLockedAttackTarget();

		const bool bIsLockedEnemyInRange = FPlayerAttackSolver::IsEnemyInInputDirection(
			PlayerLocation,
			FVector::ZeroVector,
			false,
			LockedEnemy,
			m_AttackParameter
		);

		if (bIsLockedEnemyInRange)
		{
			return LockedEnemy;
		}

		ClearLockedAttackTarget();
	}

	AEnemyBase* TargetEnemy = FindClosestAttackCandidate(PlayerLocation, MoveDirection, bHasMoveInput);
	if (!TargetEnemy) return nullptr;

	//次の入力なし攻撃でも再利用できるように固定
	SetLockedAttackTarget(TargetEnemy);

	return TargetEnemy;
}

//カメラ演出用の攻撃対象を検索
AEnemyBase* UPlayerAttackTargetComponent::FindAttackCameraTarget(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput) const
{
	//カメラ検索では固定ターゲット状態を変更しない
	return FindClosestAttackCandidate(PlayerLocation, MoveDirection, bHasMoveInput);
}

//固定攻撃ターゲットを設定
void UPlayerAttackTargetComponent::SetLockedAttackTarget(AEnemyBase* Enemy)
{
	m_LockedAttackTarget = Enemy;

	OnAttackTargetChanged.Broadcast(Enemy);
}

//const固定攻撃ターゲットを設定
void UPlayerAttackTargetComponent::SetLockedAttackTarget(const AEnemyBase* Enemy)
{
	SetLockedAttackTarget(const_cast<AEnemyBase*>(Enemy));
}

//固定攻撃ターゲットを解除
void UPlayerAttackTargetComponent::ClearLockedAttackTarget()
{
	m_LockedAttackTarget = nullptr;

	OnAttackTargetChanged.Broadcast(nullptr);
}

//固定攻撃ターゲットが有効か
bool UPlayerAttackTargetComponent::HasLockedAttackTarget() const
{
	if (!m_LockedAttackTarget.IsValid()) return false;

	const AEnemyBase* LockedEnemy = m_LockedAttackTarget.Get();
	if (!LockedEnemy) return false;

	//非アクティブまたは死亡中の敵は固定対象として扱わない
	return LockedEnemy->GetIsActive() && !LockedEnemy->GetIsDying();
}

//固定攻撃ターゲットを取得
AEnemyBase* UPlayerAttackTargetComponent::GetLockedAttackTarget() const
{
	return HasLockedAttackTarget() ? m_LockedAttackTarget.Get() : nullptr;
}

//敵が画面内にいるか
bool UPlayerAttackTargetComponent::IsEnemyVisibleForAttack(const AEnemyBase* Enemy) const
{
	if (!m_Player || !Enemy) return false;

	APlayerController* PlayerController = Cast<APlayerController>(m_Player->GetController());

	//Controllerがない環境では画面判定を行わない
	if (!PlayerController)
	{
		return true;
	}

	FVector2D ScreenPosition = FVector2D::ZeroVector;

	//敵のワールド位置を画面座標へ変換
	if (!PlayerController->ProjectWorldLocationToScreen(Enemy->GetActorLocation(), ScreenPosition))
	{
		return false;
	}

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

	//Viewportを取得できない場合は画面内として扱う
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		return true;
	}

	return
		ScreenPosition.X >= 0.f &&
		ScreenPosition.Y >= 0.f &&
		ScreenPosition.X <= static_cast<float>(ViewportSizeX) &&
		ScreenPosition.Y <= static_cast<float>(ViewportSizeY);
}

//条件に一致する最も近い攻撃対象を検索
AEnemyBase* UPlayerAttackTargetComponent::FindClosestAttackCandidate(const FVector& PlayerLocation, const FVector& MoveDirection, bool bHasMoveInput) const
{
	if (!m_Player || !m_AttackParameter) return nullptr;

	UWorld* World = m_Player->GetWorld();
	if (!World) return nullptr;

	UEnemyManager* EnemyManager = World->GetSubsystem<UEnemyManager>();
	if (!EnemyManager) return nullptr;

	const AEnemyBase* ClosestEnemy = EnemyManager->GetClosestActiveEnemyFromCoordinates(PlayerLocation);
	AEnemyBase* TargetEnemy = const_cast<AEnemyBase*>(ClosestEnemy);

	//攻撃対象として使用できない敵を除外
	if (!FPlayerAttackSolver::IsValidAttackEnemy(TargetEnemy))
	{
		return nullptr;
	}

	//距離と入力方向の条件に一致しない敵を除外
	if (!FPlayerAttackSolver::IsEnemyInInputDirection(PlayerLocation, MoveDirection, bHasMoveInput, TargetEnemy, m_AttackParameter))
	{
		return nullptr;
	}

	//入力方向攻撃では画面外の敵へ吸着しない
	if (bHasMoveInput && !IsEnemyVisibleForAttack(TargetEnemy))
	{
		return nullptr;
	}

	return TargetEnemy;
}
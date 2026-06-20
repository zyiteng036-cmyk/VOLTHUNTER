
#include "Player_SkillComponent.h"
#include "Components/CapsuleComponent.h"
#include "Player_MovementComponent.h"
#include "Player_AttackComponent.h"
#include "Player_CameraComponent.h"
#include "Player_ElectroGaugeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "../PlayerCharacter.h"

//定数定義
namespace SkillConstants
{
	constexpr float SelectInputThreshold = 0.5f; //スキル選択のスティック入力しきい値
}

//コンストラクタ
UPlayer_SkillComponent::UPlayer_SkillComponent()
{
	//毎フレームTick処理を行うように設定
	PrimaryComponentTick.bCanEverTick = true;
}

//ゲーム開始時に呼ばれる処理
void UPlayer_SkillComponent::BeginPlay()
{
	Super::BeginPlay();

	//オーナーをプレイヤーキャラクターとして取得
	m_Player = Cast<APlayerCharacter>(GetOwner());
	if (!m_Player) return;

	//各種コンポーネントの取得と保持
	m_MovementComponent = m_Player->FindComponentByClass<UPlayer_MovementComponent>();
	m_AttackComponent = m_Player->FindComponentByClass<UPlayer_AttackComponent>();
	m_ElectroGaugeComponent = m_Player->FindComponentByClass<UPlayer_ElectroGaugeComponent>();

	//初期状態のスキル使用可否を通知
	if (m_OnSkillUsableChanged.IsBound())
	{
		m_OnSkillUsableChanged.Broadcast(m_LastUsableState);
	}
}

//毎フレーム呼ばれる処理
void UPlayer_SkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//ThunderFlash実行中の更新処理
	if (m_IsSkillActive && m_SelectedSkill == EPlayerSkill::ThunderFlash)
	{
		_updateThunderFlash(DeltaTime);
	}

	//スキル発動中でヒットUIが未表示の場合の処理
	if (m_IsSkillActive && !m_HasShownHitUI && m_Player)
	{
		//プレイヤー側のヒットフラグを確認
		if (m_Player->GetIsHit())
		{
			//UIクラスが設定されているか確認
			if (m_SkillHitUIClass)
			{
				//UIを作成
				m_ActiveHitUIInstance = CreateWidget<UUserWidget>(GetWorld(), m_SkillHitUIClass);

				if (m_ActiveHitUIInstance)
				{
					//画面に追加
					m_ActiveHitUIInstance->AddToViewport();
				}
			}

			//表示済みフラグを立てて連発を防ぐ
			m_HasShownHitUI = true;
		}
	}

	//現在のスキル使用可否を取得
	const bool bCurrentUsable = CheckCurrentSkillUsable();

	//前回のフレームと状態が違っていたら通知を送る
	if (bCurrentUsable != m_LastUsableState)
	{
		m_LastUsableState = bCurrentUsable;

		//UI等のリスナーへ通知
		if (m_OnSkillUsableChanged.IsBound())
		{
			m_OnSkillUsableChanged.Broadcast(m_LastUsableState);
		}
	}
}

//スキル選択入力の処理
void UPlayer_SkillComponent::Input_SkillSelect(const FInputActionValue& Value)
{
	if (m_SelectLocked) return;
	if (m_IsSkillActive) return;
	if (!m_Player || !m_Player->HasActorBegunPlay()) return;

	//入力値の取得
	const float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis)) return;

	//連続入力を防ぐためロック
	m_SelectLocked = true;

	//入力方向に応じてスキルを切り替え
	if (Axis > SkillConstants::SelectInputThreshold)
	{
		SelectNextSkill();
	}
	else if (Axis < -SkillConstants::SelectInputThreshold)
	{
		SelectPrevSkill();
	}
}

//スキル選択入力の解除処理
void UPlayer_SkillComponent::Input_SkillReleased()
{
	//ロックを解除
	m_SelectLocked = false;
}

//スキル発動入力の処理
void UPlayer_SkillComponent::Input_SkillActive()
{
	if (!m_Player) return;
	if (m_MovementComponent->GetIsJump() || !m_Player->GetCharacterMovement()->IsMovingOnGround()) return;
	if (m_IsSkillActive) return;
	if (!m_CanSkillActive) return;

	//現在のアビリティをリセット
	m_CurrentAbilitySkill = nullptr;

	//選択中のスキルに応じた発動処理
	switch (m_SelectedSkill)
	{
	case EPlayerSkill::ThunderFlash:
	{
		//使用可能かチェック
		if (!CanUseThunderFlash())
		{
			return;
		}
		//アビリティを設定して開始処理を呼ぶ
		m_CurrentAbilitySkill = m_Player->m_AbilityPlayer_Skill01;
		StartThunderFlash();

		//電力ゲージを消費
		m_ElectroGaugeComponent->SubtractionElectoroGauge(m_ThunderFlashCost);
	}
	break;
	default:
		break;
	}

	if (!m_CurrentAbilitySkill) return;

	//スキル発動状態を設定
	m_IsSkillActive = true;
	m_Player->SetIsEnhancedAttack(true);
	m_Player->SetInvincible(true);

	//アビリティを実行
	m_Player->GetAbilitySystemComponent()->TryActivateAbilityByClass(m_CurrentAbilitySkill);
}

//次のスキルへ切り替え
void UPlayer_SkillComponent::SelectNextSkill()
{
	int32 SkillIndex = static_cast<int32>(m_SelectedSkill);
	SkillIndex = (SkillIndex + 1) % static_cast<int32>(EPlayerSkill::MAX);
	m_SelectedSkill = static_cast<EPlayerSkill>(SkillIndex);
}

//前のスキルへ切り替え
void UPlayer_SkillComponent::SelectPrevSkill()
{
	int32 SkillIndex = static_cast<int32>(m_SelectedSkill);
	SkillIndex = (SkillIndex - 1 + static_cast<int32>(EPlayerSkill::MAX)) % static_cast<int32>(EPlayerSkill::MAX);
	m_SelectedSkill = static_cast<EPlayerSkill>(SkillIndex);
}

//ThunderFlash開始処理
void UPlayer_SkillComponent::StartThunderFlash()
{
	if (!m_Player || !m_Player->HasActorBegunPlay()) return;

	//スキル用カメラの開始
	if (UPlayer_CameraComponent* CameraComp = m_Player->FindComponentByClass<UPlayer_CameraComponent>())
	{
		CameraComp->StartSkillCamera();
	}

	//初期化
	m_IsSkillActive = true;
	m_Teleported = false;
	m_DelayElapsed = 0.0f;

	m_HasShownHitUI = false;
	m_ActiveHitUIInstance = nullptr;

	//ワープ先の計算
	m_StartLocation = m_Player->GetActorLocation();
	m_TargetLocation = m_StartLocation + m_Player->GetActorForwardVector() * m_Skill01Distance;

	//ワープ前に敵に引っかからないようコリジョンを無視設定
	m_Player->GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Ignore
	);

	//移動と攻撃を禁止
	if (m_MovementComponent)
	{
		m_MovementComponent->SetCanMovement(false);
	}
	if (m_AttackComponent)
	{
		m_AttackComponent->SetCanAttack(false);
	}
}

//ThunderFlash更新処理
void UPlayer_SkillComponent::_updateThunderFlash(float DeltaTime)
{
	if (!m_Player || !m_Player->HasActorBegunPlay()) return;

	m_DelayElapsed += DeltaTime;

	//まだ待ち時間中の場合は処理を抜ける
	if (m_DelayElapsed < m_DelayTime)
	{
		return;
	}

	//すでにテレポート済みなら何もしない
	if (m_Teleported)
	{
		return;
	}

	//見た目を消す
	m_Player->GetMesh()->SetVisibility(false, true);

	//瞬間移動
	m_Player->SetActorLocation(m_TargetLocation, true);

	//表示を戻す
	m_Player->GetMesh()->SetVisibility(true, true);

	//テレポート完了フラグを立てる
	m_Teleported = true;
}

//スキルの終了処理
void UPlayer_SkillComponent::EndSkill()
{
	//強化攻撃と無敵状態を解除
	m_Player->SetIsEnhancedAttack(false);
	m_IsSkillActive = false;
	m_Player->SetInvincible(false);

	//UIが表示されていれば削除
	if (m_ActiveHitUIInstance)
	{
		m_ActiveHitUIInstance->RemoveFromParent();
		m_ActiveHitUIInstance = nullptr;
	}

	//コリジョン設定を元に戻す
	m_Player->GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Block
	);

	//移動と攻撃の許可を再開
	if (m_MovementComponent)
	{
		m_MovementComponent->SetCanMovement(true);
	}
	if (m_AttackComponent)
	{
		m_AttackComponent->SetCanAttack(true);
	}
}

//ThunderFlashが使用可能かどうかの判定
bool UPlayer_SkillComponent::CanUseThunderFlash() const
{
	if (!m_ElectroGaugeComponent) return false;

	//オーバーチャージ状態なら使用可能
	if (m_ElectroGaugeComponent->IsOvercharge())
	{
		return true;
	}

	//ゲージがコスト以上あれば使用可能
	return m_ElectroGaugeComponent->GetCurrentGauge() >= m_ThunderFlashCost;
}

//スキルが発動中か取得
bool UPlayer_SkillComponent::GetIsSkillActive() const
{
	return m_IsSkillActive;
}

//スキル発動可能状態の設定
void UPlayer_SkillComponent::SetCanSkillActive(bool CanSkillActive)
{
	m_CanSkillActive = CanSkillActive;
}

//現在選択中のスキルが使用可能かチェック
bool UPlayer_SkillComponent::CheckCurrentSkillUsable() const
{
	//プレイヤーやゲージコンポーネントがない場合は不可
	if (!m_Player || !m_ElectroGaugeComponent)
	{
		return false;
	}

	//選択中のスキルに応じてコスト判定
	switch (m_SelectedSkill)
	{
	case EPlayerSkill::ThunderFlash:
		return CanUseThunderFlash(); //既存の判定関数を利用

	default:
		return false;
	}
}
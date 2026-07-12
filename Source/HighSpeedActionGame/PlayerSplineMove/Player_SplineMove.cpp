#include "Player_SplineMove.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../PlayerComponent/PlayerAttackComponent.h"
#include "../PlayerComponent/PlayerEvasiveComponent.h"
#include "../PlayerComponent/PlayerSkillComponent.h"

namespace
{
	//Spline距離の最小値
	constexpr float MinSplineLength = 1.f;
}

//初期化
APlayer_SplineMove::APlayer_SplineMove()
{
	PrimaryActorTick.bCanEverTick = true;

	m_Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = m_Spline;

	m_StartCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("StartCollision"));
	m_StartCollision->SetupAttachment(RootComponent);

	m_SplineEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SplineEffect"));
	m_SplineEffectComponent->SetupAttachment(RootComponent);
	m_SplineEffectComponent->SetAutoActivate(false);
	m_SplineEffectComponent->SetVisibility(false, true);

	m_InputPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InputPromptWidget"));
	m_InputPromptWidget->SetupAttachment(m_StartCollision);
	m_InputPromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
	m_InputPromptWidget->SetDrawAtDesiredSize(true);
	m_InputPromptWidget->SetVisibility(false);

	m_MoveAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MoveAudioComponent"));
	m_MoveAudioComponent->SetupAttachment(RootComponent);
	m_MoveAudioComponent->SetAutoActivate(false);
}

//開始時
void APlayer_SplineMove::BeginPlay()
{
	Super::BeginPlay();

	if (m_StartCollision)
	{
		m_StartCollision->OnComponentBeginOverlap.AddDynamic(
			this,
			&APlayer_SplineMove::OnStartOverlap
		);

		m_StartCollision->OnComponentEndOverlap.AddDynamic(
			this,
			&APlayer_SplineMove::OnEndOverlap
		);
	}

	if (m_SplineEffectComponent)
	{
		m_SplineEffectComponent->Deactivate();
		m_SplineEffectComponent->SetVisibility(false, true);
	}
}

//毎フレーム更新
void APlayer_SplineMove::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickSplineMove(DeltaTime);
}

//プレイヤー側からSpline移動開始を要求
void APlayer_SplineMove::RequestStartSplineMove(AActor* RequestActor)
{
	if (!CanStartSplineMove(RequestActor)) return;

	BeginSplineMove(RequestActor);
}

//開始範囲に入った時
void APlayer_SplineMove::OnStartOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (!PlayerCharacter) return;

	if (!CachePlayerComponents(PlayerCharacter)) return;

	m_Player = PlayerCharacter;
	m_CandidateActor = PlayerCharacter;

	ShowInputPrompt();

	PlayerCharacter->SetCurrentSplineMoveActor(this);
}

//開始範囲から出た時
void APlayer_SplineMove::OnEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (m_bRunningSplineMove && m_MovingActor.Get() == OtherActor)
	{
		return;
	}

	if (m_CandidateActor.Get() != OtherActor) return;

	m_CandidateActor = nullptr;

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor))
	{
		PlayerCharacter->SetCurrentSplineMoveActor(nullptr);
	}

	HideInputPrompt();
}

//プレイヤー関連コンポーネントを取得
bool APlayer_SplineMove::CachePlayerComponents(APlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter) return false;

	m_AttackComponent = PlayerCharacter->FindComponentByClass<UPlayerAttackComponent>();
	m_EvasiveComponent = PlayerCharacter->FindComponentByClass<UPlayerEvasiveComponent>();
	m_SkillComponent = PlayerCharacter->FindComponentByClass<UPlayerSkillComponent>();

	if (!m_AttackComponent.IsValid()) return false;
	if (!m_EvasiveComponent.IsValid()) return false;
	if (!m_SkillComponent.IsValid()) return false;

	return true;
}

//Spline移動を開始できるか
bool APlayer_SplineMove::CanStartSplineMove(AActor* RequestActor) const
{
	if (m_bRunningSplineMove) return false;
	if (!RequestActor) return false;
	if (!m_Spline) return false;
	if (m_CandidateActor.Get() != RequestActor) return false;

	if (!m_AttackComponent.IsValid()) return false;
	if (!m_EvasiveComponent.IsValid()) return false;
	if (!m_SkillComponent.IsValid()) return false;

	if (m_SkillComponent->GetIsSkillActive())
	{
		return false;
	}

	return true;
}

//Spline移動を開始
void APlayer_SplineMove::BeginSplineMove(AActor* RequestActor)
{
	if (!RequestActor) return;
	if (!m_Spline) return;

	HideInputPrompt();

	m_bRunningSplineMove = true;
	m_MovingActor = RequestActor;

	const float SplineLength = m_Spline->GetSplineLength();

	m_StartDistance = m_bStartFromClosestPoint
		? m_Spline->FindInputKeyClosestToWorldLocation(RequestActor->GetActorLocation())
		: 0.f;

	m_CurrentDistance = FMath::Clamp(m_StartDistance, 0.f, SplineLength);

	DisablePlayerActions();
	ApplySplineMoveLock(RequestActor);

	const FVector StartLocation =
		m_Spline->GetLocationAtDistanceAlongSpline(
			m_CurrentDistance,
			ESplineCoordinateSpace::World
		);

	const FRotator StartRotation =
		m_Spline->GetRotationAtDistanceAlongSpline(
			m_CurrentDistance,
			ESplineCoordinateSpace::World
		);

	RequestActor->SetActorLocationAndRotation(StartLocation, StartRotation);

	StartSplineMoveEffect(StartLocation, StartRotation);
	StartMoveSound(StartLocation);

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(RequestActor))
	{
		PlayerCharacter->NotifySplineMoveStarted();
	}
}

//Spline移動を更新
void APlayer_SplineMove::TickSplineMove(float DeltaTime)
{
	if (!m_bRunningSplineMove) return;

	if (!m_MovingActor.IsValid())
	{
		FinishSplineMove();
		return;
	}

	if (!m_Spline)
	{
		FinishSplineMove();
		return;
	}

	const float SplineLength = m_Spline->GetSplineLength();

	if (SplineLength <= MinSplineLength)
	{
		FinishSplineMove();
		return;
	}

	m_CurrentDistance =
		FMath::Clamp(
			m_CurrentDistance + m_MoveSpeed * DeltaTime,
			0.f,
			SplineLength
		);

	const FVector Location =
		m_Spline->GetLocationAtDistanceAlongSpline(
			m_CurrentDistance,
			ESplineCoordinateSpace::World
		);

	const FRotator Rotation =
		m_Spline->GetRotationAtDistanceAlongSpline(
			m_CurrentDistance,
			ESplineCoordinateSpace::World
		);

	m_MovingActor->SetActorLocationAndRotation(Location, Rotation);

	if (m_SplineEffectComponent)
	{
		m_SplineEffectComponent->SetWorldLocationAndRotation(Location, Rotation);
	}

	if (m_MoveAudioComponent)
	{
		m_MoveAudioComponent->SetWorldLocation(Location);
	}

	if (m_CurrentDistance >= SplineLength)
	{
		FinishSplineMove();
	}
}

//Spline移動を終了
void APlayer_SplineMove::FinishSplineMove()
{
	AActor* FinishedActor = m_MovingActor.Get();

	m_bRunningSplineMove = false;

	if (FinishedActor)
	{
		RestoreSplineMoveLock(FinishedActor);
	}

	RestorePlayerActions();
	StopSplineMoveEffect();
	StopMoveSound();

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FinishedActor))
	{
		PlayerCharacter->NotifySplineMoveFinished();
		PlayerCharacter->SetCurrentSplineMoveActor(nullptr);
	}
	m_MovingActor = nullptr;
	m_CurrentDistance = 0.f;
	m_StartDistance = 0.f;
}

//移動開始時のロックを適用
void APlayer_SplineMove::ApplySplineMoveLock(AActor* TargetActor)
{
	if (!TargetActor) return;

	if (m_bDisableMoveInput)
	{
		if (APawn* Pawn = Cast<APawn>(TargetActor))
		{
			if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
			{
				PC->SetIgnoreMoveInput(true);
			}
		}
	}

	if (ACharacter* Character = Cast<ACharacter>(TargetActor))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
			MoveComp->DisableMovement();
		}

		if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
		{
			MeshComp->SetVisibility(!m_bHideMovingActorMesh, true);
		}
	}
}

//移動終了時のロックを解除
void APlayer_SplineMove::RestoreSplineMoveLock(AActor* TargetActor)
{
	if (!TargetActor) return;

	if (m_bDisableMoveInput)
	{
		if (APawn* Pawn = Cast<APawn>(TargetActor))
		{
			if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
			{
				PC->SetIgnoreMoveInput(false);
			}
		}
	}

	if (ACharacter* Character = Cast<ACharacter>(TargetActor))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}

		if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
		{
			MeshComp->SetVisibility(true, true);
		}
	}
}

//攻撃、回避、スキルを停止
void APlayer_SplineMove::DisablePlayerActions()
{
	if (m_AttackComponent.IsValid())
	{
		m_AttackComponent->SetCanAttack(false);
	}

	if (m_EvasiveComponent.IsValid())
	{
		m_EvasiveComponent->SetCanEvasive(false);
	}

	if (m_SkillComponent.IsValid())
	{
		m_SkillComponent->SetCanSkillActive(false);
	}
}

//攻撃、回避、スキルを復帰
void APlayer_SplineMove::RestorePlayerActions()
{
	if (m_AttackComponent.IsValid())
	{
		m_AttackComponent->SetCanAttack(true);
	}

	if (m_EvasiveComponent.IsValid())
	{
		m_EvasiveComponent->SetCanEvasive(true);
	}

	if (m_SkillComponent.IsValid())
	{
		m_SkillComponent->SetCanSkillActive(true);
	}
}

//Spline演出を開始
void APlayer_SplineMove::StartSplineMoveEffect(const FVector& Location, const FRotator& Rotation)
{
	if (!m_SplineEffectComponent) return;
	if (!m_SplineMoveEffect) return;

	m_SplineEffectComponent->Deactivate();
	m_SplineEffectComponent->SetAsset(m_SplineMoveEffect, true);
	m_SplineEffectComponent->SetWorldLocationAndRotation(Location, Rotation);
	m_SplineEffectComponent->SetVisibility(true, true);
	m_SplineEffectComponent->Activate(true);
}

//Spline演出を停止
void APlayer_SplineMove::StopSplineMoveEffect()
{
	if (!m_SplineEffectComponent) return;

	m_SplineEffectComponent->Deactivate();
	m_SplineEffectComponent->SetVisibility(false, true);
}

//移動音を開始
void APlayer_SplineMove::StartMoveSound(const FVector& Location)
{
	if (!m_MoveAudioComponent) return;
	if (!m_SplineMoveSound) return;

	m_MoveAudioComponent->SetSound(m_SplineMoveSound);
	m_MoveAudioComponent->SetWorldLocation(Location);
	m_MoveAudioComponent->Play();
}

//移動音を停止
void APlayer_SplineMove::StopMoveSound()
{
	if (!m_MoveAudioComponent) return;
	if (!m_MoveAudioComponent->IsPlaying()) return;

	m_MoveAudioComponent->Stop();
}

//入力案内UIを表示
void APlayer_SplineMove::ShowInputPrompt()
{
	if (!m_InputPromptWidget) return;

	m_InputPromptWidget->SetVisibility(true);
}

//入力案内UIを非表示
void APlayer_SplineMove::HideInputPrompt()
{
	if (!m_InputPromptWidget) return;

	m_InputPromptWidget->SetVisibility(false);
}
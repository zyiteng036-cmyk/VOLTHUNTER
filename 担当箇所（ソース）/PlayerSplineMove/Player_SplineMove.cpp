// Fill out your copyright notice in the Description page of Project Settings.


#include "Player_SplineMove.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "../PlayerComponent/Player_AttackComponent.h"
#include "../PlayerComponent/Player_EvasiveComponent.h"
#include "../PlayerComponent/Player_SkillComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../PlayerCharacter.h"
#include "Components/WidgetComponent.h"
#include "Components/AudioComponent.h"

// Sets default values
APlayer_SplineMove::APlayer_SplineMove()
	:m_Spline(nullptr)
	, m_StartCollision(nullptr)
	, m_SplineEffectComponent(nullptr)
	, m_SplineMoveEffect(nullptr)
	, m_MoveSpeed(1500.f)
	, m_RunningSplineMove(false)
	, m_StartDistance(0.f)
	, m_CurrentDistance(0.f)
	, m_CandidateActor(nullptr)
	, m_MovingActor(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_Spline = CreateDefaultSubobject<USplineComponent>("Spline");
	RootComponent = m_Spline;

	m_StartCollision = CreateDefaultSubobject<UBoxComponent>("StartCollision");
	m_StartCollision->SetupAttachment(RootComponent);

	m_SplineEffectComponent = CreateDefaultSubobject<UNiagaraComponent>("SplineEffect");
	m_SplineEffectComponent->SetupAttachment(RootComponent);
	m_SplineEffectComponent->SetAutoActivate(false);

	m_InputPromptWidget = CreateDefaultSubobject<UWidgetComponent>("InputPromptWidget");
	m_InputPromptWidget->SetupAttachment(m_StartCollision); // コリジョンに追従させる
	m_InputPromptWidget->SetWidgetSpace(EWidgetSpace::Screen); // 常にカメラの方を向く設定（Worldにすると板ポリ状になる）
	m_InputPromptWidget->SetDrawAtDesiredSize(true); // ウィジェットのサイズに合わせて自動調整
	m_InputPromptWidget->SetVisibility(false); // 最初は非表示

	m_MoveAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MoveAudioComponent"));
	m_MoveAudioComponent->SetupAttachment(RootComponent);
	m_MoveAudioComponent->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void APlayer_SplineMove::BeginPlay()
{
	Super::BeginPlay();

	if (m_SplineMoveEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineMoveEffect OK: %s"), *m_SplineMoveEffect->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SplineMoveEffect STILL NULL in BeginPlay"));
	}
	m_StartCollision->OnComponentBeginOverlap.AddDynamic(this, &APlayer_SplineMove::OnStartOverlap);

	m_StartCollision->OnComponentEndOverlap.AddDynamic(this, &APlayer_SplineMove::OnEndOverlap);



}

// Called every frame
void APlayer_SplineMove::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!m_RunningSplineMove || !m_MovingActor.IsValid())
		return;

	//距離を速度で
	m_CurrentDistance += m_MoveSpeed * DeltaTime;

	const float SplineLength = m_Spline->GetSplineLength();


	const FVector Location = m_Spline->GetLocationAtDistanceAlongSpline(m_CurrentDistance, ESplineCoordinateSpace::World);
	const FRotator Rotation = m_Spline->GetRotationAtDistanceAlongSpline(m_CurrentDistance, ESplineCoordinateSpace::World);

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
		m_RunningSplineMove = false;
		m_AttackComponent->SetCanAttack(true);
		m_EvasiveComponent->SetCanEvasive(true);
		m_SkillComponent->SetCanSkillActive(true);


		// キャラの移動復帰＆メッシュ表示
		if (ACharacter* Character = Cast<ACharacter>(m_MovingActor.Get()))
		{
			// === 入力復帰 ===
			if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
			{
				PlayerController->SetIgnoreMoveInput(false);
			}
			if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Walking);
			}
			if (USkeletalMeshComponent* MeshComp = Cast<USkeletalMeshComponent>(m_MovingActor->GetComponentByClass(USkeletalMeshComponent::StaticClass())))
			{
				MeshComp->SetVisibility(true, true);
			}
		}

		// エフェクト確実非表示・停止
		if (m_SplineEffectComponent)
		{
			m_SplineEffectComponent->Deactivate();
			m_SplineEffectComponent->SetVisibility(false, true);
		}

		if (m_MoveAudioComponent && m_MoveAudioComponent->IsPlaying())
		{
			m_MoveAudioComponent->Stop();
		}

		m_MovingActor = nullptr;
	}
}



void APlayer_SplineMove::RequestStartSplineMove(AActor* RequestActor)
{
	if (m_RunningSplineMove) return;
	if (RequestActor != m_CandidateActor) return;
	if (m_SkillComponent && m_SkillComponent->GetIsSkillActive())return;

	if (m_InputPromptWidget)
	{
		m_InputPromptWidget->SetVisibility(false);
	}

	m_RunningSplineMove = true;
	m_MovingActor = RequestActor;
	m_CurrentDistance = 0.f;

	//スタート距離を計算
	const float InputKey = m_Spline->FindInputKeyClosestToWorldLocation(RequestActor->GetActorLocation());

	m_AttackComponent->SetCanAttack(false);
	m_EvasiveComponent->SetCanEvasive(false);
	m_SkillComponent->SetCanSkillActive(false);

	// === 入力制御 ===
	if (APawn* Pawn = Cast<APawn>(RequestActor))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			PlayerController->SetIgnoreMoveInput(true);   // ★移動のみ無効
		}
	}

	// キャラの移動停止＆メッシュ非表示
	if (ACharacter* Character = Cast<ACharacter>(RequestActor))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
			MoveComp->DisableMovement();
		}
		if (USkeletalMeshComponent* MeshComp = Cast<USkeletalMeshComponent>(RequestActor->GetComponentByClass(USkeletalMeshComponent::StaticClass())))
		{
			MeshComp->SetVisibility(false, true);
		}
	}

	if (m_MoveAudioComponent && m_SplineMoveSound)
	{
		m_MoveAudioComponent->SetSound(m_SplineMoveSound);
		m_MoveAudioComponent->Play();
	}

	if (m_SplineEffectComponent)
	{
		if (m_SplineMoveEffect)
		{
			// 毎回アセットセット→Activate(true)でリセットし2回目以降を保証
			m_SplineEffectComponent->Deactivate();
			m_SplineEffectComponent->SetAsset(m_SplineMoveEffect, true);
			m_SplineEffectComponent->Activate(true);
			m_SplineEffectComponent->SetVisibility(true, true);


			UE_LOG(LogTemp, Warning, TEXT("Spline Niagara Effect Activated"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SplineMoveEffect is NULL"));
		}
	}

}

void APlayer_SplineMove::OnStartOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (!PlayerCharacter) return;

	m_Player = PlayerCharacter;

	m_AttackComponent = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>();
	m_EvasiveComponent = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>();
	m_SkillComponent = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>();

	if (!m_AttackComponent || !m_EvasiveComponent || !m_SkillComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("SplineMove: Player components missing"));
		return;
	}
	if (m_InputPromptWidget)
	{
		m_InputPromptWidget->SetVisibility(true);
	}

	m_CandidateActor = PlayerCharacter;
	PlayerCharacter->SetCurrentSplineMoveActor(this);
}

void APlayer_SplineMove::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (m_CandidateActor.Get() == OtherActor)
	{
		m_CandidateActor = nullptr;

		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor))
		{
			PlayerCharacter->SetCurrentSplineMoveActor(nullptr);
		}

		if (m_InputPromptWidget)
		{
			m_InputPromptWidget->SetVisibility(false);
		}
	}
}


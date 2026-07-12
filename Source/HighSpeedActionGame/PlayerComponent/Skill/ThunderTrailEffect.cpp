#include "ThunderTrailEffect.h"
#include "NiagaraComponent.h"

//初期化
AThunderTrailEffect::AThunderTrailEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

	m_NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	RootComponent = m_NiagaraComponent;

	if (m_NiagaraComponent)
	{
		m_NiagaraComponent->SetAutoActivate(true);
	}
}

//開始時
void AThunderTrailEffect::BeginPlay()
{
	Super::BeginPlay();

	m_StartLocation = GetActorLocation();
	m_FixedRotation = MakeFixedHorizontalRotation(GetActorRotation());
}

//毎フレーム更新
void AThunderTrailEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_bIsFadeOut)
	{
		UpdateFadeOut(DeltaTime);
		return;
	}

	UpdateTrailFollow();
}

//フェードアウト開始
void AThunderTrailEffect::BeginFadeOut()
{
	if (m_bIsFadeOut) return;

	m_bIsFadeOut = true;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetLifeSpan(m_FadeOutLifeSpan);
}

//追従中の更新
void AThunderTrailEffect::UpdateTrailFollow()
{
	AActor* ParentActor = GetAttachParentActor();
	if (!ParentActor) return;
	if (m_BaseEffectLength <= 0.f) return;

	SetActorLocation(m_StartLocation);
	SetActorRotation(m_FixedRotation);

	const FVector CurrentPlayerLocation = ParentActor->GetActorLocation();
	const FVector MoveVector = CurrentPlayerLocation - m_StartLocation;
	const FVector ForwardVector = FRotationMatrix(m_FixedRotation).GetUnitAxis(EAxis::X);

	const float ForwardDistance =
		FMath::Max(
			0.f,
			FVector::DotProduct(MoveVector, ForwardVector)
		);

	const FVector NewScale =
		FVector(
			ForwardDistance / m_BaseEffectLength,
			m_ThicknessScale,
			m_ThicknessScale
		);

	SetActorScale3D(NewScale);
}

//フェードアウト中の更新
void AThunderTrailEffect::UpdateFadeOut(float DeltaTime)
{
	const FVector CurrentScale = GetActorScale3D();

	const float NewThickness =
		FMath::FInterpTo(
			CurrentScale.Y,
			0.f,
			DeltaTime,
			m_FadeOutInterpSpeed
		);

	SetActorScale3D(
		FVector(
			CurrentScale.X,
			NewThickness,
			NewThickness
		)
	);
}

//固定回転を作成
FRotator AThunderTrailEffect::MakeFixedHorizontalRotation(
	const FRotator& SourceRotation
) const
{
	FRotator FixedRotation = SourceRotation;
	FixedRotation.Pitch = 0.f;
	FixedRotation.Roll = 0.f;

	return FixedRotation;
}
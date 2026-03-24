// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackCollisionDetection.h"
#include "../CollisionChannelName.h"
#include "../IDamageable/Damageable.h"
#include "Components/StaticMeshComponent.h"
#include "../HitJudgmentComponent/HitJudgmentComponent.h"

namespace {
	constexpr float InitRadius = 30.f;//初期半径
}

// Sets default values
AAttackCollisionDetection::AAttackCollisionDetection()
	:m_VisualMesh(nullptr)
	, m_HitSphere(nullptr)
	, m_IsActive(false)
	, m_Visible(false)
	, m_HitActorTags()
	, m_Damage(0.f)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Tags.AddUnique(FName(TEXT("AttackCollisionDetection")));

	m_HitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HitSphere"));
	RootComponent = m_HitSphere.Get();
	m_HitSphere->InitSphereRadius(InitRadius);
	m_HitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//衝突判定なしに設定
	m_HitSphere->SetCollisionObjectType(AttackDetection_Collision);
	m_HitSphere->SetCollisionResponseToAllChannels(ECR_Ignore);//他のオブジェクトと衝突判定を受けないようにする↓のがあればPawnとはOverlapちゃんとする
	m_HitSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);//Pawnと重なった時だけ衝突判定するようにする設定
	m_HitSphere->SetCollisionResponseToChannel(JustEvasive_Collision, ECR_Overlap);
	//m_HitSphere->SetHiddenInGame(false);



	m_HitSphere->OnComponentBeginOverlap.AddDynamic(this, &AAttackCollisionDetection::OnOverlapBegin);


	// StaticMesh生成
	m_VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("m_VisualMesh"));
	m_VisualMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));

	if (SphereMesh.Succeeded())
	{
		m_VisualMesh->SetStaticMesh(SphereMesh.Object);
		m_VisualMesh->SetRelativeScale3D(FVector(1.0f));
		m_VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		m_VisualMesh->SetupAttachment(RootComponent);
	}

	SetActive(false);
	SetVisible(false);

}

// Called when the game starts or when spawned
void AAttackCollisionDetection::BeginPlay()
{
	Super::BeginPlay();
	//オーバーラップイベント有効に//コンストラクタだと動作せず
	m_HitSphere->SetGenerateOverlapEvents(true);
}

// Called every frame
void AAttackCollisionDetection::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);
	if (!m_IsActive) { return; }

	if (!m_Visible) { return; }
	//動的にコリジョンサイズが変わるかも知れないので記入しておく
	// スフィアの半径取得
	float SphereRadius = m_HitSphere->GetUnscaledSphereRadius();

	//半径50cmが基本サイズ
	constexpr float BASE_RADIUS = 50.0f;

	float Scale = SphereRadius / BASE_RADIUS;

	m_VisualMesh->SetRelativeScale3D(FVector(Scale));

}

void AAttackCollisionDetection::SetActive(const bool _isActive) {
	m_IsActive = _isActive;

	if (!IsValid(this) || !m_HitSphere)return;
	SetActorHiddenInGame(!_isActive);//隠すかのフラグなのでフラグ反転
	SetActorEnableCollision(_isActive);

	if (_isActive) { m_HitSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); }//コリジョンの反発はしない設定（overrapはする）
	else { m_HitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); }

	//非アクティブ時のみの設定
	if (!_isActive) {
		SetVisible(false);
		m_HitActorTags.Empty();
	}
}

bool AAttackCollisionDetection::GetIsActive()const {
	return m_IsActive;
}

void AAttackCollisionDetection::SetHitActorTags(const TArray<FString> _tags) {
	m_HitActorTags = _tags;
}

void AAttackCollisionDetection::SetRadius(const float _radius) {
	m_HitSphere->SetSphereRadius(_radius, true);
}

void AAttackCollisionDetection::SetLocation(const FVector& _location) {
	SetActorLocation(_location);
}

void AAttackCollisionDetection::SetDamage(const float _damage) {
	m_Damage = _damage;
}

void AAttackCollisionDetection::SetDamageInfo(const FDamageInfo& _damageInfo) {
	m_DamageInfo = _damageInfo;
}

void AAttackCollisionDetection::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor == this)return;

	bool IsHit = false;

	for (int i = 0; i < m_HitActorTags.Num(); ++i) {
		FName Tag(*m_HitActorTags[i]);//FNameにキャスト
		if (OtherActor->ActorHasTag(Tag)) {
			IsHit = true;
			break;
		}
	}

	if (!IsHit)return;

	IDamageable* Damageable = Cast<IDamageable>(OtherActor);

	if (Damageable) {
		Damageable->TakeDamage(m_DamageInfo);
		if (m_HitJudgmentComponent) {
			m_HitJudgmentComponent->NotifyAttackHit(OtherActor);
		}
	}
}

void AAttackCollisionDetection::SetVisible(const bool _isvisible) {
	if (m_VisualMesh && m_VisualMesh->IsValidLowLevel() && IsValid(m_VisualMesh))
	{
		m_VisualMesh->SetVisibility(_isvisible, true);
		SetActorHiddenInGame(!_isvisible);//隠すかのフラグなのでフラグ反転

		m_Visible = _isvisible;

	}
}

void AAttackCollisionDetection::SetHitJudgmentComponent(const UHitJudgmentComponent* _hitJudgmentComponent) {
	m_HitJudgmentComponent = _hitJudgmentComponent;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AHomingProjectile::AHomingProjectile()
	:m_LaunchDelay(0.5f)
	,m_LaunchUpSpeed(1500.f)
	,m_LaunchRandomSpread(30.f)
	,m_HomingMagnitude(1500000.f)
	,m_StopHomingDistance(300.f)
	, m_bHasHit(false)
{
	// 親クラスで生成済みのコンポーネント設定
	if (m_ProjectileMovementComponent)
	{
		// 最初は誘導をOFFにしておく
		m_ProjectileMovementComponent->bIsHomingProjectile = false;

		// 弾の速度設定（かなり速くする）
		m_ProjectileMovementComponent->InitialSpeed = 12000.f; // 初速
		m_ProjectileMovementComponent->MaxSpeed = 15000.f;     // 最大速度

		// 誘導時の摩擦（0にすると慣性でフワフワし、高くするとキビキビ動く）
		m_ProjectileMovementComponent->ProjectileGravityScale = 1.f; // 少し重力を残すとミサイルっぽい
	}
}

void AHomingProjectile::BeginPlay()
{
	Super::BeginPlay();

	m_PrevLocation = GetActorLocation();

	//上方向への打ち上げ
	FVector LaunchDirection = FVector::UpVector;

	//少しランダムな角度
	LaunchDirection = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(LaunchDirection, m_LaunchRandomSpread);

	//速度を適用
	if (m_ProjectileMovementComponent)
	{
		m_ProjectileMovementComponent->Velocity = LaunchDirection * m_LaunchUpSpeed;
	}

	//指定時間後に追従
	GetWorld()->GetTimerManager().SetTimer(
		m_HomingTimerHandle,
		this,
		&AHomingProjectile::StartHoming,
		m_LaunchDelay,
		false
	);
}

void AHomingProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_bHasHit)
	{
		return;
	}
	if (CheckCollision())
	{
		return;
	}

	// 誘導中、かつターゲットが存在する場合のみチェック
	if (m_ProjectileMovementComponent && m_ProjectileMovementComponent->bIsHomingProjectile)
	{
		USceneComponent* Target = m_ProjectileMovementComponent->HomingTargetComponent.Get();

		if (Target)
		{
			// ターゲット（プレイヤー）の位置を取得
			FVector TargetLocation = Target->GetComponentLocation();
			// 自分の位置
			FVector MyLocation = GetActorLocation();

			// 距離の2乗を計算（ルート計算を避けるため高速）
			float DistSquared = FVector::DistSquared(TargetLocation, MyLocation);

			// 設定距離の2乗と比較
			// 例: 距離300なら、300*300 = 90000 と比較
			if (DistSquared <= (m_StopHomingDistance * m_StopHomingDistance))
			{
				// 距離圏内に入ったので誘導OFF！
				m_ProjectileMovementComponent->bIsHomingProjectile = false;

				m_ProjectileMovementComponent->HomingTargetComponent = nullptr;

			}
		}
	}
}

void AHomingProjectile::StartHoming()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	if (PlayerPawn && m_ProjectileMovementComponent)
	{
		// ターゲットをセット
		m_ProjectileMovementComponent->HomingTargetComponent = PlayerPawn->GetRootComponent();

		// 誘導をON
		m_ProjectileMovementComponent->bIsHomingProjectile = true;

		// 誘導の強さを適用
		m_ProjectileMovementComponent->HomingAccelerationMagnitude = m_HomingMagnitude;
	
		m_ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

		FVector CurrentVelocity = m_ProjectileMovementComponent->Velocity;

		if (CurrentVelocity.Z > 0.f)
		{
			CurrentVelocity.Z = 0.f;
		}

		// 速度を再適用
		m_ProjectileMovementComponent->Velocity = CurrentVelocity;
	}
}

bool AHomingProjectile::CheckCollision()
{
	//今の場所
	FVector CurrentLocation = GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner()); 

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		m_PrevLocation,    // Start
		CurrentLocation,   // End
		ECC_WorldStatic,   // 壁・床などの静的オブジェクトのみ判定
		QueryParams
	);

	//ヒット時の処理
	if (bHit)
	{
		m_ProjectileMovementComponent->bIsHomingProjectile = false;

		AActor* HitActor = HitResult.GetActor();
		// 自分以外 かつ オーナー(Boss)以外 に当たった場合
		if (HitActor && HitActor != this && HitActor != GetOwner())
		{
			//二重ヒット防止フラグを立てる
			m_bHasHit = true;

			//親クラスのメソッドを呼んで、攻撃判定を消す
			OnCollisionEnd();

			//数秒後に完全に消滅させる
			GetWorld()->GetTimerManager().SetTimer(
				m_FadeOutTimerHandle,
				[this]()
				{
					Destroy();
				},
				1.f, 
				false
			);

			return true;
		}
	}

	//当たらなかったので、位置を更新して次フレームへ
	m_PrevLocation = CurrentLocation;
	return false;
}


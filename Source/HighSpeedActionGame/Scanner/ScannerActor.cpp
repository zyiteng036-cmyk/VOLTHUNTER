// Fill out your copyright notice in the Description page of Project Settings.


#include "ScannerActor.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

// Sets default values
AScannerActor::AScannerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.bStartWithTickEnabled = false;

	//メッシュコンポーネントの作成
	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	RootComponent = SphereMesh;

	//コリジョンなしの設定
	SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereMesh->SetCollisionProfileName(TEXT("NoCollision"));

	//デフォルト値の設定
	ExpansionSpeed = 20.0f;
	LifeDuration = 2.0f; 
	bIsActive = false;
}

// Called when the game starts or when spawned
void AScannerActor::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void AScannerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive)
	{
		// 現在のスケールを取得
		FVector CurrentScale = GetActorScale3D();

		// 加算するスケール量を計算 (Speed * DeltaTime)
		float ScaleAmount = ExpansionSpeed * DeltaTime;
		FVector AddedScale(ScaleAmount, ScaleAmount, ScaleAmount);

		// 新しいスケールを適用
		SetActorScale3D(CurrentScale + AddedScale);
	}
}

void AScannerActor::StartScannerEffect()
{
	if (bIsActive) return; // すでに動いていたら無視

	bIsActive = true;

	// 拡大処理のためにTickを有効化
	SetActorTickEnabled(true);

	// 指定時間後に DestroySelf を呼ぶタイマーをセット
	GetWorld()->GetTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&AScannerActor::DestroySelf,
		LifeDuration,
		false
	);
}

void AScannerActor::DestroySelf()
{
	Destroy();
}


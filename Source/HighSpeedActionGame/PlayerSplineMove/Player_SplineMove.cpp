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

namespace PlayerSplineMoveConstants
{
	constexpr float DefaultDistance = 0.0f; //初期距離用の定数
}

//デフォルト値の設定とコンポーネントの生成
APlayer_SplineMove::APlayer_SplineMove()
{
	//毎フレームTickを呼び出す設定
	PrimaryActorTick.bCanEverTick = true;

	//スプラインコンポーネントの生成とルート設定
	m_Spline = CreateDefaultSubobject<USplineComponent>("Spline");
	RootComponent = m_Spline;

	//開始位置判定用コリジョンの生成とアタッチ
	m_StartCollision = CreateDefaultSubobject<UBoxComponent>("StartCollision");
	m_StartCollision->SetupAttachment(RootComponent);

	//移動演出用エフェクトの生成とアタッチ
	m_SplineEffectComponent = CreateDefaultSubobject<UNiagaraComponent>("SplineEffect");
	m_SplineEffectComponent->SetupAttachment(RootComponent);
	m_SplineEffectComponent->SetAutoActivate(false);

	//操作を促すUIウィジェットの生成と設定
	m_InputPromptWidget = CreateDefaultSubobject<UWidgetComponent>("InputPromptWidget");
	m_InputPromptWidget->SetupAttachment(m_StartCollision); //コリジョンに追従させる
	m_InputPromptWidget->SetWidgetSpace(EWidgetSpace::Screen); //常にカメラの方を向く設定(Worldにすると板ポリ状になる)
	m_InputPromptWidget->SetDrawAtDesiredSize(true); //ウィジェットのサイズに合わせて自動調整
	m_InputPromptWidget->SetVisibility(false); //最初は非表示

	//移動音再生用オーディオコンポーネントの生成とアタッチ
	m_MoveAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MoveAudioComponent"));
	m_MoveAudioComponent->SetupAttachment(RootComponent);
	m_MoveAudioComponent->SetAutoActivate(false);
}

//ゲーム開始時またはスポン時の初期化
void APlayer_SplineMove::BeginPlay()
{
	Super::BeginPlay();

	//エフェクトアセットの割り当て成否チェック
	if (m_SplineMoveEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineMoveEffect OK: %s"), *m_SplineMoveEffect->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SplineMoveEffect STILL NULL in BeginPlay"));
	}

	//コリジョンのオーバーラップイベントをバインド
	m_StartCollision->OnComponentBeginOverlap.AddDynamic(this, &APlayer_SplineMove::OnStartOverlap);
	m_StartCollision->OnComponentEndOverlap.AddDynamic(this, &APlayer_SplineMove::OnEndOverlap);
}

//毎フレームの更新処理
void APlayer_SplineMove::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//移動中でない、または移動対象アクターが無効なら処理しない
	if (!m_RunningSplineMove || !m_MovingActor.IsValid())
		return;

	//フレーム時間と移動速度に基づいて現在の走行距離を更新
	m_CurrentDistance += m_MoveSpeed * DeltaTime;

	//スプライン全体の長さを取得
	const float SplineLength = m_Spline->GetSplineLength();

	//スプライン上の現在距離に応じた座標と回転を取得
	const FVector Location = m_Spline->GetLocationAtDistanceAlongSpline(m_CurrentDistance, ESplineCoordinateSpace::World);
	const FRotator Rotation = m_Spline->GetRotationAtDistanceAlongSpline(m_CurrentDistance, ESplineCoordinateSpace::World);

	//移動対象のアクターの座標と回転を同期
	m_MovingActor->SetActorLocationAndRotation(Location, Rotation);

	//エフェクトコンポーネントの位置を同期
	if (m_SplineEffectComponent)
	{
		m_SplineEffectComponent->SetWorldLocationAndRotation(Location, Rotation);
	}

	//オーディオコンポーネントの位置を同期
	if (m_MoveAudioComponent)
	{
		m_MoveAudioComponent->SetWorldLocation(Location);
	}

	//終点に到達したかの判定
	if (m_CurrentDistance >= SplineLength)
	{
		//走行状態を終了
		m_RunningSplineMove = false;

		//プレイヤーのアクション制限を解除
		m_AttackComponent->SetCanAttack(true);
		m_EvasiveComponent->SetCanEvasive(true);
		m_SkillComponent->SetCanSkillActive(true);

		//キャラの移動復帰とメッシュ表示
		if (ACharacter* Character = Cast<ACharacter>(m_MovingActor.Get()))
		{
			//入力復帰
			if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
			{
				PlayerController->SetIgnoreMoveInput(false);
			}
			//移動モードを歩行に復帰
			if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Walking);
			}
			//メッシュの可視性を復帰
			if (USkeletalMeshComponent* MeshComp = Cast<USkeletalMeshComponent>(m_MovingActor->GetComponentByClass(USkeletalMeshComponent::StaticClass())))
			{
				MeshComp->SetVisibility(true, true);
			}
		}

		//エフェクトの非表示と停止
		if (m_SplineEffectComponent)
		{
			m_SplineEffectComponent->Deactivate();
			m_SplineEffectComponent->SetVisibility(false, true);
		}

		//サウンドの停止
		if (m_MoveAudioComponent && m_MoveAudioComponent->IsPlaying())
		{
			m_MoveAudioComponent->Stop();
		}

		//移動対象の参照をクリア
		m_MovingActor = nullptr;
	}
}

//プレイヤー側から移動開始を要求
void APlayer_SplineMove::RequestStartSplineMove(AActor* RequestActor)
{
	//多重走行の防止、候補ではないアクターからの要求拒否、スキル発動中の拒否チェック
	if (m_RunningSplineMove) return;
	if (RequestActor != m_CandidateActor) return;
	if (m_SkillComponent.IsValid() && m_SkillComponent->GetIsSkillActive()) return;

	//UIプロンプトを非表示にする
	if (m_InputPromptWidget)
	{
		m_InputPromptWidget->SetVisibility(false);
	}

	//走行フラグと対象を設定
	m_RunningSplineMove = true;
	m_MovingActor = RequestActor;
	m_CurrentDistance = PlayerSplineMoveConstants::DefaultDistance;

	//スタート距離を計算
	const float InputKey = m_Spline->FindInputKeyClosestToWorldLocation(RequestActor->GetActorLocation());

	//走行中のプレイヤーアクションを禁止
	m_AttackComponent->SetCanAttack(false);
	m_EvasiveComponent->SetCanEvasive(false);
	m_SkillComponent->SetCanSkillActive(false);

	//入力制御(移動操作のみ無効化)
	if (APawn* Pawn = Cast<APawn>(RequestActor))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			PlayerController->SetIgnoreMoveInput(true);
		}
	}

	//キャラの移動停止とメッシュ非表示
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

	//効果音の再生開始
	if (m_MoveAudioComponent && m_SplineMoveSound)
	{
		m_MoveAudioComponent->SetSound(m_SplineMoveSound);
		m_MoveAudioComponent->Play();
	}

	//走行エフェクトの再生と初期化
	if (m_SplineEffectComponent)
	{
		if (m_SplineMoveEffect)
		{
			//毎回アセットセットとActivateでリセットし2回目以降を保証
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

//アクセス可能範囲に入った時
void APlayer_SplineMove::OnStartOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//進入したのがプレイヤーキャラクターかチェック
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (!PlayerCharacter) return;

	m_Player = PlayerCharacter;

	//プレイヤーの各制御コンポーネントを取得
	m_AttackComponent = PlayerCharacter->FindComponentByClass<UPlayer_AttackComponent>();
	m_EvasiveComponent = PlayerCharacter->FindComponentByClass<UPlayer_EvasiveComponent>();
	m_SkillComponent = PlayerCharacter->FindComponentByClass<UPlayer_SkillComponent>();

	//コンポーネントの有効性チェック
	if (!m_AttackComponent.IsValid() || !m_EvasiveComponent.IsValid() || !m_SkillComponent.IsValid())
	{
		return;
	}

	//入力プロンプトUIを表示
	if (m_InputPromptWidget)
	{
		m_InputPromptWidget->SetVisibility(true);
	}

	//走行候補として設定
	m_CandidateActor = PlayerCharacter;
	PlayerCharacter->SetCurrentSplineMoveActor(this);
}

//アクセス可能範囲から出た時
void APlayer_SplineMove::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//候補アクターが範囲外に出た場合
	if (m_CandidateActor.Get() == OtherActor)
	{
		m_CandidateActor = nullptr;

		//プレイヤー側の参照を解除
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor))
		{
			PlayerCharacter->SetCurrentSplineMoveActor(nullptr);
		}

		//入力プロンプトUIを非表示
		if (m_InputPromptWidget)
		{
			m_InputPromptWidget->SetVisibility(false);
		}
	}
}
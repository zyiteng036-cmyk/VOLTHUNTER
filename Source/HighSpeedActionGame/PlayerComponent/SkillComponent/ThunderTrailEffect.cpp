//設定でプロジェクトのコピーライト通知を記述してください

#include "ThunderTrailEffect.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"

namespace EffectConstants
{
	constexpr float ZeroValue = 0.0f; //計算や初期化用のゼロ値
}

//コンストラクタ
AThunderTrailEffect::AThunderTrailEffect()
{
	//毎フレームTick処理を行うように設定
	PrimaryActorTick.bCanEverTick = true;

	//プレイヤーの移動が終わった後に位置を更新する
	//Tickグループを「TG_PostUpdateWork」に設定
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

	//コンポーネントの生成と設定
	m_NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	RootComponent = m_NiagaraComp;
	m_NiagaraComp->SetAutoActivate(true);
}

//ゲーム開始時に呼ばれる処理
void AThunderTrailEffect::BeginPlay()
{
	Super::BeginPlay();

	//生成された瞬間の座標と回転を記録
	m_StartLocation = GetActorLocation();
	m_FixedRotation = GetActorRotation();

	//エフェクトは水平方向にのみ伸ばすためピッチとロールをリセット
	m_FixedRotation.Pitch = EffectConstants::ZeroValue;
	m_FixedRotation.Roll = EffectConstants::ZeroValue;
}

//毎フレーム呼ばれる処理
void AThunderTrailEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//フェードアウト処理
	if (m_IsFinished)
	{
		//現在のスケールを取得
		FVector CurrentScale = GetActorScale3D();

		//徐々に細くするための補間処理
		float NewThickness = FMath::FInterpTo(CurrentScale.Y, EffectConstants::ZeroValue, DeltaTime, m_FadeOutInterpSpeed);

		//新しいスケールを適用
		SetActorScale3D(FVector(CurrentScale.X, NewThickness, NewThickness));

		return;
	}

	//プレイヤーへの追従と伸長処理
	AActor* Parent = GetAttachParentActor();

	if (Parent)
	{
		//開始地点と開始アングルに固定
		SetActorLocation(m_StartLocation);
		SetActorRotation(m_FixedRotation);

		//現在の親(プレイヤー)の座標を取得
		FVector CurrentPlayerPos = Parent->GetActorLocation();

		//進行方向へのベクトルを計算
		FVector MoveVector = CurrentPlayerPos - m_StartLocation;

		//エフェクトの正面方向ベクトルを取得
		FVector ForwardVector = FRotationMatrix(m_FixedRotation).GetUnitAxis(EAxis::X);

		//エフェクトの正面方向成分(距離)を計算
		float ForwardDistance = FVector::DotProduct(MoveVector, ForwardVector);

		//後ろに下がった場合はマイナススケールにならないようゼロに制限
		ForwardDistance = FMath::Max(EffectConstants::ZeroValue, ForwardDistance);

		//新しいスケールを計算して適用
		FVector NewScale = FVector(ForwardDistance / m_BaseEffectLength, m_ThicknessScale, m_ThicknessScale);
		SetActorScale3D(NewScale);
	}
}

//エフェクトのフェードアウト開始処理
void AThunderTrailEffect::BeginFadeOut()
{
	//既にフェードアウト中なら処理を抜ける
	if (m_IsFinished) return;

	//フェードアウト中フラグを立てる
	m_IsFinished = true;

	//プレイヤーから切り離しその場に残留させる
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	//指定時間後に破棄するよう寿命を設定
	SetLifeSpan(m_FadeOutLifeSpan);
}
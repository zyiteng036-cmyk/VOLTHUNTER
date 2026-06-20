#include "PlayerSword.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"

//デフォルト値の設定とコンポーネント生成
APlayerSword::APlayerSword()
{
	//毎フレームTickを呼び出す設定(不要な場合はパフォーマンス向上のためfalseに設定可能)
	PrimaryActorTick.bCanEverTick = true;

	//剣のメッシュコンポーネントを生成してルートに設定
	m_SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	SetRootComponent(m_SwordMesh);

	//コリジョンを無効化
	m_SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//アタッチによる移動を許可するためMovableに設定
	m_SwordMesh->SetMobility(EComponentMobility::Movable);

	//トレイルエフェクトコンポーネントを生成してメッシュにアタッチ
	m_TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComponent"));
	m_TrailComponent->SetupAttachment(m_SwordMesh);

	//自動再生を無効化
	m_TrailComponent->SetAutoActivate(false);
}

//ゲーム開始時の初期設定
void APlayerSword::BeginPlay()
{
	Super::BeginPlay();

	//所有者であるキャラクターを取得
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		//所有者がキャラクターでない場合はエラーログを出力して処理を抜ける
		UE_LOG(LogTemp, Error, TEXT("PlayerSword has no Character owner"));
		return;
	}

	//キャラクターのスケルタルメッシュを取得
	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	if (!OwnerMesh)
	{
		return;
	}

	//指定されたソケットに対してサイズを維持したまま吸着アタッチ
	AttachToComponent(
		OwnerMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		m_AttachSocketName
	);
}

//攻撃状態に応じたトレイルエフェクトの制御
void APlayerSword::SetAttackActive(bool bActive)
{
	//必要なコンポーネントやアセットが有効でなければ処理しない
	if (!m_TrailComponent || !m_TrailNiagara)
	{
		return;
	}

	//初回呼び出し時のみエフェクトの設定とアタッチを行う
	if (!m_TrailInitialized)
	{
		//ナイアガラアセットをコンポーネントに設定
		m_TrailComponent->SetAsset(m_TrailNiagara);

		//メッシュのソケット構造等に合わせて再アタッチ
		m_TrailComponent->AttachToComponent(
			m_SwordMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale
		);

		//初期状態として一度非アクティブ化
		m_TrailComponent->Deactivate();

		//初期化完了フラグを更新
		m_TrailInitialized = true;

		//多重初期化の警戒用ログ
		UE_LOG(LogTemp, Error, TEXT("What?!SecondInitialized??"));
	}

	//アクティブ要求フラグに応じて再生・停止を切り替え
	if (bActive)
	{
		//エフェクトが未再生の場合のみアクティブ化
		if (!m_TrailComponent->IsActive())
		{
			m_TrailComponent->Activate();
		}
	}
	else
	{
		//エフェクトを非アクティブ化
		m_TrailComponent->Deactivate();
	}
}

//剣メッシュの表示状態を外部から設定
void APlayerSword::SetSwordMeshVisibility(bool bVisible)
{
	//メッシュコンポーネントが存在する場合のみ可視性を変更
	if (m_SwordMesh)
	{
		m_SwordMesh->SetVisibility(bVisible, false);
	}
}
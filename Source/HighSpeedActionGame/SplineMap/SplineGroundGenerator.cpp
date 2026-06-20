#include "SplineGroundGenerator.h"

ASplineGroundGenerator::ASplineGroundGenerator()
{
    // コンポーネントを作ってくっつける
    PrimaryActorTick.bCanEverTick = false; // 地面なのでTickは不要（軽くする）

    PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
    RootComponent = PathSpline;

    GroundMeshInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GroundMeshInstances"));
    GroundMeshInstances->SetupAttachment(RootComponent);

    // コリジョン（当たり判定）をONにする設定
    GroundMeshInstances->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
}

void ASplineGroundGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    //古いメッシュを全消去（これをしないと無限に増える）
    GroundMeshInstances->ClearInstances();

    if (!TileMesh) return; // メッシュが空っぽなら何もしない

    // メッシュをセット
    if (GroundMeshInstances->GetStaticMesh() != TileMesh)
    {
        GroundMeshInstances->SetStaticMesh(TileMesh);
    }

    //スプラインの長さを測って、縦に何個置けるか計算
    float TotalLength = PathSpline->GetSplineLength();
    int32 NumberOfRows = FMath::FloorToInt(TotalLength / TileSize);

    //ループ処理開始
    // i = 縦のループ（距離）
    for (int32 i = 0; i < NumberOfRows; i++)
    {
        float CurrentDistance = i * TileSize;

        // その地点の「座標(Location)」と「回転(Rotation)」と「右方向(RightVector)」を取得
        FVector CenterLocation = PathSpline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
        FRotator Rotation = PathSpline->GetRotationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
        FVector RightVector = PathSpline->GetRightVectorAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);

        // j = 横のループ（幅）
        for (int32 j = 0; j < NumberOfColumns; j++)
        {
            // 横へのズレ幅を計算
            float OffsetMultiplier = (float)j;

            // もし「中心合わせ」モードなら、真ん中に来るように計算をズラす
            if (bCenterOnSpline)
            {
                OffsetMultiplier = (float)j - (float)(NumberOfColumns - 1) * 0.5f;
            }

            // 最終的な場所を計算： 「中心点」 ＋ 「右方向ベクトル × ズレ幅 × タイルサイズ」
            FVector SpawnLocation = CenterLocation + (RightVector * OffsetMultiplier * TileSize);

            // 配置！ (World座標で指定)
            FTransform SpawnTransform(Rotation, SpawnLocation, TileScale);
            GroundMeshInstances->AddInstance(SpawnTransform, true);
        }
    }
}
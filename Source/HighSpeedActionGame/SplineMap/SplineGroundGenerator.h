//担当
//伊藤直樹

//スプラインを利用しStaticMeshを置きやすくするクラス

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "SplineGroundGenerator.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API ASplineGroundGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASplineGroundGenerator();

    // BPのConstruction Scriptにあたる関数
    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    // --- コンポーネント（部品） ---

    // 道のりを作るための線
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USplineComponent* PathSpline;

    // 大量配置するためのメッシュ管理コンポーネント
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* GroundMeshInstances;

    // --- 設定パラメータ（エディタでいじる用） ---

    // 配置したいブロックのメッシュ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Settings")
    UStaticMesh* TileMesh;

    // ブロック1個のサイズ（基本100.0f）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Settings")
    float TileSize = 100.0f;

    //メッシュの拡大率 (X, Y, Z)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Settings")
    FVector TileScale = FVector(1.0f, 1.0f, 1.0f);


    // 道の幅（何列並べるか）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Settings")
    int32 NumberOfColumns = 1;

    // 道の中心に合わせるか（チェックするとスプラインが真ん中に来る）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Settings")
    bool bCenterOnSpline = true;
};

//担当
//伊藤直樹

//プレイヤーに追尾する攻撃クラス

#pragma once

#include "CoreMinimal.h"
#include "../ProjectileBase.h"
#include "HomingProjectile.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API AHomingProjectile : public AProjectileBase
{
	GENERATED_BODY()
	
public:
	AHomingProjectile();

protected:
	virtual void BeginPlay()override;

	virtual void Tick(float DeltaTime) override;


	//追尾を開始する
	void StartHoming();

	bool CheckCollision();

protected:
	// 打ち上げから追尾開始までの待ち時間（秒）
	UPROPERTY(EditAnywhere, Category = "Homing Param")
	float m_LaunchDelay;

	// 最初に打ち上がる速度（上方向への勢い）
	UPROPERTY(EditAnywhere, Category = "Homing Param")
	float m_LaunchUpSpeed;

	// 打ち上がる際に、少しランダムに散らす角度
	UPROPERTY(EditAnywhere, Category = "Homing Param")
	float m_LaunchRandomSpread;

	// 追尾開始後の誘導性能（高いほど急旋回できる）
	UPROPERTY(EditAnywhere, Category = "Homing Param")
	float m_HomingMagnitude;

	// この距離より近づくと誘導をやめる
	UPROPERTY(EditAnywhere, Category = "Homing")
	float m_StopHomingDistance;

	// 前フレームの位置を記憶する
	FVector m_PrevLocation;

private:
	FTimerHandle m_HomingTimerHandle;

	// 既に何かに当たったかどうかのフラグ
	bool m_bHasHit;

	// フェードアウト用のタイマーハンドル
	FTimerHandle m_FadeOutTimerHandle;
};

//担当佐々木奏太
// 攻撃の衝突判定クラス

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "../DamageInfo.h"
#include "AttackCollisionDetection.generated.h"



class UStaticMeshComponent;
class UHitJudgmentComponent;

UCLASS()
class HIGHSPEEDACTIONGAME_API AAttackCollisionDetection : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAttackCollisionDetection();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	void SetActive(const bool _isActive);//アクティブセット
	bool GetIsActive()const;//アクティブかどうか
	void SetHitActorTags(const TArray<FString> _tags);//タグセット
	void SetRadius(const float _radius);//半径セット
	void SetLocation(const FVector& Location);//座標セット
	void SetDamage(const float _damage);//ダメージセット

	//衝突判定開始
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 可視化できるようにするメソッド
	void SetVisible(const bool _isvisible);

	void SetDamageInfo(const FDamageInfo& _damageInfo);//ダメージ情報セット
	void SetHitJudgmentComponent(const UHitJudgmentComponent* _hitJudgmentComponent);//使用している衝突判定クラス
protected:

	// 可視化用Mesh
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> m_VisualMesh;

	//衝突判定をとる球
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> m_HitSphere;

	//アクティブ
	UPROPERTY(VisibleAnywhere)
	bool m_IsActive;

	//表示非表示
	UPROPERTY(VisibleAnywhere)
	bool m_Visible;

	//今後どちらにも当たる攻撃が出てきたときように配列にしておく
	UPROPERTY(VisibleAnywhere)
	TArray<FString> m_HitActorTags;

	float m_Damage;

	FDamageInfo m_DamageInfo;

	const UHitJudgmentComponent* m_HitJudgmentComponent;

};

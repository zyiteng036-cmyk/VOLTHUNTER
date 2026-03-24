//佐々木奏太担当
//発射物基底クラス

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../AttackCollisionDetection/AttackCollisionParam.h"
#include "../DamageInfo.h"
#include "../DataAssets/AttackData.h"
#include "ProjectileBase.generated.h"

class UHitJudgmentComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
UCLASS()
class HIGHSPEEDACTIONGAME_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectileBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetAttackCollisionParam(const FAttackCollisionParam& _param);
	void SetDamageInfo(const FDamageInfo& _damageInfo);


	void OnCollisionBegin();
	void OnCollisionTick();
	void OnCollisionEnd();

	UFUNCTION()
	void HandlePlayerDeath(AActor* _actor);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TObjectPtr<UHitJudgmentComponent> m_HitJudgmentComponent;

	UPROPERTY(VisibleAnywhere)
	FAttackCollisionParam m_AttackCollisionParam;


	UPROPERTY(VisibleAnywhere)
	FDamageInfo m_DamageInfo;

	UPROPERTY(VisibleAnywhere, Category = Movement)
	TObjectPtr<UProjectileMovementComponent> m_ProjectileMovementComponent;

	// ヘッダーファイル
	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* m_NiagaraComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float m_TimeUntilDestroyed;

	float m_Timer;
};

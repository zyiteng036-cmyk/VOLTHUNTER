//佐々木奏太担当
//盾持ちの雑魚クラス

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "Enemy_GruntGuard.generated.h"


UCLASS()
class HIGHSPEEDACTIONGAME_API AEnemy_GruntGuard : public AEnemyBase
{
	GENERATED_BODY()

	AEnemy_GruntGuard();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void TakeDamage(const FDamageInfo& _damageInfo)override;

	void SetAIControllerIsActive(const bool _isActive)override;
	void SetIsPreparingAttack(const bool _isPreparingAttack) { m_IsPreparingAttack = _isPreparingAttack; }
private:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> m_GuardMontage;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float m_Timer;//汎用タイマー

	float m_AttackTimer;

	float m_PreparationForAttackTime;//攻撃準備時間

	float m_UntilTheAttack;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float m_AttackSpeed;

	bool m_AddOrSubFlug;
	FRotator m_StandardRot;

	FVector m_AttackVector;

	bool m_IsPreparingAttack;
};

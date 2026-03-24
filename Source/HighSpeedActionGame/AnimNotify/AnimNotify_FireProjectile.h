// 担当：佐々木奏太
//発射物発射アニメ通知クラス


#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "../AttackCollisionDetection/AttackCollisionParam.h"
#include "AnimNotify_FireProjectile.generated.h"

class AProjectileBase;
class UAttackData;
/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API UAnimNotify_FireProjectile : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AProjectileBase> m_Projectile;//発射物

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FName SocketName = NAME_None;//ソケットの名前

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackData")
	TObjectPtr<UAttackData> m_AttackData;//攻撃のデータ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackData")
	FAttackCollisionParam m_AttackCollisionParam;//攻撃用衝突判定のパラメーター
};

//担当
//伊藤直樹

//剣を管理するクラス

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerSword.generated.h"

class UStaticMeshComponent; 
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class HIGHSPEEDACTIONGAME_API APlayerSword : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerSword();

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//攻撃開始/終了
	void SetAttackActive(bool _Active);

	void SetSwordMeshVisibility(bool bVisible);
protected:
	// 刀メッシュ（StaticMesh）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> m_SwordMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Attach")
	FName AttachSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	TObjectPtr<UNiagaraSystem> m_TrailNiagara;

	// 内部管理用
	UPROPERTY(VisibleAnywhere, Category = "Trail")
	TObjectPtr<UNiagaraComponent> m_TrailComponent;

private:
	bool m_TrailInitialized;
};

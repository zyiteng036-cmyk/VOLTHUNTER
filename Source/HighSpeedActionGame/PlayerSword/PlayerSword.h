//担当
//伊藤直樹

//プレイヤーが装備し、攻撃時のエフェクト(トレイル)やメッシュの可視性を制御する武器クラス
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
	//アクターのデフォルト値を設定
	APlayerSword();

public:
	//ゲーム開始時またはスポン時に呼び出される初期化処理
	virtual void BeginPlay() override;

	//攻撃開始/終了によるエフェクトの制御
	void SetAttackActive(bool bActive);

	//剣メッシュの表示・非表示切り替え
	void SetSwordMeshVisibility(bool bVisible);

protected:
	//刀メッシュ(StaticMesh)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> m_SwordMesh = nullptr;

	//所有者へアタッチする際のソケット名
	UPROPERTY(EditDefaultsOnly, Category = "Attach")
	FName m_AttachSocketName = NAME_None;

	//武器の軌跡(トレイル)用演出エフェクトアセット
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail")
	TObjectPtr<UNiagaraSystem> m_TrailNiagara = nullptr;

	//内部管理用トレイルコンポーネント
	UPROPERTY(VisibleAnywhere, Category = "Trail")
	TObjectPtr<UNiagaraComponent> m_TrailComponent = nullptr;

private:
	//トレイルが初期化されたかどうかのフラグ
	bool m_TrailInitialized = false;
};
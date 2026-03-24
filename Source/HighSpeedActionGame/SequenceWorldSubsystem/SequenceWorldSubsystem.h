// 担当佐々木奏太
//シーケンス再生用のワールドサブシステム

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SequenceWorldSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossSequenceFinished);

/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API USequenceWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//ボスがアクティブになったときに呼ばれる
	void HandleOnBossActive();

	//ボスシーケンス再生用
	UFUNCTION()
	void PlayBossSeqence();

	//ボスシーケンス再生用デリゲード
	UPROPERTY(BlueprintAssignable, Category = "Sequence")
	FOnBossSequenceFinished OnBossSequenceFinished;

protected:
	//ボスシーケンス再生終了用メソッド
	UFUNCTION()
	void HandleBossSequenceFinished();

	bool m_HasPlayedBossCutscene = false;//ボスシーケンス再生したかどうか
};

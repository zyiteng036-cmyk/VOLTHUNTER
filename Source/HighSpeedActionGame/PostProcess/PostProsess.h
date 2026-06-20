//担当
//伊藤直樹

//特定のタグを持つアクターからポストプロセスコンポーネントを検索し、有効・無効を制御するワールドサブシステムクラス
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PostProsess.generated.h"

UCLASS()
class HIGHSPEEDACTIONGAME_API UPostProsess : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//外部（スキルなど）から呼ぶ関数
	void SetPostProsessActive(bool bActive);

	//対象となるポストプロセスアクターを検索またはキャッシュから取得
	AActor* FindPostProsessActor();

private:
	//Actorを見つけたらここに保存
	TWeakObjectPtr<AActor> m_CachedEffectActor = nullptr;
};
#include "PostProsess.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PostProcessComponent.h"
#include "GameFramework/Actor.h"

namespace PostProcessConstants
{
	//探すタグの名前を定数にしておく
	const FName EffectTagName = FName("PostProsess");
	//配列の先頭要素にアクセスするためのマジックナンバー排除用定数
	constexpr int32 FirstElementIndex = 0;
}

//外部からポストプロセスの有効・無効を切り替える
void UPostProsess::SetPostProsessActive(bool bActive)
{
	//Actorを取得（キャッシュにあればそれを使う）
	AActor* TargetActor = FindPostProsessActor();

	//対象のアクターが有効かチェック
	if (TargetActor)
	{
		//ポストプロセスコンポーネントを探して切り替える
		if (UPostProcessComponent* PP = TargetActor->FindComponentByClass<UPostProcessComponent>())
		{
			//コンポーネントの有効状態を設定
			PP->bEnabled = bActive;
		}
	}
}

//レベル内から対象のアクターを検索する
AActor* UPostProsess::FindPostProsessActor()
{
	//すでにキャッシュされている場合はキャッシュを返す
	if (m_CachedEffectActor.IsValid())
	{
		return m_CachedEffectActor.Get();
	}

	//ワールドインスタンスの有効性チェック
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	//タグを使ってレベル内を検索する
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, PostProcessConstants::EffectTagName, FoundActors);

	//アクターが見つかったかチェック
	if (!FoundActors.IsEmpty())
	{
		//見つかったらキャッシュする
		m_CachedEffectActor = FoundActors[PostProcessConstants::FirstElementIndex];
		return m_CachedEffectActor.Get();
	}

	//見つからなかった場合
	return nullptr;
}
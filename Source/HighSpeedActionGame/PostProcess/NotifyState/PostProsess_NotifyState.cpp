#include "PostProsess_NotifyState.h"
#include "../PostProsess.h"
#include "Engine/World.h"

//Notify開始時（バーの左端）の処理
void UPostProsess_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	//親クラスの処理を呼び出し
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	//メッシュコンポーネントが有効かチェック
	if (MeshComp)
	{
		//ワールドインスタンスを取得
		if (UWorld* World = MeshComp->GetWorld())
		{
			//サブシステムを取得して有効化
			if (UPostProsess* PostProsess = World->GetSubsystem<UPostProsess>())
			{
				PostProsess->SetPostProsessActive(true);
			}
		}
	}
}

//Notify終了時（バーの右端）の処理
void UPostProsess_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	//親クラスの処理を呼び出し
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	//メッシュコンポーネントが有効かチェック
	if (MeshComp)
	{
		//ワールドインスタンスを取得
		if (UWorld* World = MeshComp->GetWorld())
		{
			//サブシステムを取得して無効化
			if (UPostProsess* PostProsess = World->GetSubsystem<UPostProsess>())
			{
				PostProsess->SetPostProsessActive(false);
			}
		}
	}
}
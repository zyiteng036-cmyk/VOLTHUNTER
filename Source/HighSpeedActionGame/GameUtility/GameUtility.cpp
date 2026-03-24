//----------------------------------------------------------
// ファイル名		：GameUtility.cpp
// 概要				：どこからでも呼び出せるユーティリティクラス
// 作成者			：24CU0117 佐々木奏太
// 更新内容			：2025/07/25 作成
//----------------------------------------------------------

//インクルード
#include "GameUtility.h"
//#include "TestWidget.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Containers/UnrealString.h"
#include "Engine/World.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"

//タグからAActorを取得
AActor* CGameUtility::GetActorFromTag(UWorld* World, const FName& _tag) {
	if (World == nullptr) { return nullptr; }

	//全てのAActorから検索
	TSubclassOf<AActor> findClass;
	findClass = AActor::StaticClass();
	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(World, findClass, actors);

	//検索結果Actorがあれば
	if (actors.Num() > 0) {
		//その中のactorsを順番に検索
		for (int idx = 0; idx < actors.Num(); ++idx) {
			AActor* pActor = Cast<AActor>(actors[idx]);

			//タグ名で判別する
			if (pActor->ActorHasTag(_tag))
			{
				return pActor;
			}
		}
	}

	return nullptr;

}

float CGameUtility::GetFpsCorrection(float _deltatime) {
	//現在のFPSを測定
	float fps = 1.0f / _deltatime;

	//処理落ちしても一定の速度で処理するように補正
	return (60.f / fps);
}
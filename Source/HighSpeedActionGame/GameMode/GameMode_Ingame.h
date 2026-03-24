//佐々木奏太担当
//インゲーム用のゲームモード
//担当
//伊藤直樹


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "../Sound/GameBGMComponent.h"
#include "GameMode_Ingame.generated.h"


class APlayerCharacter;
class AResultPlayerController;
class UMediaPlayer;
class UMediaSource;
class ALevelSequenceActor;
/**
 *
 */
UCLASS()
class HIGHSPEEDACTIONGAME_API AGameMode_Ingame : public AGameModeBase
{
	GENERATED_BODY()
public:
	AGameMode_Ingame();

	//プレイヤー復帰
	void RespawnPlayer(APlayerCharacter* Player);

	//復帰地点更新
	void SetRespawnTransform(const FTransform& NewTransform);

	void RequestRespawn(APlayerCharacter* Player);

	UFUNCTION()
	void OnBossBattleStart();

	UFUNCTION()
	void OnGameClear();

	UFUNCTION()
	void PlayBossSequenceIfAvailable();

protected:
	//スタート時、生成時
	virtual void BeginPlay() override;

protected:
	//現在の復帰地点
	FTransform RespawnTransform;

	UPROPERTY(EditAnywhere, Category = "Clear")
	TSubclassOf<AActor> m_PostProcessActor;


	UPROPERTY(EditAnywhere, Category = "Clear")
	TSubclassOf<UUserWidget> m_ResultUserWidget;

	UPROPERTY(EditAnywhere, Category = "Clear")
	TSubclassOf<UUserWidget> m_PlayerWidgetClass;
	UPROPERTY()
	TObjectPtr<UUserWidget> m_CurrentWidgetInstance;

	UPROPERTY(EditAnywhere, Category = "Clear")
	TSubclassOf<AResultPlayerController> m_ResultControllerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	TObjectPtr<UMediaPlayer> m_MediaPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	TObjectPtr<UMediaSource> m_MediaSource;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGM")
	UGameBGMComponent* BGMManager;
};

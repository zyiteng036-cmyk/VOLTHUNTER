//ヒットストップコンポーネント
//佐々木奏太担当箇所


#include "HitStopComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

bool UHitStopComponent::m_IsUseHitStop = false;

// Sets default values for this component's properties
UHitStopComponent::UHitStopComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHitStopComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

//引数で渡した値の時間スローになる
void UHitStopComponent::StartHitStop(const float _customDuration) {
	UWorld* World = GetWorld();
	if (!World)return;

	//タイマー使用中なら既存タイマーリセットして秒数更新
	if (m_IsUseHitStop) {
		World->GetTimerManager().ClearTimer(m_HitStopTimerHandle);
	}
	else {
		// 現在の時間倍率を保存
		m_OriginalTimeDilation = UGameplayStatics::GetGlobalTimeDilation(this);
		// ヒットストップの時間倍率に設定
		UGameplayStatics::SetGlobalTimeDilation(this, m_HitStopTimeDilation);
		//ヒットストップ使用フラグ
		m_IsUseHitStop = true;
	}

	// 引数で渡された持続時間が正であればそちらを使い、そうでなければデフォルト値を使う
	float Duration = (_customDuration > 0.f) ? _customDuration : m_HitStopDuration;

	// 時間停止を続けるタイマーをセット//Durationの秒数になるように補正する
	World->GetTimerManager().SetTimer(m_HitStopTimerHandle, this, &UHitStopComponent::EndHitStop, Duration * m_HitStopTimeDilation, false);

}

bool UHitStopComponent::IsHitStopActive() const {
	if (GetWorld())
	{
		return GetWorld()->GetTimerManager().IsTimerActive(m_HitStopTimerHandle);
	}
	return false;
}


// ヒットストップ終了処理
void UHitStopComponent::EndHitStop() {
	m_IsUseHitStop = false;
	UGameplayStatics::SetGlobalTimeDilation(this, m_OriginalTimeDilation);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(m_HitStopTimerHandle);
	}
}


#pragma once

#include "Engine/EngineTypes.h"

//ジャスト回避用コリジョンチャンネル
constexpr ECollisionChannel JustEvasive_Collision = ECC_GameTraceChannel1;

//攻撃判定用コリジョンチャンネル
constexpr ECollisionChannel AttackDetection_Collision = ECC_GameTraceChannel2;

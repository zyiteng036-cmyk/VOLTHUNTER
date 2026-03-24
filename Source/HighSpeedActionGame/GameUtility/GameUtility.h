//佐々木奏太担当
//ユーティリティクラス


#pragma once

//前方宣言
class FString;
class UTestWidth;
class AActor;

class CGameUtility {
	//ユーティリティクラスのためメンバ関数はpublicに
public:
	//タグからActorを取得
	static AActor* GetActorFromTag(UWorld* World, const FName& _tag);

	//処理落ち補正値を取得
	static float GetFpsCorrection(float _delatTime);
};
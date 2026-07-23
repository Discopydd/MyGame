
#pragma once
#define DIRECTINPUT_VERSION 0x0800  
#include <dinput.h>
#include <Windows.h>
#include <wrl.h>
#include"../base/WinApp.h"
namespace MyEngine {

/// <summary>
/// Inputに関する処理と状態を管理するクラスです。
/// </summary>
class Input {
public:
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public: 
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="winApp">処理対象のオブジェクトへのポインタ。</param>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// Inputのインスタンスを生成します。
	/// </summary>
	Input(const Input&) = delete;
    /// <summary>
    /// 演算子「=」による計算結果を生成します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    Input& operator=(const Input&) = delete;

    /// <summary>
    /// Instanceを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    static Input* GetInstance();
	/// <summary>
	/// 入力や経過時間に応じて、状態を1フレーム分更新します。
	/// </summary>
	void Update();

	/// <summary>
	/// Push Key処理を実行します。
	/// </summary>
	/// <param name="keyNumber">処理に使用するkeyNumberの値。</param>
	/// <returns>判定結果。</returns>
	bool PushKey(BYTE keyNumber);

	/// <summary>
	/// Trigger Key処理を実行します。
	/// </summary>
	/// <param name="keyNumber">処理に使用するkeyNumberの値。</param>
	/// <returns>判定結果。</returns>
	bool TriggerKey(BYTE keyNumber);

	/// <summary>
	/// 使用しているリソースを解放し、終了処理を行います。
	/// </summary>
	void Finalize();
	/// <summary>
	/// All Keysを初期状態へ戻します。
	/// </summary>
	void ResetAllKeys();
private:
	ComPtr<IDirectInputDevice8> keyboard;
	ComPtr<IDirectInput8> directInput;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
	WinApp* winApp_ = nullptr;
	 /// <summary>
	 /// Inputのインスタンスを生成します。
	 /// </summary>
	 Input() = default;
    /// <summary>
    /// Inputが保持するリソースを破棄します。
    /// </summary>
    ~Input() = default;
};


} // namespace MyEngine

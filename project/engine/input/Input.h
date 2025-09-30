
#pragma once
#include <dinput.h>
#include <Windows.h>
#include <wrl.h>
#include<dinput.h>
#include"../base/WinApp.h"
class Input {
public:
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public: 
	void Initialize(WinApp* winApp);

	Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    static Input* GetInstance();
	void Update();

	bool PushKey(BYTE keyNumber);

	bool TriggerKey(BYTE keyNumber);

	void Finalize();
private:
	ComPtr<IDirectInputDevice8> keyboard;
	ComPtr<IDirectInput8> directInput;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
	WinApp* winApp_ = nullptr;
	 Input() = default;
    ~Input() = default;
};


#include"Input.h"
#include<cassert>


#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

Input* Input::GetInstance() {
    static Input instance;
    return &instance;
}
void Input::Initialize(WinApp* winApp)
{
	this->winApp_ = winApp;
	HRESULT result;
	
result = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
assert(SUCCEEDED(result));

result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
assert(SUCCEEDED(result));

result = keyboard->SetDataFormat(&c_dfDIKeyboard);
assert(SUCCEEDED(result));


result = keyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
assert(SUCCEEDED(result));

}

void Input::Update()
{
	memcpy(keyPre, key, sizeof(key));

	      keyboard->Acquire();

            
            keyboard->GetDeviceState(sizeof(key), key);

}

bool Input::PushKey(BYTE keyNumber)
{
	if (key[keyNumber]) {
		return true;
	}

	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	if (!keyPre[keyNumber] && key[keyNumber]) {
        return true;
    }

    return false;
}
void Input::Finalize() {
    if (keyboard) {
        keyboard->Unacquire();
        keyboard.Reset();
    }
    directInput.Reset();
    winApp_ = nullptr;
}
void Input::ResetAllKeys() {
    memset(key,    0, sizeof(key));
    memset(keyPre, 0, sizeof(keyPre));
}

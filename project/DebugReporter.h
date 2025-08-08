#pragma once
#include <wrl.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <d3d12sdklayers.h>

class DebugReporter {
public:
    DebugReporter() {

        HRESULT hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug_));
        if (FAILED(hr)) {
        
        }
    }

    ~DebugReporter() {
        if (debug_) {
            debug_->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
            debug_->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
            debug_->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
        }
    }

private:
    Microsoft::WRL::ComPtr<IDXGIDebug1> debug_;
};
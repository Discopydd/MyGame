#pragma once
#include <wrl.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <d3d12sdklayers.h>

namespace MyEngine {

/// <summary>
/// DebugReporterに関する処理と状態を管理するクラスです。
/// </summary>
class DebugReporter {
public:
    /// <summary>
    /// DebugReporterのインスタンスを生成します。
    /// </summary>
    DebugReporter() {

        HRESULT hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug_));
        if (FAILED(hr)) {
        
        }
    }

    /// <summary>
    /// DebugReporterが保持するリソースを破棄します。
    /// </summary>
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

} // namespace MyEngine

#define INITGUID
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

static ID3D11Device* dev;
static ID3D11DeviceContext* ctx;
static IDXGISwapChain* sc;
static ID3D11RenderTargetView* rtv;

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_KEYDOWN && w == VK_ESCAPE) PostQuitMessage(0);
    return DefWindowProcW(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int s) {
    WNDCLASSW wc = {0, WndProc, 0, 0, i, 0, 0, 0, 0, L"ICHxEngine-v0.0.0.1"};
    RegisterClassW(&wc);
    HWND h = CreateWindowExW(WS_EX_TOPMOST | WS_EX_APPWINDOW, L"ICHxEngine-v0.0.0.1", L"", WS_POPUP | WS_VISIBLE, 
                             0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0, 0, i, 0);
    
    DXGI_SWAP_CHAIN_DESC scd = {0};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = GetSystemMetrics(SM_CXSCREEN);
    scd.BufferDesc.Height = GetSystemMetrics(SM_CYSCREEN);
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = h;
    scd.SampleDesc.Count = 1;
    scd.Windowed = 1;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    
    D3D_FEATURE_LEVEL fl[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1};
    D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, fl, 2, D3D11_SDK_VERSION, &scd, &sc, &dev, 0, &ctx);
    
    ID3D11Texture2D* b;
    sc->lpVtbl->GetBuffer(sc, 0, &IID_ID3D11Texture2D, (void**)&b);
    dev->lpVtbl->CreateRenderTargetView(dev, (ID3D11Resource*)b, 0, &rtv);
    b->lpVtbl->Release(b);
    
    ctx->lpVtbl->OMSetRenderTargets(ctx, 1, &rtv, 0);
    
    float white[] = {1, 1, 1, 1};
    MSG m;
    while (1) {
        while (PeekMessageW(&m, 0, 0, 0, PM_REMOVE)) {
            if (m.message == WM_QUIT) goto cleanup;
            DispatchMessageW(&m);
        }
        ctx->lpVtbl->ClearRenderTargetView(ctx, rtv, white);
        sc->lpVtbl->Present(sc, 0, 0);
    }
    
cleanup:
    rtv->lpVtbl->Release(rtv);
    ctx->lpVtbl->Release(ctx);
    sc->lpVtbl->Release(sc);
    dev->lpVtbl->Release(dev);
    return 0;
}

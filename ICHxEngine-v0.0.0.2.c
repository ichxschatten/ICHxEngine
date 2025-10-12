#define INITGUID
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include "resources/planimetric_background_1024x600.c"

static LRESULT CALLBACK P(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_KEYDOWN && w == VK_ESCAPE) PostQuitMessage(0);
    return DefWindowProcW(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int s) {
    ID3D11Device* dev;
    ID3D11DeviceContext* ctx;
    IDXGISwapChain* swp;
    ID3D11RenderTargetView* rtv;
    ID3D11ShaderResourceView* srv;
    ID3D11SamplerState* smp;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11Texture2D* b;
    ID3DBlob* vb, *pb;

    DEVMODEW dm = { .dmSize = sizeof(dm), .dmPelsWidth = 1024, .dmPelsHeight = 600, .dmBitsPerPel = 32, .dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL };
    ChangeDisplaySettingsW(&dm, CDS_FULLSCREEN);

    WNDCLASSW wc = { .lpfnWndProc = P, .hInstance = i, .lpszClassName = L"ICHxEngine-v0.0.0.2" };
    RegisterClassW(&wc);
    HWND h = CreateWindowExW(WS_EX_TOPMOST, L"ICHxEngine-v0.0.0.2", 0, WS_POPUP | WS_VISIBLE, 0, 0, 1024, 600, 0, 0, i, 0);
    ShowCursor(0);

    DXGI_SWAP_CHAIN_DESC sd = {
        .BufferCount = 1,
        .BufferDesc = { 1024, 600, { 0, 0 }, DXGI_FORMAT_R8G8B8A8_UNORM },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .OutputWindow = h,
        .SampleDesc = { 1, 0 },
        .Windowed = FALSE
    };
    D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &sd, &swp, &dev, 0, &ctx);

    swp->lpVtbl->GetBuffer(swp, 0, &IID_ID3D11Texture2D, (void**)&b);
    dev->lpVtbl->CreateRenderTargetView(dev, (ID3D11Resource*)b, 0, &rtv);
    b->lpVtbl->Release(b);

    D3D11_TEXTURE2D_DESC td = {
        .Width = 1024, .Height = 600, .MipLevels = 1, .ArraySize = 1,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM, .SampleDesc = { 1, 0 },
        .Usage = D3D11_USAGE_IMMUTABLE, .BindFlags = D3D11_BIND_SHADER_RESOURCE
    };
    D3D11_SUBRESOURCE_DATA srd = { planimetric_background_bgrx_1024x600, 4096, 0 };
    dev->lpVtbl->CreateTexture2D(dev, &td, &srd, &b);
    dev->lpVtbl->CreateShaderResourceView(dev, (ID3D11Resource*)b, 0, &srv);
    b->lpVtbl->Release(b);

    D3D11_SAMPLER_DESC smd = {
        .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
        .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
        .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
        .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP
    };
    dev->lpVtbl->CreateSamplerState(dev, &smd, &smp);

    char vs_c[] = "struct O{float4 p:SV_POSITION;float2 u:TEXCOORD;};O main(uint i:SV_VertexID){O o;o.u=float2(i&1,i>>1);o.p=float4(o.u.x*2-1,1-o.u.y*2,0,1);return o;}";
    char ps_c[] = "Texture2D t:register(t0);SamplerState s:register(s0);float4 main(float4 p:SV_POSITION,float2 u:TEXCOORD):SV_Target{return t.Sample(s,u);}";
    
    D3DCompile(vs_c, sizeof(vs_c), 0, 0, 0, "main", "vs_4_0", 0, 0, &vb, 0);
    D3DCompile(ps_c, sizeof(ps_c), 0, 0, 0, "main", "ps_4_0", 0, 0, &pb, 0);
    dev->lpVtbl->CreateVertexShader(dev, vb->lpVtbl->GetBufferPointer(vb), vb->lpVtbl->GetBufferSize(vb), 0, &vs);
    dev->lpVtbl->CreatePixelShader(dev, pb->lpVtbl->GetBufferPointer(pb), pb->lpVtbl->GetBufferSize(pb), 0, &ps);
    vb->lpVtbl->Release(vb);
    pb->lpVtbl->Release(pb);

    D3D11_VIEWPORT vp = { 0, 0, 1024, 600, 0, 1 };
    ctx->lpVtbl->RSSetViewports(ctx, 1, &vp);
    ctx->lpVtbl->OMSetRenderTargets(ctx, 1, &rtv, 0);
    ctx->lpVtbl->IASetPrimitiveTopology(ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    ctx->lpVtbl->VSSetShader(ctx, vs, 0, 0);
    ctx->lpVtbl->PSSetShader(ctx, ps, 0, 0);
    ctx->lpVtbl->PSSetShaderResources(ctx, 0, 1, &srv);
    ctx->lpVtbl->PSSetSamplers(ctx, 0, 1, &smp);
    
    MSG m;
    while (1) {
        while (PeekMessageW(&m, 0, 0, 0, PM_REMOVE)) {
            if (m.message == WM_QUIT) {
                ChangeDisplaySettingsW(0, 0);
                return 0;
            }
            DispatchMessageW(&m);
        }
        ctx->lpVtbl->Draw(ctx, 4, 0);
        swp->lpVtbl->Present(swp, 0, 0);
    }
}

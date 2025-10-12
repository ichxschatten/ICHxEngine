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
#include "resources/planimetric_background_menu_768x450.c"

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_KEYDOWN && w == VK_ESCAPE) PostQuitMessage(0);
    return DefWindowProcW(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int s) {
    ID3D11Device* dev;
    ID3D11DeviceContext* ctx;
    IDXGISwapChain* swp;
    ID3D11RenderTargetView* rtv;
    ID3D11ShaderResourceView *srv_bg, *srv_menu;
    ID3D11SamplerState* smp;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11Texture2D* tex;
    ID3DBlob* blob;
    ID3D11Buffer *cb_bg, *cb_menu;
    ID3D11BlendState* blend;

    DEVMODEW dm = { .dmSize = sizeof(dm), .dmPelsWidth = 1024, .dmPelsHeight = 600, .dmBitsPerPel = 32, .dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL };
    ChangeDisplaySettingsW(&dm, CDS_FULLSCREEN);

    WNDCLASSW wc = { .lpfnWndProc = WndProc, .hInstance = i, .lpszClassName = L"ICHxEngine-v0.0.0.3" };
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(WS_EX_TOPMOST, L"ICHxEngine-v0.0.0.3", 0, WS_POPUP | WS_VISIBLE, 0, 0, 1024, 600, 0, 0, i, 0);
    ShowCursor(0);

    DXGI_SWAP_CHAIN_DESC sd = {
        .BufferCount = 1,
        .BufferDesc = {1024, 600, {0,0}, DXGI_FORMAT_R8G8B8A8_UNORM},
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .OutputWindow = hwnd,
        .SampleDesc = {1, 0},
        .Windowed = FALSE
    };
    D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &sd, &swp, &dev, 0, &ctx);

    swp->lpVtbl->GetBuffer(swp, 0, &IID_ID3D11Texture2D, (void**)&tex);
    dev->lpVtbl->CreateRenderTargetView(dev, (ID3D11Resource*)tex, 0, &rtv);
    tex->lpVtbl->Release(tex);

    D3D11_TEXTURE2D_DESC td = {
        .Width = 1024, .Height = 600, .MipLevels = 1, .ArraySize = 1,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM, .SampleDesc = {1, 0},
        .Usage = D3D11_USAGE_IMMUTABLE, .BindFlags = D3D11_BIND_SHADER_RESOURCE
    };
    D3D11_SUBRESOURCE_DATA srd = {planimetric_background_bgrx_1024x600, 4096, 0};
    dev->lpVtbl->CreateTexture2D(dev, &td, &srd, &tex);
    dev->lpVtbl->CreateShaderResourceView(dev, (ID3D11Resource*)tex, 0, &srv_bg);
    tex->lpVtbl->Release(tex);
    
    td.Width = 768; td.Height = 450;
    srd.pSysMem = planimetric_background_menu_bgra_768x450; srd.SysMemPitch = 3072;
    dev->lpVtbl->CreateTexture2D(dev, &td, &srd, &tex);
    dev->lpVtbl->CreateShaderResourceView(dev, (ID3D11Resource*)tex, 0, &srv_menu);
    tex->lpVtbl->Release(tex);

    D3D11_SAMPLER_DESC smd = {
        .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
        .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
        .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
        .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP
    };
    dev->lpVtbl->CreateSamplerState(dev, &smd, &smp);

    struct { float s[2]; float o[2]; } cbuf_bg = { {1.0f, 1.0f}, {0.0f, 0.0f} };
    struct { float s[2]; float o[2]; } cbuf_menu = { {768.0f / 1024.0f, 450.0f / 600.0f}, {0.0f, 0.0f} };

    D3D11_BUFFER_DESC cbd = {
        .ByteWidth = 16, .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_CONSTANT_BUFFER
    };
    srd.pSysMem = &cbuf_bg; srd.SysMemPitch = 0; srd.SysMemSlicePitch = 0;
    dev->lpVtbl->CreateBuffer(dev, &cbd, &srd, &cb_bg);
    srd.pSysMem = &cbuf_menu;
    dev->lpVtbl->CreateBuffer(dev, &cbd, &srd, &cb_menu);

    const char vs_code[] = 
        "cbuffer CBuf:register(b0){float2 s;float2 o;};"
        "struct VOut{float4 p:SV_POSITION;float2 u:TEXCOORD;};"
        "VOut main(uint i:SV_VertexID){"
        "VOut v;v.u=float2(i&1,i>>1);"
        "v.p=float4((v.u.x*2-1)*s.x+o.x,(1-v.u.y*2)*s.y+o.y,0,1);"
        "return v;}";
    
    const char ps_code[] = 
        "Texture2D t:register(t0);SamplerState s:register(s0);"
        "float4 main(float4 p:SV_POSITION,float2 u:TEXCOORD):SV_Target{"
        "return t.Sample(s,u);}";
    
    D3DCompile(vs_code, sizeof(vs_code), 0, 0, 0, "main", "vs_4_0", 0, 0, &blob, 0);
    dev->lpVtbl->CreateVertexShader(dev, blob->lpVtbl->GetBufferPointer(blob), blob->lpVtbl->GetBufferSize(blob), 0, &vs);
    blob->lpVtbl->Release(blob);
    
    D3DCompile(ps_code, sizeof(ps_code), 0, 0, 0, "main", "ps_4_0", 0, 0, &blob, 0);
    dev->lpVtbl->CreatePixelShader(dev, blob->lpVtbl->GetBufferPointer(blob), blob->lpVtbl->GetBufferSize(blob), 0, &ps);
    blob->lpVtbl->Release(blob);

    D3D11_BLEND_DESC bd = {
        .RenderTarget[0] = {
            .BlendEnable = TRUE,
            .SrcBlend = D3D11_BLEND_SRC_ALPHA,
            .DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
            .BlendOp = D3D11_BLEND_OP_ADD,
            .SrcBlendAlpha = D3D11_BLEND_ONE,
            .DestBlendAlpha = D3D11_BLEND_ZERO,
            .BlendOpAlpha = D3D11_BLEND_OP_ADD,
            .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
        }
    };
    dev->lpVtbl->CreateBlendState(dev, &bd, &blend);

    D3D11_VIEWPORT vp = {0, 0, 1024, 600, 0, 1};
    ctx->lpVtbl->RSSetViewports(ctx, 1, &vp);
    ctx->lpVtbl->OMSetRenderTargets(ctx, 1, &rtv, 0);
    ctx->lpVtbl->IASetPrimitiveTopology(ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    ctx->lpVtbl->VSSetShader(ctx, vs, 0, 0);
    ctx->lpVtbl->PSSetShader(ctx, ps, 0, 0);
    ctx->lpVtbl->PSSetSamplers(ctx, 0, 1, &smp);

    MSG msg;
    while (1) {
        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                ChangeDisplaySettingsW(0, 0);
                return 0;
            }
            DispatchMessageW(&msg);
        }

        float blend_factor[4] = {0,0,0,0};
        ctx->lpVtbl->OMSetBlendState(ctx, 0, blend_factor, 0xffffffff);
        ctx->lpVtbl->VSSetConstantBuffers(ctx, 0, 1, &cb_bg);
        ctx->lpVtbl->PSSetShaderResources(ctx, 0, 1, &srv_bg);
        ctx->lpVtbl->Draw(ctx, 4, 0);

        ctx->lpVtbl->OMSetBlendState(ctx, blend, blend_factor, 0xffffffff);
        ctx->lpVtbl->VSSetConstantBuffers(ctx, 0, 1, &cb_menu);
        ctx->lpVtbl->PSSetShaderResources(ctx, 0, 1, &srv_menu);
        ctx->lpVtbl->Draw(ctx, 4, 0);

        swp->lpVtbl->Present(swp, 0, 0);
    }
}

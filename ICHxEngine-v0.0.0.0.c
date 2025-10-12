#include <windows.h>

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_KEYDOWN && w == VK_ESCAPE) PostQuitMessage(0);
    return DefWindowProcW(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int s) {
    WNDCLASSW wc = {0, WndProc, 0, 0, i, 0, 0, 0, 0, L"ICHxEngine-v0.0.0.0"};
    RegisterClassW(&wc);
    CreateWindowExW(WS_EX_TOPMOST | WS_EX_APPWINDOW, L"ICHxEngine-v0.0.0.0", L"", WS_POPUP | WS_VISIBLE, 
                    0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0, 0, i, 0);
    MSG m;
    while (GetMessageW(&m, 0, 0, 0)) DispatchMessageW(&m);
    return 0;
}

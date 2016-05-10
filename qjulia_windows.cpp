//-----------------------------------------------------------------------------
#include "qjulia.h"
#include "qjulia.cpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
//-----------------------------------------------------------------------------
struct system_state_t
{
    HWND hwnd;
    HDC hdc, mdc;
    HBITMAP hbm;
};
//-----------------------------------------------------------------------------
static LRESULT CALLBACK
winproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_DESTROY:
        case WM_KEYDOWN:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(win, msg, wparam, lparam);
}

//-----------------------------------------------------------------------------
static double
sys_get_time()
{
    static LARGE_INTEGER freq = {};
    static LARGE_INTEGER counter0 = {};

    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&counter0);
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (counter.QuadPart - counter0.QuadPart) / (double)freq.QuadPart;
}
//-----------------------------------------------------------------------------
static void
sys_display_text(system_state_t *sys, const char *text)
{
    SetWindowText(sys->hwnd, text);
}
//-----------------------------------------------------------------------------
static int
win_init(application_state_t *app)
{
    system_state_t *sys = app->sys_state;

    WNDCLASS winclass = {};
    winclass.lpfnWndProc = winproc;
    winclass.hInstance = GetModuleHandle(NULL);
    winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    winclass.lpszClassName = k_app_name;
    if (!RegisterClass(&winclass)) return 0;

    RECT rect = { 0, 0, app->resolution[0], app->resolution[1] };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, FALSE))
        return 0;

    sys->hwnd = CreateWindow(k_app_name, k_app_name,
                             WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left, rect.bottom - rect.top,
                             NULL, NULL, NULL, 0);
    if (!sys->hwnd) return 0;
    if (!(sys->hdc = GetDC(sys->hwnd))) return 0;

    BITMAPINFO *bi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256);
    if (!bi) return 0;

    memset(bi, 0, sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256);
    bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi->bmiHeader.biPlanes = 1;
    bi->bmiHeader.biBitCount = 32;
    bi->bmiHeader.biCompression = BI_RGB;
    bi->bmiHeader.biWidth = app->resolution[0];
    bi->bmiHeader.biHeight = -app->resolution[1];
    bi->bmiHeader.biSizeImage = app->resolution[0] * app->resolution[1];
    sys->mdc = CreateCompatibleDC(sys->hdc);
    sys->hbm = CreateDIBSection(sys->hdc, bi, DIB_RGB_COLORS, (void **)&app->displayptr, NULL, 0);
    free(bi);

    if (!SelectObject(sys->mdc, sys->hbm)) return 0;
    return 1;
}
//-----------------------------------------------------------------------------
static void
win_deinit(system_state_t *sys)
{
    if (sys->hdc) {
        ReleaseDC(sys->hwnd, sys->hdc);
        sys->hdc = NULL;
    }
}
//-----------------------------------------------------------------------------
static void
win_update(application_state_t *app)
{
    BitBlt(app->sys_state->hdc, 0, 0, app->resolution[0], app->resolution[1],
           app->sys_state->mdc, 0, 0, SRCCOPY);
}
//-----------------------------------------------------------------------------
int
main(void)
{
    system_state_t sys_state = {};
    application_state_t app_state = {};
    app_state.resolution[0] = k_app_resx;
    app_state.resolution[1] = k_app_resy;
    app_state.sys_state = &sys_state;

    if (!win_init(&app_state)) {
        win_deinit(&sys_state);
        return 1;
    }

    MSG msg = { 0 };
    for (;;) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) break;
        } else {
            update(&app_state);
            win_update(&app_state);
        }
    }

    win_deinit(&sys_state);
    return 0;
}
//-----------------------------------------------------------------------------

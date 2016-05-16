﻿#include "qjulia.h"
#include "qjulia.cpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <process.h>

#define k_max_thread_count 16

struct thread_context_t
{
    HANDLE h;
    uint32_t id;
    HANDLE done_event;
    system_context_t *sys;
};

struct system_context_t
{
    HWND hwnd;
    HDC hdc, mdc;
    HBITMAP hbm;

    HANDLE render_semaphore;
    uint32_t thread_count;
    thread_context_t thread[k_max_thread_count];
};

static uint32_t WINAPI
render_thread(void *context)
{
    return 0;
}

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

static void
sys_display_text(system_context_t *sys, const char *text)
{
    assert(sys && text);
    SetWindowText(sys->hwnd, text);
}

static bool
win_init(application_context_t *app)
{
    assert(app && app->sys);
    system_context_t *sys = app->sys;

    WNDCLASS winclass = {};
    winclass.lpfnWndProc = winproc;
    winclass.hInstance = GetModuleHandle(NULL);
    winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    winclass.lpszClassName = k_app_name;
    if (!RegisterClass(&winclass)) return false;

    RECT rect = { 0, 0, app->resolution[0], app->resolution[1] };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, FALSE))
        return false;

    sys->hwnd = CreateWindow(k_app_name, k_app_name,
                             WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left, rect.bottom - rect.top,
                             NULL, NULL, NULL, 0);
    if (!sys->hwnd) return false;
    if (!(sys->hdc = GetDC(sys->hwnd))) return false;

    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biWidth = app->resolution[0];
    bi.bmiHeader.biHeight = -app->resolution[1];
    bi.bmiHeader.biSizeImage = app->resolution[0] * app->resolution[1];
    sys->mdc = CreateCompatibleDC(sys->hdc);
    sys->hbm = CreateDIBSection(sys->hdc, &bi, DIB_RGB_COLORS, (void **)&app->displayptr, NULL, 0);

    if (!SelectObject(sys->mdc, sys->hbm)) return false;

    sys->render_semaphore = CreateSemaphore(NULL, 0, sys->thread_count, NULL);
    if (!sys->render_semaphore) return false;

    for (uint32_t i = 0; i < sys->thread_count; ++i) {
        sys->thread[i].sys = sys;
        sys->thread[i].h = NULL;
        sys->thread[i].id = i;
        sys->thread[i].done_event = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
        if (!sys->thread[i].done_event) return false;
    }
    for (uint32_t i = 0; i < sys->thread_count; ++i) {
        sys->thread[i].h = (HANDLE)_beginthreadex(NULL, 0, render_thread,
                                                  &sys->thread[i], 0, NULL);
        if (!sys->thread[i].h) return false;
    }

    return true;
}

static void
win_deinit(system_context_t *sys)
{
    assert(sys);
    if (sys->hdc) {
        ReleaseDC(sys->hwnd, sys->hdc);
        sys->hdc = NULL;
    }
}

static void
win_update(application_context_t *app)
{
    assert(app && app->sys);
    BitBlt(app->sys->hdc, 0, 0, app->resolution[0], app->resolution[1],
           app->sys->mdc, 0, 0, SRCCOPY);
}

int
main()
{
    system_context_t sys = {};
    application_context_t app = {};
    app.resolution[0] = k_app_resx;
    app.resolution[1] = k_app_resy;
    app.sys = &sys;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    sys.thread_count = (uint32_t)si.dwNumberOfProcessors;

    if (!win_init(&app)) {
        win_deinit(&sys);
        return 1;
    }

    MSG msg = { 0 };
    for (;;) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) break;
        } else {
            update(&app);
            win_update(&app);
        }
    }

    win_deinit(&sys);
    return 0;
}

#include "qjulia.h"
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
    HANDLE start_event, done_event;
    application_context_t *app;
};

struct system_context_t
{
    HWND hwnd;
    HDC hdc, mdc;
    HBITMAP hbm;

    HANDLE quit_event;
    uint32_t thread_count;
    thread_context_t thread[k_max_thread_count];
    volatile long tile_index;
};

static void
win_render(application_context_t *app)
{
    assert(app && app->sys);
    for (;;) {
        uint32_t idx = (uint32_t)_InterlockedExchangeAdd(&app->sys->tile_index, 1);
        if (idx >= k_tile_count) break;
        render_tile(app, idx);
    }
}

static uint32_t WINAPI
win_render_thread(void *context)
{
    thread_context_t *thread = (thread_context_t *)context;
    assert(thread && thread->app && thread->app->sys);

    system_context_t *sys = thread->app->sys;

    HANDLE wait[2] = { sys->quit_event, thread->start_event };

    for (;;) {
        DWORD r = WaitForMultipleObjects(2, wait, FALSE, INFINITE);
        if (r == WAIT_OBJECT_0) break;

        win_render(thread->app);

        SetEvent(thread->done_event);
    }

    return 0;
}

static LRESULT CALLBACK
win_message_handler(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
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
    winclass.lpfnWndProc = win_message_handler;
    winclass.hInstance = GetModuleHandle(NULL);
    winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    winclass.lpszClassName = k_app_name;
    if (!RegisterClass(&winclass)) return false;

    RECT rect = { 0, 0, k_app_resx, k_app_resy };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
                          FALSE))
        return false;

    sys->hwnd = CreateWindow(k_app_name, k_app_name,
                             WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX |
                             WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left, rect.bottom - rect.top,
                             NULL, NULL, NULL, 0);
    if (!sys->hwnd) return false;
    if (!(sys->hdc = GetDC(sys->hwnd))) return false;

    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biWidth = k_app_resx;
    bi.bmiHeader.biHeight = -k_app_resy;
    bi.bmiHeader.biSizeImage = k_app_resx * k_app_resy;
    sys->mdc = CreateCompatibleDC(sys->hdc);
    sys->hbm = CreateDIBSection(sys->hdc, &bi, DIB_RGB_COLORS, (void **)&app->displayptr,
                                NULL, 0);

    if (!SelectObject(sys->mdc, sys->hbm)) return false;


    sys->quit_event = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
    if (!sys->quit_event) return false;

    for (uint32_t i = 0; i < sys->thread_count; ++i) {
        sys->thread[i].app = app;
        sys->thread[i].h = NULL;
        sys->thread[i].id = i;
        sys->thread[i].start_event = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
        sys->thread[i].done_event = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
        if (!sys->thread[i].done_event || !sys->thread[i].start_event) return false;
    }
    for (uint32_t i = 0; i < sys->thread_count; ++i) {
        sys->thread[i].h = (HANDLE)_beginthreadex(NULL, 0, win_render_thread,
                                                  &sys->thread[i], 0, NULL);
        if (!sys->thread[i].h) return false;
    }

    return true;
}

static void
win_deinit(system_context_t *sys)
{
    assert(sys);

    // terminate all worker threads
    SetEvent(sys->quit_event);
    HANDLE done[k_max_thread_count];
    for (uint32_t i = 0; i < sys->thread_count; ++i) done[i] = sys->thread[i].h;
    WaitForMultipleObjects(sys->thread_count, done, TRUE, INFINITE);
    for (uint32_t i = 0; i < sys->thread_count; ++i) CloseHandle(sys->thread[i].h);

    if (sys->hdc) {
        ReleaseDC(sys->hwnd, sys->hdc);
        sys->hdc = NULL;
    }
}

static void
win_update(application_context_t *app)
{
    assert(app && app->sys);
    system_context_t *sys = app->sys;

    _InterlockedExchange(&sys->tile_index, 0);

    HANDLE done[k_max_thread_count];
    for (uint32_t i = 0; i < sys->thread_count; ++i) {
        done[i] = sys->thread[i].done_event;
        SetEvent(sys->thread[i].start_event);
    }
    win_render(app);

    WaitForMultipleObjects(sys->thread_count, done, TRUE, INFINITE);

    BitBlt(sys->hdc, 0, 0, k_app_resx, k_app_resy, sys->mdc, 0, 0, SRCCOPY);
}

int
main()
{
    system_context_t sys = {};
    application_context_t app = {};
    app.sys = &sys;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    sys.thread_count = (uint32_t)(si.dwNumberOfProcessors - 1);

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

#include "qjulia.h"

struct vec3_t
{
    double x, y, z;
};
//-----------------------------------------------------------------------------
static inline vec3_t
operator+(vec3_t &v0, vec3_t &v1)
{
    vec3_t r;
    r.x = v0.x + v1.x;
    r.y = v0.y + v1.y;
    r.z = v0.z + v1.z;
    return r;
}
//-----------------------------------------------------------------------------
static void
update_frame_stats(application_state_t *app)
{
    static double prev_time = -1.0;
    static double prev_fps_time = 0.0;
    static int fps_frame = 0;

    if (prev_time < 0.0) {
        prev_time = sys_get_time();
        prev_fps_time = prev_time;
    }

    app->time = sys_get_time();
    app->time_delta = app->time - prev_time;
    prev_time = app->time;

    if ((app->time - prev_fps_time) >= 1.0) {
        double fps = fps_frame / (app->time - prev_fps_time);
        double us = (1.0 / fps) * 1000000.0;
        char text[256];
        snprintf(text, sizeof(text), "[%d fps  %d us] %s", (int)fps, (int)us, k_app_name);
        sys_display_text(app->sys_state, text);
        prev_fps_time = app->time;
        fps_frame = 0;
    }
    fps_frame++;
}
//-----------------------------------------------------------------------------
static void
update(application_state_t *app)
{
    vec3_t v0 = { 1.0, 2.0, 3.0 };
    vec3_t v1 = { 1.0, 2.0, 3.0 };
    vec3_t v2 = v0 + v1;

    update_frame_stats(app);
    memset(app->displayptr, 0xff, app->resolution[0] * app->resolution[1] * 4);
}
//-----------------------------------------------------------------------------

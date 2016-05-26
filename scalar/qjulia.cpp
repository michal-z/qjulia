static void
update_frame_stats(application_context_t *app)
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
        sys_display_text(app->sys, text);
        prev_fps_time = app->time;
        fps_frame = 0;
    }
    fps_frame++;
}

static void
update(application_context_t *app)
{
    update_frame_stats(app);
}

static void
render_tile(application_context_t *app, uint32_t tileid)
{
    size_t x0 = (tileid % k_tile_xcount) * k_tile_size;
    size_t y0 = (tileid / k_tile_xcount) * k_tile_size;
    size_t x1 = x0 + k_tile_size;
    size_t y1 = y0 + k_tile_size;

    for (size_t y = y0; y < y1; ++y) {
        for (size_t x = x0; x < x1; ++x) {
            size_t idx = (x + y * k_app_resx) * 4;
            app->displayptr[idx] = 0;
            app->displayptr[idx + 1] = 0;
            app->displayptr[idx + 2] = (unsigned char)(255.0 * sin((double)x));
            app->displayptr[idx + 3] = 255;
        }
    }
}

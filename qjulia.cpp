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

static void
update(application_state_t *app)
{
    update_frame_stats(app);

    for (size_t y = 0; y < app->resolution[1]; ++y) {
        for (size_t x = 0; x < app->resolution[0]; ++x) {
            size_t idx = (x + y * app->resolution[0]) * 4;
            app->displayptr[idx] = 0;
            app->displayptr[idx + 1] = 0;
            app->displayptr[idx + 2] = (unsigned char)(255.0 * sin((double)x));
            app->displayptr[idx + 3] = 255;
        }
    }
}

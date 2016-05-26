#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define k_app_name "qjulia"
#define k_app_resx 1024
#define k_app_resy 1024

#define k_tile_size 64
#define k_tile_xcount (k_app_resx / k_tile_size)
#define k_tile_ycount (k_app_resy / k_tile_size)
#define k_tile_count (k_tile_xcount * k_tile_ycount)

struct system_context_t;
struct application_context_t
{
    double time, time_delta;
    uint8_t *displayptr;
    system_context_t *sys;
};

double sys_get_time();
void sys_display_text(system_context_t *sys, const char *text);

#include "qjulia_math.h"

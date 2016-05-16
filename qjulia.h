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

struct system_context_t;
struct application_context_t
{
    int32_t resolution[2];
    double time, time_delta;
    uint8_t *displayptr;
    system_context_t *sys;
};

double sys_get_time();
void sys_display_text(system_context_t *sys, const char *text);

#include "qjulia_math.h"

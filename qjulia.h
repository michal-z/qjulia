#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define k_app_name "qjulia"
#define k_app_resx 1024
#define k_app_resy 1024

struct system_state_t;
struct application_state_t
{
    int resolution[2];
    double time, time_delta;
    unsigned char *displayptr;
    system_state_t *sys_state;
};

double sys_get_time();
void sys_display_text(system_state_t *sys, const char *text);

#include "qjulia_math.h"

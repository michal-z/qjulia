#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define k_app_name "qjulia"
#define k_app_resx 1024
#define k_app_resy 1024

struct system_state;
typedef struct system_state system_state_t;

typedef struct application_state
{
    int resolution[2];
    double time, time_delta;
    unsigned char *displayptr;
    system_state_t *sys_state;
} application_state_t;

double sys_get_time(void);
void sys_display_text(system_state_t *sys, const char *text);

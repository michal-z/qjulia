#pragma once

struct vec3_t
{
    double x, y, z;
};

static inline vec3_t
vec3_make(double x, double y, double z)
{
    vec3_t r = { x, y, z };
    return r;
}

static inline vec3_t
operator+(const vec3_t &v0, const vec3_t &v1)
{
    vec3_t r;
    r.x = v0.x + v1.x;
    r.y = v0.y + v1.y;
    r.z = v0.z + v1.z;
    return r;
}

static inline vec3_t
operator-(const vec3_t &v0, const vec3_t &v1)
{
    vec3_t r;
    r.x = v0.x - v1.x;
    r.y = v0.y - v1.y;
    r.z = v0.z - v1.z;
    return r;
}

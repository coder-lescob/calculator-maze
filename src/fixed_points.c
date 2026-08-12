#include "fixed_points.h"

#include <stdint.h>
#include <string.h>

inline fixed16_t float_to_fixed16(float x) {
    return (fixed16_t)(x * (UINT16_MAX - 1));
}

inline float fixed16_to_float(fixed16_t x) {
    return (float)x / (UINT16_MAX - 1);
}

inline fixed16_t fixed16_mul(fixed16_t a, fixed16_t b) {
    return (fixed16_t)(((int64_t)a * b) >> 16);
}

inline fixed16_t fixed16_div(fixed16_t a, fixed16_t b) {
    return (fixed16_t)(((int64_t)(a << 16) / b));
}
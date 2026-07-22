#ifndef FIXED_POINTS_H
#define FIXED_POINTS_H

#include <stdint.h>
#include <string.h>

typedef int32_t fixed16_t;

#define FRACTIONAL_PART_f16 (16)
#define FLOOR_f16(X) (X & 0xffff0000)
#define FRAC_f16(X) (X & 0xffff)

#define INT_TO_FIXED16(X) ((X) << FRACTIONAL_PART_f16)
#define FIXED16_TO_INT(X) ((X) >> FRACTIONAL_PART_f16)

/**
 * convert a number from float to fixed16
 */
fixed16_t float_to_fixed16(float x);

/**
 * convert number from fixed16 to float
 */
float fixed16_to_float(fixed16_t x);

/**
 * multiplie two fixed points the correct way
 */
fixed16_t fixed16_mul(fixed16_t a, fixed16_t b);

#endif
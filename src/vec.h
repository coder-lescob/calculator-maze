#ifndef VEC_H
#define VEC_H

typedef struct {
    float x, y;
} Vec2;

/**
 * add two vectors together
 */
Vec2 vec_add(Vec2 a, Vec2 b);

/**
 * subtracts two vectors together
 */
Vec2 vec_sub(Vec2 a, Vec2 b);

/**
 * scales a vector by a scalar (number)
 */
Vec2 vec_scale(float scalar, Vec2 v);

/**
 * divide a vector by it's length so that the length of the new vector is 1
 */
Vec2 normalize(Vec2 v);

/**
 * uses the pathagorean theorem to figure out the length of the vector v.
 * assumes that x and y basis vectors have the same length and are perpendicular
 */
float vec_length(Vec2 v);

#endif

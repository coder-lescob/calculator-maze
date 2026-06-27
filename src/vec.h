#ifndef VEC_H
#define VEC_H

typedef struct {
    float x, y;
} Vec2;

Vec2 vec_add(Vec2 a, Vec2 b);
Vec2 vec_sub(Vec2 a, Vec2 b);
Vec2 vec_scale(float scalar, Vec2 v);
Vec2 normalize(Vec2 v);

#endif
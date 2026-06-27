#include "vec.h"
#include <math.h>

Vec2 vec_add(Vec2 a, Vec2 b) {
    return (Vec2) { a.x + b.x, a.y + b.y };
}

Vec2 vec_sub(Vec2 a, Vec2 b) {
    return (Vec2) { a.x - b.x, a.y - b.y };
}

Vec2 vec_scale(float scalar, Vec2 v) {
    return (Vec2) { scalar * v.x, scalar * v.y };
}

Vec2 normalize(Vec2 v) {
    float len = sqrt(v.x * v.x + v.y * v.y);
    return (Vec2) { v.x / len, v.y / len };
}
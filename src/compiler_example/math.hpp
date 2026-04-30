#pragma once
#include <cmath>

// AI generated, don't feel like programming matrix maths

struct mat4 {
    float m[16];
};

// =======================
// Basic helpers
// =======================

inline mat4 mat4_identity() {
    mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

inline float deg_to_rad(float deg) {
    return deg * 3.1415926535f / 180.0f;
}

// =======================
// Matrix multiply: r = a * b
// =======================

inline mat4 mat4_mul(const mat4& a, const mat4& b) {
    mat4 r{};

    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            r.m[col*4 + row] =
                a.m[0*4 + row] * b.m[col*4 + 0] +
                a.m[1*4 + row] * b.m[col*4 + 1] +
                a.m[2*4 + row] * b.m[col*4 + 2] +
                a.m[3*4 + row] * b.m[col*4 + 3];
        }
    }

    return r;
}

// =======================
// Translation
// =======================

inline mat4 mat4_translate(float x, float y, float z) {
    mat4 r = mat4_identity();
    r.m[12] = x;
    r.m[13] = y;
    r.m[14] = z;
    return r;
}

// =======================
// Scaling
// =======================

inline mat4 mat4_scale(float x, float y, float z) {
    mat4 r{};
    r.m[0] = x;
    r.m[5] = y;
    r.m[10] = z;
    r.m[15] = 1.0f;
    return r;
}

// =======================
// Rotation X
// =======================

inline mat4 mat4_rotate_x(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    mat4 r = mat4_identity();
    r.m[5] = c;
    r.m[6] = s;
    r.m[9] = -s;
    r.m[10] = c;
    return r;
}

// =======================
// Rotation Y
// =======================

inline mat4 mat4_rotate_y(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    mat4 r = mat4_identity();
    r.m[0] = c;
    r.m[2] = -s;
    r.m[8] = s;
    r.m[10] = c;
    return r;
}

// =======================
// Rotation Z
// =======================

inline mat4 mat4_rotate_z(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    mat4 r = mat4_identity();
    r.m[0] = c;
    r.m[4] = -s;
    r.m[1] = s;
    r.m[5] = c;
    return r;
}

// =======================
// Perspective projection
// =======================

inline mat4 mat4_perspective(float fov_rad, float aspect, float znear, float zfar) {
    mat4 r{};
    float f = 1.0f / std::tan(fov_rad * 0.5f);

    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zfar + znear) / (znear - zfar);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zfar * znear) / (znear - zfar);

    return r;
}
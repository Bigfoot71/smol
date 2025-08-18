/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include <smol.h>

#include "./internal/sl__simd.h"

/* === Quaternion Functions === */

sl_quat_t sl_quat_from_euler(sl_vec3_t v)
{
    v.x *= 0.5f;
    v.y *= 0.5f;
    v.z *= 0.5f;

    float cp = cosf(v.x); // Pitch (X)
    float sp = sinf(v.x);
    float cy = cosf(v.y); // Yaw (Y)
    float sy = sinf(v.y);
    float cr = cosf(v.z); // Roll (Z)
    float sr = sinf(v.z);

    sl_quat_t q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    float len_sq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (len_sq < 1e-4f) return SL_QUAT_IDENTITY;

    float inv_len = 1.0f / sqrtf(len_sq);
    for (int i = 0; i < 4; ++i) {
        q.v[i] *= inv_len;
    }

    return q;
}

sl_vec3_t sl_quat_to_euler(sl_quat_t q)
{
    sl_vec3_t angles;

    // Roll (X axis rotation)
    float sinrCosp = 2.0f * (q.w * q.x + q.y * q.z);
    float cosrCosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    angles.x = atan2f(sinrCosp, cosrCosp);

    // Pitch (Y axis rotation)
    float sinp = 2.0f * (q.w * q.y - q.z * q.x);
    if (fabsf(sinp) >= 1.0f) {
        angles.y = copysignf(SL_PI * 0.5f, sinp); // use 90 degrees if out of range
    }
    else {
        angles.y = asinf(sinp);
    }

    // Yaw (Z axis rotation)
    float sinyCosp = 2.0f * (q.w * q.z + q.x * q.y);
    float cosyCosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    angles.z = atan2f(sinyCosp, cosyCosp);

    return angles;
}

sl_quat_t sl_quat_from_mat4(const sl_mat4_t* m)
{
    sl_quat_t q;
    float trace = m->m00 + m->m11 + m->m22;

    if (trace > 0.0f) {
        float s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (m->m21 - m->m12) * s;
        q.y = (m->m02 - m->m20) * s;
        q.z = (m->m10 - m->m01) * s;
    }
    else {
        if (m->m00 > m->m11 && m->m00 > m->m22) {
            float s = 2.0f * sqrtf(1.0f + m->m00 - m->m11 - m->m22);
            q.w = (m->m21 - m->m12) / s;
            q.x = 0.25f * s;
            q.y = (m->m01 + m->m10) / s;
            q.z = (m->m02 + m->m20) / s;
        }
        else if (m->m11 > m->m22) {
            float s = 2.0f * sqrtf(1.0f + m->m11 - m->m00 - m->m22);
            q.w = (m->m02 - m->m20) / s;
            q.x = (m->m01 + m->m10) / s;
            q.y = 0.25f * s;
            q.z = (m->m12 + m->m21) / s;
        }
        else {
            float s = 2.0f * sqrtf(1.0f + m->m22 - m->m00 - m->m11);
            q.w = (m->m10 - m->m01) / s;
            q.x = (m->m02 + m->m20) / s;
            q.y = (m->m12 + m->m21) / s;
            q.z = 0.25f * s;
        }
    }

    return q;
}

sl_mat4_t sl_quat_to_mat4(sl_quat_t q)
{
    sl_mat4_t result = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    float a2 = q.x * q.x;
    float b2 = q.y * q.y;
    float c2 = q.z * q.z;
    float ac = q.x * q.z;
    float ab = q.x * q.y;
    float bc = q.y * q.z;
    float ad = q.w * q.x;
    float bd = q.w * q.y;
    float cd = q.w * q.z;

    result.m00 = 1 - 2 * (b2 + c2);
    result.m01 = 2 * (ab + cd);
    result.m02 = 2 * (ac - bd);

    result.m10 = 2 * (ab - cd);
    result.m11 = 1 - 2 * (a2 + c2);
    result.m12 = 2 * (bc + ad);

    result.m20 = 2 * (ac + bd);
    result.m21 = 2 * (bc - ad);
    result.m22 = 1 - 2 * (a2 + b2);

    return result;
}

sl_quat_t sl_quat_lerp(sl_quat_t a, sl_quat_t b, float t)
{
    float dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;

    if (dot < 0.0f) {
        b.w = -b.w;
        b.x = -b.x;
        b.y = -b.y;
        b.z = -b.z;
    }

    for (int i = 0; i < 4; ++i) {
        a.v[i] += t * (b.v[i] - a.v[i]);
    }

    return sl_quat_normalize(a);
}

sl_quat_t sl_quat_slerp(sl_quat_t a, sl_quat_t b, float t)
{
    float dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;

    if (dot < 0.0f) {
        b.w = -b.w;
        b.x = -b.x;
        b.y = -b.y;
        b.z = -b.z;
        dot = -dot;
    }

    if (dot > 0.9995f) {
        for (int i = 0; i < 4; ++i) {
            a.v[i] += t * (b.v[i] - a.v[i]);
        }
        return sl_quat_normalize(a);
    }

    float th0 = acosf(dot);
    float th = th0 * t;
    float sin_th = sinf(th0);

    float w1 = cosf(th) - dot * sinf(th) / sin_th;
    float w2 = sinf(th) / sin_th;

    for (int i = 0; i < 4; ++i) {
        a.v[i] = w1 * a.v[i] + w2 * b.v[i];
    }

    return a;
}

/* === Matrix 4x4 Functions === */

sl_mat4_t sl_mat4_translate(sl_vec3_t v)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    result.m30 = v.x;
    result.m31 = v.y;
    result.m32 = v.z;

    return result;
}

sl_mat4_t sl_mat4_rotate(sl_vec3_t axis, float radians)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    float x = axis.x, y = axis.y, z = axis.z;
    float len_sq = x * x + y * y + z * z;

    if (len_sq != 1.0f && len_sq != 0.0f) {
        float inv_len = 1.0f / sqrtf(len_sq);
        x *= inv_len;
        y *= inv_len;
        z *= inv_len;
    }

    float sinres = sinf(radians);
    float cosres = cosf(radians);
    float t = 1.0f - cosres;

    result.m00 = x * x * t + cosres;
    result.m01 = y * x * t + z * sinres;
    result.m02 = z * x * t - y * sinres;

    result.m10 = x * y * t - z * sinres;
    result.m11 = y * y * t + cosres;
    result.m12 = z * y * t + x * sinres;

    result.m20 = x * z * t + y * sinres;
    result.m21 = y * z * t - x * sinres;
    result.m22 = z * z * t + cosres;

    return result;
}

sl_mat4_t sl_mat4_rotate_x(float radians)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    float c = cosf(radians);
    float s = sinf(radians);

    result.m11 = c;
    result.m12 = s;
    result.m21 = -s;
    result.m22 = c;

    return result;
}

sl_mat4_t sl_mat4_rotate_y(float radians)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    float c = cosf(radians);
    float s = sinf(radians);

    result.m00 = c;
    result.m02 = -s;
    result.m20 = s;
    result.m22 = c;

    return result;
}

sl_mat4_t sl_mat4_rotate_z(float radians)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    float c = cosf(radians);
    float s = sinf(radians);

    result.m00 = c;
    result.m01 = s;
    result.m10 = -s;
    result.m11 = c;

    return result;
}

sl_mat4_t sl_mat4_rotate_xyz(sl_vec3_t radians)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    float cz = cosf(-radians.z);
    float sz = sinf(-radians.z);
    float cy = cosf(-radians.y);
    float sy = sinf(-radians.y);
    float cx = cosf(-radians.x);
    float sx = sinf(-radians.x);

    result.m00 = cz * cy;
    result.m01 = (cz * sy * sx) - (sz * cx);
    result.m02 = (cz * sy * cx) + (sz * sx);

    result.m10 = sz * cy;
    result.m11 = (sz * sy * sx) + (cz * cx);
    result.m12 = (sz * sy * cx) - (cz * sx);

    result.m20 = -sy;
    result.m21 = cy * sx;
    result.m22 = cy * cx;

    return result;
}

sl_mat4_t sl_mat4_rotate_zyx(sl_vec3_t radians)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    float cz = cosf(radians.z);
    float sz = sinf(radians.z);
    float cy = cosf(radians.y);
    float sy = sinf(radians.y);
    float cx = cosf(radians.x);
    float sx = sinf(radians.x);

    result.m00 = cz * cy;
    result.m10 = cz * sy * sx - cx * sz;
    result.m20 = sz * sx + cz * cx * sy;

    result.m01 = cy * sz;
    result.m11 = cz * cx + sz * sy * sx;
    result.m21 = cx * sz * sy - cz * sx;

    result.m02 = -sy;
    result.m12 = cy * sx;
    result.m22 = cy * cx;

    return result;
}

sl_mat4_t sl_mat4_scale(sl_vec3_t scale)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    result.m00 = scale.x;
    result.m11 = scale.y;
    result.m22 = scale.z;

    return result;
}

sl_mat4_t sl_mat4_frustum(float left, float right, float bottom, float top, float znear, float zfar)
{
    sl_mat4_t result = { 0 };

    float rl = right - left;
    float tb = top - bottom;
    float fn = zfar - znear;

    result.m00 = (znear * 2.0f) / rl;
    result.m11 = (znear * 2.0f) / tb;

    result.m20 = (right + left) / rl;
    result.m21 = (top + bottom) / tb;
    result.m22 = -(zfar + znear) / fn;
    result.m23 = -1.0f;

    result.m32 = -(zfar * znear * 2.0f) / fn;

    return result;
}

sl_mat4_t sl_mat4_perspective(float fovy, float aspect, float znear, float zfar)
{
    sl_mat4_t result = { 0 };

    float top = znear * tanf(fovy * 0.5f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    float rl = right - left;
    float tb = top - bottom;
    float fn = zfar - znear;

    result.m00 = (znear * 2.0f) / rl;
    result.m11 = (znear * 2.0f) / tb;

    result.m20 = (right + left) / rl;
    result.m21 = (top + bottom) / tb;
    result.m22 = -(zfar + znear) / fn;
    result.m23 = -1.0f;

    result.m32 = -(zfar * znear * 2.0f) / fn;

    return result;
}

sl_mat4_t sl_mat4_ortho(float left, float right, float bottom, float top, float znear, float zfar)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    float rl = (right - left);
    float tb = (top - bottom);
    float fn = (zfar - znear);

    result.m00 = 2.0f / rl;
    result.m11 = 2.0f / tb;
    result.m22 = -2.0f / fn;

    result.m23 = 0.0f;
    result.m30 = -(left + right) / rl;
    result.m31 = -(top + bottom) / tb;

    result.m32 = -(zfar + znear) / fn;

    return result;
}

sl_mat4_t sl_mat4_look_at(sl_vec3_t eye, sl_vec3_t target, sl_vec3_t up)
{
    sl_mat4_t result = SL_MAT4_IDENTITY;

    float length = 0.0f;
    float inv_len = 0.0f;

    sl_vec3_t vz = {
        eye.x - target.x,
        eye.y - target.y,
        eye.z - target.z
    };

    sl_vec3_t v = { vz.x, vz.y, vz.z };
    length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length == 0.0f) length = 1.0f;
    inv_len = 1.0f / length;
    vz.x *= inv_len;
    vz.y *= inv_len;
    vz.z *= inv_len;

    sl_vec3_t vx = {
        up.y * vz.z - up.z * vz.y,
        up.z * vz.x - up.x * vz.z,
        up.x * vz.y - up.y * vz.x
    };

    for (int_fast8_t i = 0; i < 3; ++i) v.v[i] = vx.v[i];
    length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length == 0.0f) length = 1.0f;
    inv_len = 1.0f / length;
    vx.x *= inv_len;
    vx.y *= inv_len;
    vx.z *= inv_len;

    sl_vec3_t vy = {
        vz.y * vx.z - vz.z * vx.y,
        vz.z * vx.x - vz.x * vx.z,
        vz.x * vx.y - vz.y * vx.x
    };

    result.m00 = vx.x;
    result.m01 = vy.x;
    result.m02 = vz.x;

    result.m10 = vx.y;
    result.m11 = vy.y;
    result.m12 = vz.y;

    result.m20 = vx.z;
    result.m21 = vy.z;
    result.m22 = vz.z;

    result.m30 = -(vx.x * eye.x + vx.y * eye.y + vx.z * eye.z);
    result.m31 = -(vy.x * eye.x + vy.y * eye.y + vy.z * eye.z);
    result.m32 = -(vz.x * eye.x + vz.y * eye.y + vz.z * eye.z);

    return result;
}

float sl_mat4_determinant(const sl_mat4_t* mat)
{
    float result = 0.0f;

    float a00 = mat->m00, a01 = mat->m01, a02 = mat->m02, a03 = mat->m03;
    float a10 = mat->m10, a11 = mat->m11, a12 = mat->m12, a13 = mat->m13;
    float a20 = mat->m20, a21 = mat->m21, a22 = mat->m22, a23 = mat->m23;
    float a30 = mat->m30, a31 = mat->m31, a32 = mat->m32, a33 = mat->m33;

    result = a30 * a21 * a12 * a03 - a20 * a31 * a12 * a03 - a30 * a11 * a22 * a03 + a10 * a31 * a22 * a03 +
             a20 * a11 * a32 * a03 - a10 * a21 * a32 * a03 - a30 * a21 * a02 * a13 + a20 * a31 * a02 * a13 +
             a30 * a01 * a22 * a13 - a00 * a31 * a22 * a13 - a20 * a01 * a32 * a13 + a00 * a21 * a32 * a13 +
             a30 * a11 * a02 * a23 - a10 * a31 * a02 * a23 - a30 * a01 * a12 * a23 + a00 * a31 * a12 * a23 +
             a10 * a01 * a32 * a23 - a00 * a11 * a32 * a23 - a20 * a11 * a02 * a33 + a10 * a21 * a02 * a33 +
             a20 * a01 * a12 * a33 - a00 * a21 * a12 * a33 - a10 * a01 * a22 * a33 + a00 * a11 * a22 * a33;

    return result;
}

sl_mat4_t sl_mat4_transpose(const sl_mat4_t* mat)
{
    sl_mat4_t result;
    float* SL_RESTRICT R = result.a;
    const float* SL_RESTRICT M = mat->a;

#if defined(SL__HAS_SSE)

    __m128 row0 = _mm_loadu_ps(&M[0]);
    __m128 row1 = _mm_loadu_ps(&M[4]);
    __m128 row2 = _mm_loadu_ps(&M[8]);
    __m128 row3 = _mm_loadu_ps(&M[12]);

    _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

    _mm_storeu_ps(&R[0],  row0);
    _mm_storeu_ps(&R[4],  row1);
    _mm_storeu_ps(&R[8],  row2);
    _mm_storeu_ps(&R[12], row3);

#elif defined(SL__HAS_NEON)

    float32x4x2_t t0 = vtrnq_f32(vld1q_f32(&M[0]), vld1q_f32(&M[4]));
    float32x4x2_t t1 = vtrnq_f32(vld1q_f32(&M[8]), vld1q_f32(&M[12]));

    vst1q_f32(&R[0],  vcombine_f32(vget_low_f32(t0.val[0]), vget_low_f32(t1.val[0])));
    vst1q_f32(&R[4],  vcombine_f32(vget_low_f32(t0.val[1]), vget_low_f32(t1.val[1])));
    vst1q_f32(&R[8],  vcombine_f32(vget_high_f32(t0.val[0]), vget_high_f32(t1.val[0])));
    vst1q_f32(&R[12], vcombine_f32(vget_high_f32(t0.val[1]), vget_high_f32(t1.val[1])));

#else
    R[0]  = M[0];   R[1]  = M[4];   R[2]  = M[8];   R[3]  = M[12];
    R[4]  = M[1];   R[5]  = M[5];   R[6]  = M[9];   R[7]  = M[13];
    R[8]  = M[2];   R[9]  = M[6];   R[10] = M[10];  R[11] = M[14];
    R[12] = M[3];   R[13] = M[7];   R[14] = M[11];  R[15] = M[15];
#endif

    return result;
}

sl_mat4_t sl_mat4_inverse(const sl_mat4_t* mat)
{
    sl_mat4_t result = { 0 };

    float a00 = mat->m00, a01 = mat->m01, a02 = mat->m02, a03 = mat->m03;
    float a10 = mat->m10, a11 = mat->m11, a12 = mat->m12, a13 = mat->m13;
    float a20 = mat->m20, a21 = mat->m21, a22 = mat->m22, a23 = mat->m23;
    float a30 = mat->m30, a31 = mat->m31, a32 = mat->m32, a33 = mat->m33;

    float b00 = a00 * a11 - a01 * a10;
    float b01 = a00 * a12 - a02 * a10;
    float b02 = a00 * a13 - a03 * a10;
    float b03 = a01 * a12 - a02 * a11;
    float b04 = a01 * a13 - a03 * a11;
    float b05 = a02 * a13 - a03 * a12;
    float b06 = a20 * a31 - a21 * a30;
    float b07 = a20 * a32 - a22 * a30;
    float b08 = a20 * a33 - a23 * a30;
    float b09 = a21 * a32 - a22 * a31;
    float b10 = a21 * a33 - a23 * a31;
    float b11 = a22 * a33 - a23 * a32;

    float inv_det = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

    result.m00 = (a11 * b11 - a12 * b10 + a13 * b09) * inv_det;
    result.m01 = (-a01 * b11 + a02 * b10 - a03 * b09) * inv_det;
    result.m02 = (a31 * b05 - a32 * b04 + a33 * b03) * inv_det;
    result.m03 = (-a21 * b05 + a22 * b04 - a23 * b03) * inv_det;
    result.m10 = (-a10 * b11 + a12 * b08 - a13 * b07) * inv_det;
    result.m11 = (a00 * b11 - a02 * b08 + a03 * b07) * inv_det;
    result.m12 = (-a30 * b05 + a32 * b02 - a33 * b01) * inv_det;
    result.m13 = (a20 * b05 - a22 * b02 + a23 * b01) * inv_det;
    result.m20 = (a10 * b10 - a11 * b08 + a13 * b06) * inv_det;
    result.m21 = (-a00 * b10 + a01 * b08 - a03 * b06) * inv_det;
    result.m22 = (a30 * b04 - a31 * b02 + a33 * b00) * inv_det;
    result.m23 = (-a20 * b04 + a21 * b02 - a23 * b00) * inv_det;
    result.m30 = (-a10 * b09 + a11 * b07 - a12 * b06) * inv_det;
    result.m31 = (a00 * b09 - a01 * b07 + a02 * b06) * inv_det;
    result.m32 = (-a30 * b03 + a31 * b01 - a32 * b00) * inv_det;
    result.m33 = (a20 * b03 - a21 * b01 + a22 * b00) * inv_det;

    return result;
}

float sl_mat4_trace(const sl_mat4_t* mat)
{
    return mat->m00 + mat->m11 + mat->m22 + mat->m33;
}

sl_mat4_t sl_mat4_add(const sl_mat4_t* left, const sl_mat4_t* right)
{
    sl_mat4_t result;
    for (int_fast8_t i = 0; i < 16; ++i) {
        result.a[i] = left->a[i] + right->a[i];
    }
    return result;
}

sl_mat4_t sl_mat4_sub(const sl_mat4_t* left, const sl_mat4_t* right)
{
    sl_mat4_t result;
    for (int_fast8_t i = 0; i < 16; ++i) {
        result.a[i] = left->a[i] - right->a[i];
    }
    return result;
}

sl_mat4_t sl_mat4_mul(const sl_mat4_t* SL_RESTRICT left, const sl_mat4_t* SL_RESTRICT right)
{
    sl_mat4_t result;
    float* SL_RESTRICT R = result.a;
    const float* SL_RESTRICT A = left->a;
    const float* SL_RESTRICT B = right->a;

#if defined(SL__HAS_FMA_AVX)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai0 = _mm_broadcast_ss(&A[i * 4 + 0]);
        __m128 ai1 = _mm_broadcast_ss(&A[i * 4 + 1]);
        __m128 ai2 = _mm_broadcast_ss(&A[i * 4 + 2]);
        __m128 ai3 = _mm_broadcast_ss(&A[i * 4 + 3]);

        __m128 row = _mm_mul_ps(ai0, col0);
        row = _mm_fmadd_ps(ai1, col1, row);
        row = _mm_fmadd_ps(ai2, col2, row);
        row = _mm_fmadd_ps(ai3, col3, row);

        _mm_storeu_ps(&R[i * 4], row);
    }

#elif defined(SL__HAS_AVX)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai0 = _mm_broadcast_ss(&A[i * 4 + 0]);
        __m128 ai1 = _mm_broadcast_ss(&A[i * 4 + 1]);
        __m128 ai2 = _mm_broadcast_ss(&A[i * 4 + 2]);
        __m128 ai3 = _mm_broadcast_ss(&A[i * 4 + 3]);

        __m128 row = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(ai0, col0), _mm_mul_ps(ai1, col1)),
            _mm_add_ps(_mm_mul_ps(ai2, col2), _mm_mul_ps(ai3, col3))
        );

        _mm_storeu_ps(&R[i * 4], row);
    }

#elif defined(SL__HAS_SSE42)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai0 = _mm_set1_ps(A[i * 4 + 0]);
        __m128 ai1 = _mm_set1_ps(A[i * 4 + 1]);
        __m128 ai2 = _mm_set1_ps(A[i * 4 + 2]);
        __m128 ai3 = _mm_set1_ps(A[i * 4 + 3]);

        __m128 row = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(ai0, col0), _mm_mul_ps(ai1, col1)),
            _mm_add_ps(_mm_mul_ps(ai2, col2), _mm_mul_ps(ai3, col3))
        );

        _mm_storeu_ps(&R[i * 4], row);
    }

#elif defined(SL__HAS_SSE41)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai = _mm_loadu_ps(&A[i * 4]);

        R[i * 4 + 0] = _mm_cvtss_f32(_mm_dp_ps(ai, col0, 0xF1));
        R[i * 4 + 1] = _mm_cvtss_f32(_mm_dp_ps(ai, col1, 0xF1));
        R[i * 4 + 2] = _mm_cvtss_f32(_mm_dp_ps(ai, col2, 0xF1));
        R[i * 4 + 3] = _mm_cvtss_f32(_mm_dp_ps(ai, col3, 0xF1));
    }

#elif defined(SL__HAS_SSE)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai0 = _mm_set1_ps(A[i * 4 + 0]);
        __m128 ai1 = _mm_set1_ps(A[i * 4 + 1]);
        __m128 ai2 = _mm_set1_ps(A[i * 4 + 2]);
        __m128 ai3 = _mm_set1_ps(A[i * 4 + 3]);

        __m128 row = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(ai0, col0), _mm_mul_ps(ai1, col1)),
            _mm_add_ps(_mm_mul_ps(ai2, col2), _mm_mul_ps(ai3, col3))
        );

        _mm_storeu_ps(&R[i * 4], row);
    }

#elif defined(SL__HAS_NEON_FMA)

    float32x4_t col0 = vld1q_f32(&B[0]);
    float32x4_t col1 = vld1q_f32(&B[4]);
    float32x4_t col2 = vld1q_f32(&B[8]);
    float32x4_t col3 = vld1q_f32(&B[12]);

    for (int i = 0; i < 4; i++) {
        float32x4_t ai0 = vdupq_n_f32(A[i * 4 + 0]);
        float32x4_t ai1 = vdupq_n_f32(A[i * 4 + 1]);
        float32x4_t ai2 = vdupq_n_f32(A[i * 4 + 2]);
        float32x4_t ai3 = vdupq_n_f32(A[i * 4 + 3]);

        float32x4_t row = vmulq_f32(ai0, col0);
        row = vfmaq_f32(row, ai1, col1);  // FMA: row += ai1 * col1
        row = vfmaq_f32(row, ai2, col2);  // FMA: row += ai2 * col2
        row = vfmaq_f32(row, ai3, col3);  // FMA: row += ai3 * col3

        vst1q_f32(&R[i * 4], row);
    }

#elif defined(SL__HAS_NEON)

    float32x4_t col0 = vld1q_f32(&B[0]);
    float32x4_t col1 = vld1q_f32(&B[4]);
    float32x4_t col2 = vld1q_f32(&B[8]);
    float32x4_t col3 = vld1q_f32(&B[12]);

    for (int i = 0; i < 4; i++) {
        float32x4_t ai0 = vdupq_n_f32(A[i * 4 + 0]);
        float32x4_t ai1 = vdupq_n_f32(A[i * 4 + 1]);
        float32x4_t ai2 = vdupq_n_f32(A[i * 4 + 2]);
        float32x4_t ai3 = vdupq_n_f32(A[i * 4 + 3]);

        float32x4_t row = vmulq_f32(ai0, col0);
        row = vmlaq_f32(row, ai1, col1);  // FMA: row += ai1 * col1
        row = vmlaq_f32(row, ai2, col2);  // FMA: row += ai2 * col2
        row = vmlaq_f32(row, ai3, col3);  // FMA: row += ai3 * col3

        vst1q_f32(&R[i * 4], row);
    }

#else

    for (int i = 0; i < 4; i++) {
        const float ai0 = A[i * 4 + 0];
        const float ai1 = A[i * 4 + 1];
        const float ai2 = A[i * 4 + 2];
        const float ai3 = A[i * 4 + 3];

        result.a[i * 4 + 0] = ai0 * B[0]  + ai1 * B[4]  + ai2 * B[8]  + ai3 * B[12];
        result.a[i * 4 + 1] = ai0 * B[1]  + ai1 * B[5]  + ai2 * B[9]  + ai3 * B[13];
        result.a[i * 4 + 2] = ai0 * B[2]  + ai1 * B[6]  + ai2 * B[10] + ai3 * B[14];
        result.a[i * 4 + 3] = ai0 * B[3]  + ai1 * B[7]  + ai2 * B[11] + ai3 * B[15];
    }

#endif

    return result;
}

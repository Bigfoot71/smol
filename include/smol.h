#ifndef SL_H
#define SL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>

/* === Definitions === */

#if defined(_WIN32)
#   if defined(__TINYC__)
#       define __declspec(x) __attribute__((x))
#   endif
#   if defined(SL_BUILD_SHARED)
#       define SLAPI __declspec(dllexport)
#   elif defined(SL_USE_SHARED)
#       define SLAPI __declspec(dllimport)
#   endif
#else
#   if defined(SL_BUILD_SHARED)
#       define SLAPI __attribute__((visibility("default")))
#   endif
#endif

#ifndef SLAPI
#   define SLAPI extern
#endif

#ifndef SL_RESTRICT
#   if defined(__cplusplus) || defined(_MSC_VER)
#       define SL_RESTRICT __restrict
#   else
#       define SL_RESTRICT restrict
#   endif
#endif

/* === Enums === */

typedef uint64_t sl_flags_t;

#define SL_FLAG_FULLSCREEN              (1 <<  0)
#define SL_FLAG_WINDOW_OCCLUDED         (1 <<  1)
#define SL_FLAG_WINDOW_HIDDEN           (1 <<  2)
#define SL_FLAG_WINDOW_BORDERLESS       (1 <<  3)
#define SL_FLAG_WINDOW_RESIZABLE        (1 <<  4)
#define SL_FLAG_WINDOW_MINIMIZED        (1 <<  5)
#define SL_FLAG_WINDOW_MAXIMIZED        (1 <<  6)
#define SL_FLAG_WINDOW_TOPMOST          (1 <<  7)
#define SL_FLAG_WINDOW_TRANSPARENT      (1 <<  8)
#define SL_FLAG_WINDOW_NOT_FOCUSABLE    (1 <<  9)
#define SL_FLAG_MOUSE_GRABBED           (1 << 10)
#define SL_FLAG_MOUSE_CAPTURE           (1 << 11)
#define SL_FLAG_MOUSE_RELATIVE          (1 << 12)
#define SL_FLAG_MOUSE_FOCUS             (1 << 13)
#define SL_FLAG_INPUT_FOCUS             (1 << 14)
#define SL_FLAG_KEYBOARD_GRABBED        (1 << 15)
#define SL_FLAG_HIGH_PIXEL_DENSITY      (1 << 16)
#define SL_FLAG_MSAA_X4                 (1 << 17)

typedef enum sl_mouse_button {
    SL_MOUSE_BUTTON_LEFT = 1,
    SL_MOUSE_BUTTON_MIDDLE,
    SL_MOUSE_BUTTON_RIGHT,
    SL_MOUSE_BUTTON_X1,
    SL_MOUSE_BUTTON_X2
} sl_mouse_button_t;

typedef enum sl_key {
    SL_KEY_UNKNOWN = 0,
    SL_KEY_A = 4,
    SL_KEY_B = 5,
    SL_KEY_C = 6,
    SL_KEY_D = 7,
    SL_KEY_E = 8,
    SL_KEY_F = 9,
    SL_KEY_G = 10,
    SL_KEY_H = 11,
    SL_KEY_I = 12,
    SL_KEY_J = 13,
    SL_KEY_K = 14,
    SL_KEY_L = 15,
    SL_KEY_M = 16,
    SL_KEY_N = 17,
    SL_KEY_O = 18,
    SL_KEY_P = 19,
    SL_KEY_Q = 20,
    SL_KEY_R = 21,
    SL_KEY_S = 22,
    SL_KEY_T = 23,
    SL_KEY_U = 24,
    SL_KEY_V = 25,
    SL_KEY_W = 26,
    SL_KEY_X = 27,
    SL_KEY_Y = 28,
    SL_KEY_Z = 29,
    SL_KEY_1 = 30,
    SL_KEY_2 = 31,
    SL_KEY_3 = 32,
    SL_KEY_4 = 33,
    SL_KEY_5 = 34,
    SL_KEY_6 = 35,
    SL_KEY_7 = 36,
    SL_KEY_8 = 37,
    SL_KEY_9 = 38,
    SL_KEY_0 = 39,
    SL_KEY_RETURN = 40,
    SL_KEY_ESCAPE = 41,
    SL_KEY_BACKSPACE = 42,
    SL_KEY_TAB = 43,
    SL_KEY_SPACE = 44,
    SL_KEY_MINUS = 45,
    SL_KEY_EQUALS = 46,
    SL_KEY_LEFTBRACKET = 47,
    SL_KEY_RIGHTBRACKET = 48,
    SL_KEY_BACKSLASH = 49,
    SL_KEY_NONUSHASH = 50,
    SL_KEY_SEMICOLON = 51,
    SL_KEY_APOSTROPHE = 52,
    SL_KEY_GRAVE = 53,
    SL_KEY_COMMA = 54,
    SL_KEY_PERIOD = 55,
    SL_KEY_SLASH = 56,
    SL_KEY_CAPSLOCK = 57,
    SL_KEY_F1 = 58,
    SL_KEY_F2 = 59,
    SL_KEY_F3 = 60,
    SL_KEY_F4 = 61,
    SL_KEY_F5 = 62,
    SL_KEY_F6 = 63,
    SL_KEY_F7 = 64,
    SL_KEY_F8 = 65,
    SL_KEY_F9 = 66,
    SL_KEY_F10 = 67,
    SL_KEY_F11 = 68,
    SL_KEY_F12 = 69,
    SL_KEY_PRINTSCREEN = 70,
    SL_KEY_SCROLLLOCK = 71,
    SL_KEY_PAUSE = 72,
    SL_KEY_INSERT = 73,
    SL_KEY_HOME = 74,
    SL_KEY_PAGEUP = 75,
    SL_KEY_DELETE = 76,
    SL_KEY_END = 77,
    SL_KEY_PAGEDOWN = 78,
    SL_KEY_RIGHT = 79,
    SL_KEY_LEFT = 80,
    SL_KEY_DOWN = 81,
    SL_KEY_UP = 82,
    SL_KEY_NUMLOCKCLEAR = 83,
    SL_KEY_KP_DIVIDE = 84,
    SL_KEY_KP_MULTIPLY = 85,
    SL_KEY_KP_MINUS = 86,
    SL_KEY_KP_PLUS = 87,
    SL_KEY_KP_ENTER = 88,
    SL_KEY_KP_1 = 89,
    SL_KEY_KP_2 = 90,
    SL_KEY_KP_3 = 91,
    SL_KEY_KP_4 = 92,
    SL_KEY_KP_5 = 93,
    SL_KEY_KP_6 = 94,
    SL_KEY_KP_7 = 95,
    SL_KEY_KP_8 = 96,
    SL_KEY_KP_9 = 97,
    SL_KEY_KP_0 = 98,
    SL_KEY_KP_PERIOD = 99,
    SL_KEY_NONUSBACKSLASH = 100,
    SL_KEY_APPLICATION = 101,
    SL_KEY_POWER = 102,
    SL_KEY_KP_EQUALS = 103,
    SL_KEY_F13 = 104,
    SL_KEY_F14 = 105,
    SL_KEY_F15 = 106,
    SL_KEY_F16 = 107,
    SL_KEY_F17 = 108,
    SL_KEY_F18 = 109,
    SL_KEY_F19 = 110,
    SL_KEY_F20 = 111,
    SL_KEY_F21 = 112,
    SL_KEY_F22 = 113,
    SL_KEY_F23 = 114,
    SL_KEY_F24 = 115,
    SL_KEY_EXECUTE = 116,
    SL_KEY_HELP = 117,
    SL_KEY_MENU = 118,
    SL_KEY_SELECT = 119,
    SL_KEY_STOP = 120,
    SL_KEY_AGAIN = 121,
    SL_KEY_UNDO = 122,
    SL_KEY_CUT = 123,
    SL_KEY_COPY = 124,
    SL_KEY_PASTE = 125,
    SL_KEY_FIND = 126,
    SL_KEY_MUTE = 127,
    SL_KEY_VOLUMEUP = 128,
    SL_KEY_VOLUMEDOWN = 129,
    SL_KEY_KP_COMMA = 133,
    SL_KEY_KP_EQUALSAS400 = 134,
    SL_KEY_INTERNATIONAL1 = 135,
    SL_KEY_INTERNATIONAL2 = 136,
    SL_KEY_INTERNATIONAL3 = 137,
    SL_KEY_INTERNATIONAL4 = 138,
    SL_KEY_INTERNATIONAL5 = 139,
    SL_KEY_INTERNATIONAL6 = 140,
    SL_KEY_INTERNATIONAL7 = 141,
    SL_KEY_INTERNATIONAL8 = 142,
    SL_KEY_INTERNATIONAL9 = 143,
    SL_KEY_LANG1 = 144,
    SL_KEY_LANG2 = 145,
    SL_KEY_LANG3 = 146,
    SL_KEY_LANG4 = 147,
    SL_KEY_LANG5 = 148,
    SL_KEY_LANG6 = 149,
    SL_KEY_LANG7 = 150,
    SL_KEY_LANG8 = 151,
    SL_KEY_LANG9 = 152,
    SL_KEY_ALTERASE = 153,
    SL_KEY_SYSREQ = 154,
    SL_KEY_CANCEL = 155,
    SL_KEY_CLEAR = 156,
    SL_KEY_PRIOR = 157,
    SL_KEY_RETURN2 = 158,
    SL_KEY_SEPARATOR = 159,
    SL_KEY_OUT = 160,
    SL_KEY_OPER = 161,
    SL_KEY_CLEARAGAIN = 162,
    SL_KEY_CRSEL = 163,
    SL_KEY_EXSEL = 164,
    SL_KEY_KP_00 = 176,
    SL_KEY_KP_000 = 177,
    SL_KEY_THOUSANDSSEPARATOR = 178,
    SL_KEY_DECIMALSEPARATOR = 179,
    SL_KEY_CURRENCYUNIT = 180,
    SL_KEY_CURRENCYSUBUNIT = 181,
    SL_KEY_KP_LEFTPAREN = 182,
    SL_KEY_KP_RIGHTPAREN = 183,
    SL_KEY_KP_LEFTBRACE = 184,
    SL_KEY_KP_RIGHTBRACE = 185,
    SL_KEY_KP_TAB = 186,
    SL_KEY_KP_BACKSPACE = 187,
    SL_KEY_KP_A = 188,
    SL_KEY_KP_B = 189,
    SL_KEY_KP_C = 190,
    SL_KEY_KP_D = 191,
    SL_KEY_KP_E = 192,
    SL_KEY_KP_F = 193,
    SL_KEY_KP_XOR = 194,
    SL_KEY_KP_POWER = 195,
    SL_KEY_KP_PERCENT = 196,
    SL_KEY_KP_LESS = 197,
    SL_KEY_KP_GREATER = 198,
    SL_KEY_KP_AMPERSAND = 199,
    SL_KEY_KP_DBLAMPERSAND = 200,
    SL_KEY_KP_VERTICALBAR = 201,
    SL_KEY_KP_DBLVERTICALBAR = 202,
    SL_KEY_KP_COLON = 203,
    SL_KEY_KP_HASH = 204,
    SL_KEY_KP_SPACE = 205,
    SL_KEY_KP_AT = 206,
    SL_KEY_KP_EXCLAM = 207,
    SL_KEY_KP_MEMSTORE = 208,
    SL_KEY_KP_MEMRECALL = 209,
    SL_KEY_KP_MEMCLEAR = 210,
    SL_KEY_KP_MEMADD = 211,
    SL_KEY_KP_MEMSUBTRACT = 212,
    SL_KEY_KP_MEMMULTIPLY = 213,
    SL_KEY_KP_MEMDIVIDE = 214,
    SL_KEY_KP_PLUSMINUS = 215,
    SL_KEY_KP_CLEAR = 216,
    SL_KEY_KP_CLEARENTRY = 217,
    SL_KEY_KP_BINARY = 218,
    SL_KEY_KP_OCTAL = 219,
    SL_KEY_KP_DECIMAL = 220,
    SL_KEY_KP_HEXADECIMAL = 221,
    SL_KEY_LCTRL = 224,
    SL_KEY_LSHIFT = 225,
    SL_KEY_LALT = 226,
    SL_KEY_LGUI = 227,
    SL_KEY_RCTRL = 228,
    SL_KEY_RSHIFT = 229,
    SL_KEY_RALT = 230,
    SL_KEY_RGUI = 231,
    SL_KEY_MODE = 257,
    SL_KEY_SLEEP = 258,
    SL_KEY_WAKE = 259,
    SL_KEY_CHANNEL_INCREMENT = 260,
    SL_KEY_CHANNEL_DECREMENT = 261,
    SL_KEY_MEDIA_PLAY = 262,
    SL_KEY_MEDIA_PAUSE = 263,
    SL_KEY_MEDIA_RECORD = 264,
    SL_KEY_MEDIA_FAST_FORWARD = 265,
    SL_KEY_MEDIA_REWIND = 266,
    SL_KEY_MEDIA_NEXT_TRACK = 267,
    SL_KEY_MEDIA_PREVIOUS_TRACK = 268,
    SL_KEY_MEDIA_STOP = 269,
    SL_KEY_MEDIA_EJECT = 270,
    SL_KEY_MEDIA_PLAY_PAUSE = 271,
    SL_KEY_MEDIA_SELECT = 272,
    SL_KEY_AC_NEW = 273,
    SL_KEY_AC_OPEN = 274,
    SL_KEY_AC_CLOSE = 275,
    SL_KEY_AC_EXIT = 276,
    SL_KEY_AC_SAVE = 277,
    SL_KEY_AC_PRINT = 278,
    SL_KEY_AC_PROPERTIES = 279,
    SL_KEY_AC_SEARCH = 280,
    SL_KEY_AC_HOME = 281,
    SL_KEY_AC_BACK = 282,
    SL_KEY_AC_FORWARD = 283,
    SL_KEY_AC_STOP = 284,
    SL_KEY_AC_REFRESH = 285,
    SL_KEY_AC_BOOKMARKS = 286,
    SL_KEY_SOFTLEFT = 287,
    SL_KEY_SOFTRIGHT = 288,
    SL_KEY_CALL = 289,
    SL_KEY_ENDCALL = 290,
    SL_KEY_RESERVED = 400,
    SL_KEY_COUNT = 512
} sl_key_t;

typedef enum sl_log {
    SL_LOG_INVALID,
    SL_LOG_TRACE,
    SL_LOG_VERBOSE,
    SL_LOG_DEBUG,
    SL_LOG_INFO,
    SL_LOG_WARN,
    SL_LOG_ERROR,
    SL_LOG_FATAL,
    SL_LOG_COUNT
} sl_log_t;

typedef enum sl_pixel_format {
    SL_PIXEL_FORMAT_LUMINANCE8,
    SL_PIXEL_FORMAT_ALPHA8,
    SL_PIXEL_FORMAT_LUMINANCE_ALPHA8,
    SL_PIXEL_FORMAT_RGB8,
    SL_PIXEL_FORMAT_RGBA8
} sl_pixel_format_t;

typedef enum sl_filter_mode {
    SL_FILTER_NEAREST,
    SL_FILTER_BILINEAR,
    SL_FILTER_TRILINEAR
} sl_filter_mode_t;

typedef enum sl_wrap_mode {
    SL_WRAP_CLAMP,
    SL_WRAP_REPEAT
} sl_wrap_mode_t;

typedef enum sl_cull_mode {
    SL_CULL_NONE,
    SL_CULL_BACK,
    SL_CULL_FRONT
} sl_cull_mode_t;

typedef enum sl_blend_mode {
    SL_BLEND_OPAQUE,
    SL_BLEND_PREMUL,
    SL_BLEND_ALPHA,
    SL_BLEND_MUL,
    SL_BLEND_ADD
} sl_blend_mode_t;

typedef enum sl_stencil_func {
    SL_STENCIL_DISABLE = 0,
    SL_STENCIL_ALWAYS,
    SL_STENCIL_NEVER,
    SL_STENCIL_LESS,
    SL_STENCIL_LEQUAL,
    SL_STENCIL_EQUAL,
    SL_STENCIL_GEQUAL,
    SL_STENCIL_GREATER,
    SL_STENCIL_NOTEQUAL
} sl_stencil_func_t;

typedef enum sl_stencil_op {
    SL_STENCIL_KEEP,
    SL_STENCIL_ZERO,
    SL_STENCIL_REPLACE,
    SL_STENCIL_INCR,
    SL_STENCIL_DECR,
    SL_STENCIL_INVERT
} sl_stencil_op_t;

typedef enum sl_font_type {
    SL_FONT_BITMAP,             ///< Grayscale bitmap font (anti-aliased, default stb_truetype output)
    SL_FONT_PIXEL,              ///< Pixel-art style bitmap (grayscale thresholded to binary alpha)
    SL_FONT_SDF,                ///< Signed Distance Field (needs a custom shader)
} sl_font_type_t;

/* === Structures === */

typedef struct sl_app_desc {

    sl_flags_t flags;

    const char* name;
    const char* version;
    const char* identifier;

    struct {
        void*(*malloc)(size_t size);
        void*(*calloc)(size_t nmemb, size_t size);
        void*(*realloc)(void *mem, size_t size);
        void(*free)(void *mem);
    } memory;

} sl_app_desc_t;

typedef union sl_vec2 {
    struct { float x, y; };
    float v[2];
} sl_vec2_t;

typedef union sl_vec3 {
    struct { float x, y, z; };
    float v[3];
} sl_vec3_t;

typedef union sl_vec4 {
    struct { float x, y, z, w; };
    float v[4];
} sl_vec4_t;

typedef union sl_quat {
    struct { float w, x, y, z; };
    float v[4];
} sl_quat_t;

typedef union sl_mat4 {
    struct {
        float m00, m01, m02, m03;
        float m10, m11, m12, m13;
        float m20, m21, m22, m23;
        float m30, m31, m32, m33;
    };
    float v[4][4];
    float a[16];
} sl_mat4_t;

typedef struct sl_color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} sl_color_t;

typedef struct sl_vertex_2d {
    sl_vec2_t position;
    sl_vec2_t texcoord;
    sl_color_t color;
} sl_vertex_2d_t;

typedef struct sl_vertex_3d {
    sl_vec3_t position;
    sl_vec2_t texcoord;
    sl_vec3_t normal;
    sl_color_t color;
} sl_vertex_3d_t;

typedef struct sl_image {
    void* pixels;
    int w, h;
    sl_pixel_format_t format;
} sl_image_t;

/* === ID Types === */

typedef uint32_t sl_texture_id;
typedef uint32_t sl_canvas_id;
typedef uint32_t sl_shader_id;
typedef uint32_t sl_mesh_id;
typedef uint32_t sl_font_id;

typedef uint32_t sl_sound_id;
typedef uint32_t sl_music_id;

/* === Macros === */

#define SL_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SL_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SL_CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

#ifdef __cplusplus
#define SL_VEC2(x, y)                                       \
    sl_vec2 {                                               \
        static_cast<float>((x)),                            \
        static_cast<float>((y))                             \
    }
#define SL_VEC3(x, y, z)                                    \
    sl_vec3 {                                               \
        static_cast<float>((x)),                            \
        static_cast<float>((y)),                            \
        static_cast<float>((z))                             \
    }
#define SL_VEC4(x, y, z, w)                                 \
    sl_vec4 {                                               \
        static_cast<float>((x)),                            \
        static_cast<float>((y)),                            \
        static_cast<float>((z)),                            \
        static_cast<float>((w))                             \
    }
#define SL_QUAT(w, x, y, z)                                 \
    sl_quat {                                               \
        static_cast<float>((w)),                            \
        static_cast<float>((x)),                            \
        static_cast<float>((y)),                            \
        static_cast<float>((z))                             \
    }
#define SL_COLOR(r, g, b, a)                                \
    sl_color {                                              \
        static_cast<uint8_t>((r)),                          \
        static_cast<uint8_t>((g)),                          \
        static_cast<uint8_t>((b)),                          \
        static_cast<uint8_t>((a))                           \
    }
#define SL_VERTEX_2D(p, t, c)                               \
    sl_vertex_2d {                                          \
        static_cast<sl_vec2>((p)),                          \
        static_cast<sl_vec2>((t)),                          \
        static_cast<sl_color>((c))                          \
    }
#define SL_VERTEX_3D(p, t, n, c)                            \
    sl_vertex_3d {                                          \
        static_cast<sl_vec3>((p)),                          \
        static_cast<sl_vec2>((t)),                          \
        static_cast<sl_vec3>((n)),                          \
        static_cast<sl_color>((c))                          \
    }
#define SL_MAT4_T sl_mat4
#else
#define SL_VEC2(x, y) (sl_vec2_t) { (x), (y) }
#define SL_VEC3(x, y, z) (sl_vec3_t) { (x), (y), (z)  }
#define SL_VEC4(x, y, z, w) (sl_vec4_t) { (x), (y), (z), (w)  }
#define SL_QUAT(w, x, y, z) (sl_quat_t) { (w), (x), (y), (z)  }
#define SL_COLOR(r, g, b, a) (sl_color_t) { (r), (g), (b), (a) }
#define SL_VERTEX_2D(p, t, c) (sl_vertex_2d_t) { (p), (t), (c) }
#define SL_VERTEX_3D(p, t, n, c) (sl_vertex_3d_t) { (p), (t), (n), (c) }
#define SL_MAT4_T (sl_mat4_t)
#endif

/* === Constants === */

#define SL_PI 3.14159265358979323846
#define SL_TAU 2.0 * SL_PI

#define SL_DEG2RAD (SL_PI / 180.0)
#define SL_RAD2DEG (180.0 / PI)

#define SL_VEC2_ZERO        SL_VEC2( 0,  0)
#define SL_VEC2_ONE         SL_VEC2( 1,  1)
#define SL_VEC2_UP          SL_VEC2( 0,  1)
#define SL_VEC2_DOWN        SL_VEC2( 0, -1)
#define SL_VEC2_LEFT        SL_VEC2(-1,  0)
#define SL_VEC2_RIGHT       SL_VEC2( 1,  0)

#define SL_VEC3_ZERO        SL_VEC3( 0,  0,  0)
#define SL_VEC3_ONE         SL_VEC3( 1,  1,  1)
#define SL_VEC3_UP          SL_VEC3( 0,  1,  0)
#define SL_VEC3_DOWN        SL_VEC3( 0, -1,  0)
#define SL_VEC3_LEFT        SL_VEC3(-1,  0,  0)
#define SL_VEC3_RIGHT       SL_VEC3( 1,  0,  0)
#define SL_VEC3_FORWARD     SL_VEC3( 0,  0, -1)
#define SL_VEC3_BACK        SL_VEC3( 0,  0,  1)

#define SL_VEC4_ZERO        SL_VEC4( 0, 0, 0, 0)
#define SL_VEC4_ONE         SL_VEC4( 1, 1, 1, 1)

#define SL_QUAT_IDENTITY    SL_QUAT( 1, 0, 0, 0)

#define SL_WHITE            SL_COLOR(255, 255, 255, 255)
#define SL_BLACK            SL_COLOR(  0,   0,   0, 255)
#define SL_GRAY             SL_COLOR(128, 128, 128, 255)
#define SL_LIGHT_GRAY       SL_COLOR(192, 192, 192, 255)
#define SL_DARK_GRAY        SL_COLOR( 64,  64,  64, 255)
#define SL_RED              SL_COLOR(255,   0,   0, 255)
#define SL_GREEN            SL_COLOR(  0, 255,   0, 255)
#define SL_BLUE             SL_COLOR(  0,   0, 255, 255)
#define SL_YELLOW           SL_COLOR(255, 255,   0, 255)
#define SL_CYAN             SL_COLOR(  0, 255, 255, 255)
#define SL_MAGENTA          SL_COLOR(255,   0, 255, 255)
#define SL_ORANGE           SL_COLOR(255, 165,   0, 255)
#define SL_BROWN            SL_COLOR(165,  42,  42, 255)
#define SL_PURPLE           SL_COLOR(128,   0, 128, 255)
#define SL_PINK             SL_COLOR(255, 192, 203, 255)
#define SL_GOLD             SL_COLOR(255, 215,   0, 255)
#define SL_SILVER           SL_COLOR(192, 192, 192, 255)

#define SL_MAT4_IDENTITY        \
    SL_MAT4_T {                 \
        1.0f, 0.0f, 0.0f, 0.0f, \
        0.0f, 1.0f, 0.0f, 0.0f, \
        0.0f, 0.0f, 1.0f, 0.0f, \
        0.0f, 0.0f, 0.0f, 1.0f  \
    }

#if defined(__cplusplus)
extern "C" {
#endif

/* === Core Functions === */

/**
 * @defgroup Core Core Functions
 * @{
 */

/**
 * @brief Initialize the Smol library with a window.
 *
 * This function initializes the Smol library, creating a window with
 * the specified title, width, height, and configuration flags. Must be
 * called before any other Smol functions.
 *
 * @param title The title of the window. Must not be NULL.
 * @param w Width of the window in pixels.
 * @param h Height of the window in pixels.
 * @param flags Configuration flags for initialization (sl_flags_t). Can be zero.
 * @return Returns true if the library initialized successfully, false otherwise.
 *         If initialization fails, the reason will be logged.
 */
SLAPI bool sl_init(const char* title, int w, int h, sl_flags_t flags);

/**
 * @brief Initialize the Smol library with extended application description.
 *
 * This function initializes the Smol library with more detailed
 * information about your application. It creates a window and configures
 * the library according to the provided application description.
 *
 * @param title The title of the window. Must not be NULL.
 * @param w Width of the window in pixels.
 * @param h Height of the window in pixels.
 * @param desc Pointer to an sl_app_desc_t structure describing your application.
 *             Must not be NULL, but individual fields can be zero if not needed.
 * @return Returns true if Smol initialized successfully, false otherwise.
 *         If initialization fails, the reason will be written to the logs.
 *
 * @note The sl_app_desc_t structure itself must be valid (non-NULL).
 *       Fields that are not used can be set to zero.
 */
SLAPI bool sl_init_ex(const char* title, int w, int h, const sl_app_desc_t* desc);

/** Shut down the library and cleanup resources */
SLAPI void sl_quit(void);

/**
 * Run one iteration of the main loop
 * @return false if the program should close
 * @note Can be used directly in the main loop: `while (sl_frame_step())`
 */
SLAPI bool sl_frame_step(void);

/* --- Time --- */

/** Get the elapsed time since library initialization (in seconds) */
SLAPI double sl_time(void);

/** Get the time taken by the last frame (in seconds) */
SLAPI double sl_frame_time(void);

/** Set target frame rate (FPS) */
SLAPI void sl_frame_target(int fps);

/** Get current frame rate (FPS) */
SLAPI int sl_frame_per_second(void);

/* --- Display ---- */

/** Get display scaling factor */
SLAPI float sl_display_get_scale(void);

/** Get display DPI */
SLAPI float sl_display_get_dpi(void);

/** Get the current display index */
SLAPI int sl_display_get_index(void);

/** Get display size as a 2D vector (width, height) */
SLAPI sl_vec2_t sl_display_get_size(void);

/* --- Window --- */

/** Get the window title */
SLAPI const char* sl_window_get_title(void);

/** Set the window title */
SLAPI void sl_window_set_title(const char* title);

/** Get the window width */
SLAPI int sl_window_get_width(void);

/** Get the window height */
SLAPI int sl_window_get_height(void);

/** Get window size as a 2D vector (width, height) */
SLAPI sl_vec2_t sl_window_get_size(void);

/** Set window size */
SLAPI void sl_window_set_size(int w, int h);

/** Set minimum window size */
SLAPI void sl_window_set_min_size(int w, int h);

/** Set maximum window size */
SLAPI void sl_window_set_max_size(int w, int h);

/** Get window position as a 2D vector (x, y) */
SLAPI sl_vec2_t sl_window_get_position(void);

/** Set window position */
SLAPI void sl_window_set_position(int x, int y);

/** Check if the window is fullscreen */
SLAPI bool sl_window_is_fullscreen(void);

/** Enable or disable fullscreen mode */
SLAPI void sl_window_set_fullscreen(bool enabled);

/** Check if the window is resizable */
SLAPI bool sl_window_is_resizable(void);

/** Enable or disable window resizing */
SLAPI void sl_window_set_resizable(bool resizable);

/** Check if the window is visible */
SLAPI bool sl_window_is_visible(void);

/** Minimize the window */
SLAPI void sl_window_minimize(void);

/** Maximize the window */
SLAPI void sl_window_maximize(void);

/** Restore the window from minimized/maximized state */
SLAPI void sl_window_restore(void);

/** Show the window */
SLAPI void sl_window_show(void);

/** Hide the window */
SLAPI void sl_window_hide(void);

/** Check if the window has focus */
SLAPI bool sl_window_is_focused(void);

/** Focus the window */
SLAPI void sl_window_focus(void);

/** Check if the window has borders */
SLAPI bool sl_window_is_bordered(void);

/** Enable or disable window borders */
SLAPI void sl_window_set_bordered(bool bordered);

/* --- Cursor --- */

/** Check if cursor is grabbed */
SLAPI bool sl_cursor_is_grabbed(void);

/** Grab or release the cursor */
SLAPI void sl_cursor_grab(bool grab);

/** Show the cursor */
SLAPI void sl_cursor_show(void);

/** Hide the cursor */
SLAPI void sl_cursor_hide(void);

/** Check if cursor is visible */
SLAPI bool sl_cursor_is_visible(void);

/* --- Mouse --- */

/** Enable or disable mouse capture */
SLAPI void sl_mouse_capture(bool enabled);

/** Check if a mouse button is currently pressed */
SLAPI bool sl_mouse_button_is_pressed(sl_mouse_button_t button);

/** Check if a mouse button is currently released */
SLAPI bool sl_mouse_button_is_released(sl_mouse_button_t button);

/** Check if a mouse button was just pressed this frame */
SLAPI bool sl_mouse_button_just_pressed(sl_mouse_button_t button);

/** Check if a mouse button was just released this frame */
SLAPI bool sl_mouse_button_just_released(sl_mouse_button_t button);

/** Get current mouse position */
SLAPI sl_vec2_t sl_mouse_get_position(void);

/** Set mouse position */
SLAPI void sl_mouse_set_position(sl_vec2_t p);

/** Get mouse movement delta since last frame */
SLAPI sl_vec2_t sl_mouse_get_delta(void);

/** Get mouse wheel movement */
SLAPI sl_vec2_t sl_mouse_get_wheel(void);

/* --- Keyboard --- */

/** Check if a key is currently pressed */
SLAPI bool sl_key_is_pressed(sl_key_t key);

/** Check if a key is currently released */
SLAPI bool sl_key_is_released(sl_key_t key);

/** Check if a key was just pressed this frame */
SLAPI bool sl_key_just_pressed(sl_key_t key);

/** Check if a key was just released this frame */
SLAPI bool sl_key_just_released(sl_key_t key);

/** Generate a 2D vector from four directional keys */
SLAPI sl_vec2_t sl_key_vector(sl_key_t left, sl_key_t right, sl_key_t up, sl_key_t down);

/* --- Files --- */

/** Get base path of the executable */
SLAPI const char* sl_file_base_path(void);

/** Load binary file into memory */
SLAPI void* sl_file_load(const char* file_path, size_t* size);

/** Load text file into memory (null-terminated) */
SLAPI char* sl_file_load_text(const char* file_path);

/** Write binary data to file */
SLAPI bool sl_file_write(const char* file_path, const void* data, size_t size);

/** Write text data to file */
SLAPI bool sl_file_write_text(const char* file_path, const char* data, size_t size);

/* --- Logging --- */

/** Log a message with specified log level */
SLAPI void sl_log(sl_log_t log, const char* msg, ...);

/** Log a message with va_list for specified log level */
SLAPI void sl_log_v(sl_log_t log, const char* msg, va_list args);

/** Log a trace message */
SLAPI void sl_logt(const char* msg, ...);

/** Log a verbose message */
SLAPI void sl_logv(const char* msg, ...);

/** Log a debug message */
SLAPI void sl_logd(const char* msg, ...);

/** Log an info message */
SLAPI void sl_logi(const char* msg, ...);

/** Log a warning message */
SLAPI void sl_logw(const char* msg, ...);

/** Log an error message */
SLAPI void sl_loge(const char* msg, ...);

/** Log a fatal error message */
SLAPI void sl_logf(const char* msg, ...);

/* --- Memory --- */

/** 
 * Allocates a memory block of the given size. 
 * @note Internally calls SDL_malloc.
 */
SLAPI void* sl_malloc(size_t size);

/** 
 * Allocates and zero-initializes an array of nmemb elements of the given size. 
 * @note Internally calls SDL_calloc.
 */
SLAPI void* sl_calloc(size_t nmemb, size_t size);

/** 
 * Resizes the memory block pointed to by ptr to the new size. 
 * @note Internally calls SDL_realloc.
 */
SLAPI void* sl_realloc(void* ptr, size_t size);

/** 
 * Frees the memory block pointed to by ptr. 
 * @note Internally calls SDL_free.
 */
SLAPI void sl_free(void* ptr);

/** @} */ // end of Core

/* === Render Functions === */

/**
 * @defgroup Render Render Functions
 * @{
 */

/**
 * Set specific viewport dimensions
 * Automatically flushes the batch
 */
SLAPI void sl_render_viewport(int x, int y, int w, int h);

/**
 * Enable and set scissor rectangle
 * Pass (0,0,0,0) to disable scissor
 * Automatically flushes the batch
 */
SLAPI void sl_render_scissor(int x, int y, int w, int h);

/**
 * Set stencil function and operations
 * Use SL_STENCIL_DISABLE to disable stencil
 * For framebuffers, ensure depth is enabled
 * Automatically flushes the batch
 */
SLAPI void sl_render_stencil(sl_stencil_func_t func, int ref, uint32_t mask,
                             sl_stencil_op_t sfail, sl_stencil_op_t dpfail,
                             sl_stencil_op_t dppass);

/** Clear the screen or current canvas with specified color */
SLAPI void sl_render_clear(sl_color_t color);

/**
 * Flush the batch: upload data and draw
 * Useful for specific cases, e.g., updating a shader uniform between draws
 */
SLAPI void sl_render_flush(void);

/** Flush the batch and present the frame to the screen */
SLAPI void sl_render_present(void);

/** Enable or disable depth testing
 * Automatically flushes the batch
 */
SLAPI void sl_render_depth_test(bool enabled);

/** Enable or disable writing to the depth buffer
 * Automatically flushes the batch
 */
SLAPI void sl_render_depth_write(bool enabled);

/** Set the minimum and maximum depth values
 * Automatically flushes the batch
 */
SLAPI void sl_render_depth_range(float near, float far);

/** Set which faces to cull during rendering
 * Automatically flushes the batch
 */
SLAPI void sl_render_cull_face(sl_cull_mode_t cull);

/** Set the modulation color for the batch
 * Applied per-vertex, does not affect batching
 */
SLAPI void sl_render_color(sl_color_t color);

/** Bind a texture to a slot
 * Slot 0 is default for all shaders
 * Modifying slot 0 may trigger a new draw call for the next primitive
 * Other slots modify pipeline state immediately; manual flush may be needed
 */
SLAPI void sl_render_sampler(uint32_t slot, sl_texture_id texture);

/** Set the font for text rendering
 * Zero disables text rendering
 * Text draw calls are separate; group multiple texts with same font in one draw
 */
SLAPI void sl_render_font(sl_font_id font);

/** Set the shader to use
 * Zero applies default shader
 * May trigger a new draw call for the next primitive
 * Activates shader immediately for uniform updates
 */
SLAPI void sl_render_shader(sl_shader_id shader);

/** Set the blend mode
 * May trigger a new draw call for the next primitive
 */
SLAPI void sl_render_blend(sl_blend_mode_t blend);

/** Set the canvas to render to
 * Zero renders to screen
 * Automatically flushes the batch
 */
SLAPI void sl_render_canvas(sl_canvas_id canvas);

/**
 * Set integer uniforms
 * Does NOT trigger a new draw call; manual flush may be needed
 */
SLAPI void sl_render_uniform1i(int uniform, int32_t x);
SLAPI void sl_render_uniform2i(int uniform, int32_t x, int32_t y);
SLAPI void sl_render_uniform3i(int uniform, int32_t x, int32_t y, int32_t z);
SLAPI void sl_render_uniform4i(int uniform, int32_t x, int32_t y, int32_t z, int32_t w);

/**
 * Set float uniforms
 * Does NOT trigger a new draw call; manual flush may be needed
 */
SLAPI void sl_render_uniform1f(int uniform, float x);
SLAPI void sl_render_uniform2f(int uniform, float x, float y);
SLAPI void sl_render_uniform3f(int uniform, float x, float y, float z);
SLAPI void sl_render_uniform4f(int uniform, float x, float y, float z, float w);

/**
 * Set vector uniforms
 * Does NOT trigger a new draw call; manual flush may be needed
 */
SLAPI void sl_render_uniform_vec2(int uniform, const sl_vec2_t* v, int count);
SLAPI void sl_render_uniform_vec3(int uniform, const sl_vec3_t* v, int count);
SLAPI void sl_render_uniform_vec4(int uniform, const sl_vec4_t* v, int count);

/** Normalize color and set as vec3 uniform */
SLAPI void sl_render_uniform_color3(int uniform, sl_color_t color);

/** Normalize color and set as vec4 uniform */
SLAPI void sl_render_uniform_color4(int uniform, sl_color_t color);

/**
 * Set matrix uniforms
 * Does NOT trigger a new draw call; manual flush may be needed
 */
SLAPI void sl_render_uniform_mat2(int uniform, float* v, int count);
SLAPI void sl_render_uniform_mat3(int uniform, float* v, int count);
SLAPI void sl_render_uniform_mat4(int uniform, float* v, int count);

/**
 * Set projection matrix
 * NULL restores default orthographic projection
 * Automatically flushes the batch
 */
SLAPI void sl_render_projection(const sl_mat4_t* matrix);

/**
 * Set view matrix
 * NULL restores identity
 * Automatically flushes the batch
 */
SLAPI void sl_render_view(const sl_mat4_t* matrix);

/**
 * Push a new transform matrix onto the stack, inheriting the previous one.
 * Only affects CPU-side vertex transformation; does not impact texture or projection.
 * No draw call is created.
 */
SLAPI void sl_render_push(void);

/**
 * Pop the last transform matrix from the stack.
 * Only affects CPU-side vertex transformation; does not impact texture or projection.
 */
SLAPI void sl_render_pop(void);

/**
 * Reset the current transform to identity.
 * Subsequent vertex transformations start from this state.
 */
SLAPI void sl_render_identity(void);

/**
 * Apply translation to the current transform matrix.
 * @param v Translation vector
 */
SLAPI void sl_render_translate(sl_vec3_t v);

/**
 * Apply rotation to the current transform matrix.
 * @param v Rotation vector (Euler angles)
 */
SLAPI void sl_render_rotate(sl_vec3_t v);

/**
 * Apply scale to the current transform matrix.
 * @param v Scaling factors for each axis
 */
SLAPI void sl_render_scale(sl_vec3_t v);

/**
 * Apply a custom matrix to the current transform.
 * @param matrix Matrix to multiply with current transform
 */
SLAPI void sl_render_transform(const sl_mat4_t* matrix);

/** Reset texture transform to identity */
SLAPI void sl_render_texture_identity(void);

/** Apply translation to texture transform
 * @param v Translation vector for texture coordinates
 */
SLAPI void sl_render_texture_translate(sl_vec2_t v);

/** Apply rotation to texture transform
 * @param radians Rotation angle in radians for texture coordinates
 */
SLAPI void sl_render_texture_rotate(float radians);

/** Apply scaling to texture transform
 * @param v Scaling factors for texture coordinates
 */
SLAPI void sl_render_texture_scale(sl_vec2_t v);

/** Render triangles from array (triangle list)
 *  @param triangles Array of vertices, 3 vertices per triangle
 *  @param triangle_count Number of triangles to render
 */
SLAPI void sl_render_triangle_list(const sl_vertex_2d_t* triangles, int triangle_count);

/** Render connected triangles (triangle strip)
 *  @param vertices Array of vertices forming a strip
 *  @param count Total number of vertices
 */
SLAPI void sl_render_triangle_strip(const sl_vertex_2d_t* vertices, int count);

/** Render connected triangles forming a fan
 *  @param vertices Array of vertices, first vertex is the fan center
 *  @param count Total number of vertices
 */
SLAPI void sl_render_triangle_fan(const sl_vertex_2d_t* vertices, int count);

/** Render quads from array (quad list)
 *  @param quads Array of vertices, 4 vertices per quad
 *  @param quad_count Number of quads to render
 */
SLAPI void sl_render_quad_list(const sl_vertex_2d_t* quads, int quad_count);

/** Render connected quads (quad strip)
 *  @param vertices Array of vertices forming a strip
 *  @param count Total number of vertices
 */
SLAPI void sl_render_quad_strip(const sl_vertex_2d_t* vertices, int count);

/** Render connected quads forming a fan
 *  @param vertices Array of vertices, first vertex is the fan center
 *  @param count Total number of vertices
 */
SLAPI void sl_render_quad_fan(const sl_vertex_2d_t* vertices, int count);

/** Render lines from array (line list)
 *  Only works correctly in 2D
 *  @param lines Array of points, 2 points per line
 *  @param line_count Number of lines to render
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_line_list(const sl_vec2_t* lines, int line_count, float thickness);

/** Render connected lines (line strip)
 *  Only works correctly in 2D
 *  @param points Array of connected points
 *  @param count Number of points
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_line_strip(const sl_vec2_t* points, int count, float thickness);

/** Render closed line loop
 *  Only works correctly in 2D
 *  @param points Array of points forming a closed loop
 *  @param count Number of points
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_line_loop(const sl_vec2_t* points, int count, float thickness);

/** Render a single triangle
 *  @param p0 First vertex
 *  @param p1 Second vertex
 *  @param p2 Third vertex
 */
SLAPI void sl_render_triangle(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2);

/** Render triangle outline
 *  Only works correctly in 2D
 *  @param p0 First vertex
 *  @param p1 Second vertex
 *  @param p2 Third vertex
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_triangle_lines(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, float thickness);

/** Render a single quad
 *  @param p0 First vertex (typically top-left)
 *  @param p1 Second vertex (typically bottom-left)
 *  @param p2 Third vertex (typically bottom-right)
 *  @param p3 Fourth vertex (typically top-right)
 */
SLAPI void sl_render_quad(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, sl_vec2_t p3);

/** Render quad outline
 *  Only works correctly in 2D
 *  @param p0 First vertex
 *  @param p1 Second vertex
 *  @param p2 Third vertex
 *  @param p3 Fourth vertex
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_quad_lines(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, sl_vec2_t p3, float thickness);

/** Render rectangle
 *  @param x Left coordinate
 *  @param y Top coordinate
 *  @param w Width
 *  @param h Height
 */
SLAPI void sl_render_rectangle(float x, float y, float w, float h);

/** Render rectangle outline
 *  Only works correctly in 2D
 *  @param x Left coordinate
 *  @param y Top coordinate
 *  @param w Width
 *  @param h Height
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_rectangle_lines(float x, float y, float w, float h, float thickness);

/** Render rectangle with center, size, and rotation
 *  @param center Center point of the rectangle
 *  @param size Width and height as a vector
 *  @param rotation Rotation angle in radians
 */
SLAPI void sl_render_rectangle_ex(sl_vec2_t center, sl_vec2_t size, float rotation);

/** Render rotated rectangle outline
 *  Only works correctly in 2D
 *  @param center Center point of the rectangle
 *  @param size Width and height as a vector
 *  @param rotation Rotation angle in radians
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_rectangle_lines_ex(sl_vec2_t center, sl_vec2_t size, float rotation, float thickness);

/** Render rounded rectangle with configurable corner radius and quality
 *  @param x Left coordinate
 *  @param y Top coordinate
 *  @param w Width
 *  @param h Height
 *  @param radius Corner radius (automatically clamped to max possible)
 *  @param segments Number of segments per corner arc (higher = smoother)
 */
SLAPI void sl_render_rounded_rectangle(float x, float y, float w, float h, float radius, int segments);

/** Render rounded rectangle outline with configurable corner radius and quality
 *  Only works correctly in 2D
 *  @param x Left coordinate
 *  @param y Top coordinate
 *  @param w Width
 *  @param h Height
 *  @param radius Corner radius (automatically clamped to max possible)
 *  @param thickness Line thickness in pixels
 *  @param segments Number of segments per corner arc (higher = smoother)
 */
SLAPI void sl_render_rounded_rectangle_lines(float x, float y, float w, float h, float radius, float thickness, int segments);

/** Render rounded rectangle with center, size, rotation, and corner radius
 *  Uses tessellation with fixed 8 segments per corner (no segments parameter due to arc limitations)
 *  For custom segments, use basic functions with matrix transformations
 *  @param center Center point of the rectangle
 *  @param size Width and height as a vector
 *  @param rotation Rotation angle in radians
 *  @param radius Corner radius (automatically clamped to max possible)
 */
SLAPI void sl_render_rounded_rectangle_ex(sl_vec2_t center, sl_vec2_t size, float rotation, float radius);

/** Render rotated rounded rectangle outline
 *  Only works correctly in 2D
 *  Uses tessellation with fixed 8 segments per corner (no segments parameter due to arc limitations)
 *  For custom segments, use basic functions with matrix transformations
 *  @param center Center point of the rectangle
 *  @param size Width and height as a vector
 *  @param rotation Rotation angle in radians
 *  @param radius Corner radius (automatically clamped to max possible)
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_rounded_rectangle_lines_ex(sl_vec2_t center, sl_vec2_t size, float rotation, float radius, float thickness);

/** Render filled circle
 *  @param p Center position
 *  @param radius Circle radius
 *  @param segments Number of segments (higher = smoother circle)
 */
SLAPI void sl_render_circle(sl_vec2_t p, float radius, int segments);

/** Render circle outline
 *  Only works correctly in 2D
 *  @param p Center position
 *  @param radius Circle radius
 *  @param segments Number of segments (higher = smoother circle)
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_circle_lines(sl_vec2_t p, float radius, int segments, float thickness);

/** Render filled ellipse
 *  @param p Center position
 *  @param r Radii as vector (x = horizontal radius, y = vertical radius)
 *  @param segments Number of segments (higher = smoother ellipse)
 */
SLAPI void sl_render_ellipse(sl_vec2_t p, sl_vec2_t r, int segments);

/** Render ellipse outline
 *  Only works correctly in 2D
 *  @param p Center position
 *  @param r Radii as vector (x = horizontal radius, y = vertical radius)
 *  @param segments Number of segments (higher = smoother ellipse)
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_ellipse_lines(sl_vec2_t p, sl_vec2_t r, int segments, float thickness);

/** Render filled pie slice
 *  @param center Center position
 *  @param radius Pie slice radius
 *  @param start_angle Starting angle in radians
 *  @param end_angle Ending angle in radians
 *  @param segments Number of segments for the arc (higher = smoother)
 */
SLAPI void sl_render_pie_slice(sl_vec2_t center, float radius, float start_angle, float end_angle, int segments);

/** Render pie slice outline (includes radial lines to center)
 *  Only works correctly in 2D
 *  @param center Center position
 *  @param radius Pie slice radius
 *  @param start_angle Starting angle in radians
 *  @param end_angle Ending angle in radians
 *  @param segments Number of segments for the arc (higher = smoother)
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_pie_slice_lines(sl_vec2_t center, float radius, float start_angle, float end_angle, int segments, float thickness);

/** Render filled ring (donut shape)
 *  @param center Center position
 *  @param inner_radius Inner radius of the ring
 *  @param outer_radius Outer radius of the ring
 *  @param segments Number of segments (higher = smoother ring)
 */
SLAPI void sl_render_ring(sl_vec2_t center, float inner_radius, float outer_radius, int segments);

/** Render ring outline
 *  Only works correctly in 2D
 *  @param center Center position
 *  @param inner_radius Inner radius of the ring
 *  @param outer_radius Outer radius of the ring
 *  @param segments Number of segments (higher = smoother ring)
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_ring_lines(sl_vec2_t center, float inner_radius, float outer_radius, int segments, float thickness);

/** Render arc segment of a ring (partial donut)
 *  @param center Center position
 *  @param inner_radius Inner radius of the ring
 *  @param outer_radius Outer radius of the ring
 *  @param start_angle Starting angle in radians
 *  @param end_angle Ending angle in radians
 *  @param segments Number of segments for the arc (higher = smoother)
 */
SLAPI void sl_render_ring_arc(sl_vec2_t center, float inner_radius, float outer_radius, float start_angle, float end_angle, int segments);

/** Render outline of a ring arc
 *  Only works correctly in 2D
 *  @param center Center position
 *  @param inner_radius Inner radius of the ring
 *  @param outer_radius Outer radius of the ring
 *  @param start_angle Starting angle in radians
 *  @param end_angle Ending angle in radians
 *  @param segments Number of segments for the arc (higher = smoother)
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_ring_arc_lines(sl_vec2_t center, float inner_radius, float outer_radius, float start_angle, float end_angle, int segments, float thickness);

/** Render a line using quad-based rendering for thickness compatibility and batching
 *  Thickness is projection-relative, works correctly in 2D only
 *  For 3D lines, use sl_render_mesh_lines instead
 *  This limitation applies to all line rendering functions except meshes
 *  @param p0 Start point
 *  @param p1 End point
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_line(sl_vec2_t p0, sl_vec2_t p1, float thickness);

/** Render an arc
 *  Only works correctly in 2D
 *  @param center Center position
 *  @param radius Arc radius
 *  @param start_angle Starting angle in radians
 *  @param end_angle Ending angle in radians
 *  @param thickness Line thickness in pixels
 *  @param segments Number of segments (higher = smoother arc)
 */
SLAPI void sl_render_arc(sl_vec2_t center, float radius, float start_angle, float end_angle, float thickness, int segments);

/** Render quadratic Bézier curve
 *  Only works correctly in 2D
 *  @param p0 Start point
 *  @param p1 Control point
 *  @param p2 End point
 *  @param segments Number of line segments to approximate the curve
 */
SLAPI void sl_render_bezier_quad(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, int segments);

/** Render cubic Bézier curve
 *  Only works correctly in 2D
 *  @param p0 Start point
 *  @param p1 First control point
 *  @param p2 Second control point
 *  @param p3 End point
 *  @param segments Number of line segments to approximate the curve
 */
SLAPI void sl_render_bezier_cubic(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, sl_vec2_t p3, int segments);

/** Render spline curve
 *  Only works correctly in 2D
 *  @param points Array of control points
 *  @param count Number of control points
 *  @param segments Number of line segments between each pair of points
 */
SLAPI void sl_render_spline(const sl_vec2_t* points, int count, int segments);

/** Render cross symbol at given center
 *  @param center Center position of the cross
 *  @param size Half-length of cross arms
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_cross(sl_vec2_t center, float size, float thickness);

/** Render grid lines
 *  Only works correctly in 2D
 *  @param x Left coordinate of the grid
 *  @param y Top coordinate of the grid
 *  @param w Total width of the grid
 *  @param h Total height of the grid
 *  @param cols Number of columns
 *  @param rows Number of rows
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_grid(float x, float y, float w, float h, int cols, int rows, float thickness);

/** Render arrow from one point to another
 *  @param from Starting point
 *  @param to Ending point (arrow head location)
 *  @param head_size Size of the arrow head
 *  @param thickness Line thickness in pixels
 */
SLAPI void sl_render_arrow(sl_vec2_t from, sl_vec2_t to, float head_size, float thickness);

/** Render star shape
 *  @param center Center position
 *  @param outer_radius Distance from center to star points
 *  @param inner_radius Distance from center to star indentations
 *  @param points Number of star points (minimum 3)
 */
SLAPI void sl_render_star(sl_vec2_t center, float outer_radius, float inner_radius, int points);

/** Render a single codepoint (character)
 *  @param codepoint Unicode codepoint to render
 *  @param position Base position for the character
 *  @param font_size Font size in pixels
 */
SLAPI void sl_render_codepoint(int codepoint, sl_vec2_t position, float font_size);

/** Render multiple codepoints sequentially
 *  @param codepoints Array of Unicode codepoints
 *  @param length Number of codepoints
 *  @param position Starting position
 *  @param font_size Font size in pixels
 *  @param spacing Horizontal and vertical spacing between characters
 */
SLAPI void sl_render_codepoints(const int* codepoints, int length, sl_vec2_t position, float font_size, sl_vec2_t spacing);

/** Render multiple codepoints centered on position
 *  @param codepoints Array of Unicode codepoints
 *  @param length Number of codepoints
 *  @param position Center position
 *  @param font_size Font size in pixels
 *  @param spacing Horizontal and vertical spacing between characters
 */
SLAPI void sl_render_codepoints_centered(const int* codepoints, int length, sl_vec2_t position, float font_size, sl_vec2_t spacing);

/** Render null-terminated text string
 *  @param text UTF-8 encoded string
 *  @param position Starting position
 *  @param font_size Font size in pixels
 *  @param spacing Horizontal and vertical spacing between characters
 */
SLAPI void sl_render_text(const char* text, sl_vec2_t position, float font_size, sl_vec2_t spacing);

/** Render centered text string
 *  @param text UTF-8 encoded string
 *  @param position Center position
 *  @param font_size Font size in pixels
 *  @param spacing Horizontal and vertical spacing between characters
 */
SLAPI void sl_render_text_centered(const char* text, sl_vec2_t position, float font_size, sl_vec2_t spacing);

/** Render mesh, count = number of vertices or indices if index buffer used
 *  @param mesh Mesh to render
 *  @param count Number of vertices or indices to render
 */
SLAPI void sl_render_mesh(sl_mesh_id mesh, uint32_t count);

/** Render mesh as wireframe, count = number of vertices or indices if index buffer used
 *  @param mesh Mesh to render as wireframe
 *  @param count Number of vertices or indices to render
 */
SLAPI void sl_render_mesh_lines(sl_mesh_id mesh, uint32_t count);

/** @} */ // end of Render

/* === Canvas Functions === */

/** @defgroup Canvas Canvas Functions
 *  Functions to create and manage offscreen render targets.
 *  @{
 */

/**
 * @brief Create a new canvas (offscreen render target).
 * @param w Width in pixels
 * @param h Height in pixels
 * @param format Pixel format
 * @param depth Whether to create a depth buffer
 * @return sl_canvas_id Identifier for the canvas
 */
SLAPI sl_canvas_id sl_canvas_create(int w, int h, sl_pixel_format_t format, bool depth);

/**
 * @brief Destroy a canvas
 * @param canvas Canvas identifier
 */
SLAPI void sl_canvas_destroy(sl_canvas_id canvas);

/**
 * @brief Query a canvas properties
 * @param canvas Canvas identifier
 * @param color Returns the color texture id (nullable)
 * @param depth Returns the depth texture id (nullable)
 * @param w Returns width (nullable)
 * @param h Returns height (nullable)
 * @return True if the canvas exists
 */
SLAPI bool sl_canvas_query(sl_canvas_id canvas, sl_texture_id* color, sl_texture_id* depth, int* w, int* h);

/** @} */ // Canvas

/* === Texture Functions === */

/** @defgroup Texture Texture Functions
 *  Functions to create, load, and manage textures.
 *  @{
 */

/**
 * @brief Create a texture from raw pixel data
 * @param pixels Pointer to pixel data
 * @param w Width
 * @param h Height
 * @param format Pixel format
 * @return Texture identifier
 */
SLAPI sl_texture_id sl_texture_create(const void* pixels, int w, int h, sl_pixel_format_t format);

/**
 * @brief Load a texture from file
 * @param file_path Path to image file
 * @param w Returns width (nullable)
 * @param h Returns height (nullable)
 * @return Texture identifier
 */
SLAPI sl_texture_id sl_texture_load(const char* file_path, int* w, int* h);

/**
 * @brief Load a texture from memory
 * @param image Pointer to sl_image_t structure
 * @return Texture identifier
 */
SLAPI sl_texture_id sl_texture_load_from_memory(const sl_image_t* image);

/**
 * @brief Destroy a texture
 * @param texture Texture identifier
 */
SLAPI void sl_texture_destroy(sl_texture_id texture);

/**
 * @brief Set texture parameters
 * @param texture Texture identifier
 * @param filter Filtering mode
 * @param wrap Wrapping mode
 */
SLAPI void sl_texture_parameters(sl_texture_id texture, sl_filter_mode_t filter, sl_wrap_mode_t wrap);

/**
 * @brief Query texture size
 * @param texture Texture identifier
 * @param w Returns width (nullable)
 * @param h Returns height (nullable)
 * @return True if texture exists
 */
SLAPI bool sl_texture_query(sl_texture_id texture, int* w, int* h);

/** @} */ // Texture

/* === Mesh Functions === */

/** @defgroup Mesh Mesh Functions
 *  Functions to create and update meshes.
 *  @{
 */

/**
 * @brief Create a new mesh
 * @param vertices Pointer to vertex array
 * @param v_count Number of vertices
 * @param indices Pointer to index array
 * @param i_count Number of indices
 * @return Mesh identifier
 */
SLAPI sl_mesh_id sl_mesh_create(const sl_vertex_3d_t* vertices, uint16_t v_count, const uint16_t* indices, uint32_t i_count);

/**
 * @brief Destroy a mesh
 * @param mesh Mesh identifier
 */
SLAPI void sl_mesh_destroy(sl_mesh_id mesh);

/**
 * @brief Update vertices of a mesh
 * @param mesh Mesh identifier
 * @param vertices Pointer to new vertices
 * @param count Number of vertices
 */
SLAPI void sl_mesh_update_vertices(sl_mesh_id mesh, const sl_vertex_3d_t* vertices, uint16_t count);

/**
 * @brief Update indices of a mesh
 * @param mesh Mesh identifier
 * @param indices Pointer to new indices
 * @param count Number of indices
 */
SLAPI void sl_mesh_update_indices(sl_mesh_id mesh, const uint16_t* indices, uint32_t count);

/** @} */ // Mesh

/* === Shader Functions === */

/** @defgroup Shader Shader Functions
 *  Functions to create, load and query shaders.
 *  @{
 */

/**
 * @brief Create a shader from source code
 * @param code GLSL/HLSL shader code
 * @return Shader identifier
 */
SLAPI sl_shader_id sl_shader_create(const char* code);

/**
 * @brief Load a shader from file
 * @param file_path Path to shader file
 * @return Shader identifier
 */
SLAPI sl_shader_id sl_shader_load(const char* file_path);

/**
 * @brief Destroy a shader
 * @param shader Shader identifier
 */
SLAPI void sl_shader_destroy(sl_shader_id shader);

/**
 * @brief Get uniform location in a shader
 * @param shader Shader identifier
 * @param name Uniform variable name
 * @return Uniform location (or -1 if not found)
 */
SLAPI int sl_shader_uniform(sl_shader_id shader, const char* name);

/** @} */ // Shader

/* === Image Functions === */

/** @defgroup Image Image Functions
 *  Functions to create, manipulate, and draw images stored in RAM.
 *  @{
 */

/**
 * @brief Create a blank image in RAM
 * @param image Pointer to an sl_image_t structure
 * @param w Width of the image in pixels
 * @param h Height of the image in pixels
 * @param format Pixel format
 * @return True on success, false on failure
 */
SLAPI bool sl_image_create(sl_image_t* image, int w, int h, sl_pixel_format_t format);

/**
 * @brief Load an image from a file into RAM
 * @param image Pointer to an sl_image_t structure
 * @param file_path Path to image file
 * @return True on success, false on failure
 */
SLAPI bool sl_image_load(sl_image_t* image, const char* file_path);

/**
 * @brief Destroy an image and free its memory
 * @param image Pointer to an sl_image_t structure
 */
SLAPI void sl_image_destroy(sl_image_t* image);

/**
 * @brief Set a pixel's color in an image
 * @param image Pointer to an sl_image_t structure
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Color to set
 */
SLAPI void sl_image_set_pixel(const sl_image_t* image, int x, int y, sl_color_t color);

/**
 * @brief Get a pixel's color from an image
 * @param image Pointer to an sl_image_t structure
 * @param x X coordinate
 * @param y Y coordinate
 * @return Color of the pixel
 */
SLAPI sl_color_t sl_image_get_pixel(const sl_image_t* image, int x, int y);

/**
 * @brief Fill the entire image with a single color
 * @param image Pointer to an sl_image_t structure
 * @param color Color to fill
 */
SLAPI void sl_image_fill(const sl_image_t* image, sl_color_t color);

/**
 * @brief Blit a region from one image to another
 * @param dst Destination image
 * @param x_dst X position in destination
 * @param y_dst Y position in destination
 * @param w_dst Width of destination region
 * @param h_dst Height of destination region
 * @param src Source image
 * @param x_src X position in source
 * @param y_src Y position in source
 * @param w_src Width of source region
 * @param h_src Height of source region
 */
SLAPI void sl_image_blit(const sl_image_t* dst, int x_dst, int y_dst, int w_dst, int h_dst,
                         const sl_image_t* src, int x_src, int y_src, int w_src, int h_src);

/**
 * @brief Resize an image in RAM
 * @param image Pointer to an sl_image_t structure
 * @param new_w New width
 * @param new_h New height
 * @return True on success, false on failure
 */
SLAPI bool sl_image_resize(sl_image_t* image, int new_w, int new_h);

/**
 * @brief Flip an image horizontally
 * @param image Pointer to an sl_image_t structure
 */
SLAPI void sl_image_flip_horizontal(const sl_image_t* image);

/**
 * @brief Flip an image vertically
 * @param image Pointer to an sl_image_t structure
 */
SLAPI void sl_image_flip_vertical(const sl_image_t* image);

/**
 * @brief Draw a rectangle on an image
 * @param image Pointer to an sl_image_t structure
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param color Color of the rectangle
 * @param filled True to fill, false for outline
 */
SLAPI void sl_image_draw_rectangle(const sl_image_t* image, int x, int y, int w, int h, sl_color_t color, bool filled);

/**
 * @brief Draw a circle on an image
 * @param image Pointer to an sl_image_t structure
 * @param cx Center X
 * @param cy Center Y
 * @param radius Radius
 * @param color Color of the circle
 * @param filled True to fill, false for outline
 */
SLAPI void sl_image_draw_circle(const sl_image_t* image, int cx, int cy, int radius, sl_color_t color, bool filled);

/**
 * @brief Draw a line on an image
 * @param image Pointer to an sl_image_t structure
 * @param x0 Start X
 * @param y0 Start Y
 * @param x1 End X
 * @param y1 End Y
 * @param color Color of the line
 */
SLAPI void sl_image_draw_line(const sl_image_t* image, int x0, int y0, int x1, int y1, sl_color_t color);

/** @} */ // Image

/* === Font Functions === */

/** @defgroup Font Font Functions
 *  Functions for loading, managing, and measuring fonts.
 *  @{
 */

/**
 * @brief Load a font from memory
 * @param file_data Pointer to font file data in memory
 * @param data_size Size of the font data in bytes
 * @param type Font type (e.g., bitmap, vector)
 * @param base_size Base size of the font
 * @param codepoints Array of codepoints to load
 * @param codepoint_count Number of codepoints in the array
 * @return Font ID on success, 0 or invalid ID on failure
 */
SLAPI sl_font_id sl_font_load_from_memory(const void* file_data, size_t data_size, sl_font_type_t type,
                                          int base_size, int* codepoints, int codepoint_count);

/**
 * @brief Load a font from a file
 * @param filePath Path to the font file
 * @param type Font type (e.g., bitmap, vector)
 * @param base_size Base size of the font
 * @param codepoints Array of codepoints to load
 * @param codepoint_count Number of codepoints in the array
 * @return Font ID on success, 0 or invalid ID on failure
 */
SLAPI sl_font_id sl_font_load(const char* filePath, sl_font_type_t type,
                              int base_size, int* codepoints, int codepoint_count);

/**
 * @brief Destroy a loaded font
 * @param font Font ID
 */
SLAPI void sl_font_destroy(sl_font_id font);

/**
 * @brief Measure the size of a text string with a given font
 * @param w Pointer to store the calculated width
 * @param h Pointer to store the calculated height
 * @param font Font ID
 * @param text Null-terminated string to measure
 * @param font_size Font size
 * @param x_spacing Additional horizontal spacing between characters
 * @param y_spacing Additional vertical spacing between lines
 */
SLAPI void sl_font_measure_text(float* w, float* h, sl_font_id font, const char* text,
                                float font_size, float x_spacing, float y_spacing);

/**
 * @brief Measure the size of an array of codepoints with a given font
 * @param w Pointer to store the calculated width
 * @param h Pointer to store the calculated height
 * @param font Font ID
 * @param codepoints Array of codepoints
 * @param length Number of codepoints in the array
 * @param font_size Font size
 * @param x_spacing Additional horizontal spacing between characters
 * @param y_spacing Additional vertical spacing between lines
 */
SLAPI void sl_font_measure_codepoints(float* w, float* h, sl_font_id font, const int* codepoints, int length,
                                      float font_size, float x_spacing, float y_spacing);

/** @} */ // Font

/* === Audio Functions === */

/** @defgroup Audio Audio Functions
 *  Global audio parameters and volume control.
 *  @{
 */

/**
 * @brief Get the master volume
 * @return Master volume (0.0 = mute, 1.0 = max)
 */
SLAPI float sl_audio_get_volume_master(void);

/**
 * @brief Get the music volume
 * @return Music volume (0.0 = mute, 1.0 = max)
 */
SLAPI float sl_audio_get_volume_music(void);

/**
 * @brief Get the sound effects volume
 * @return Sound volume (0.0 = mute, 1.0 = max)
 */
SLAPI float sl_audio_get_volume_sound(void);

/**
 * @brief Set the master volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
SLAPI void sl_audio_set_volume_master(float volume);

/**
 * @brief Set the music volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
SLAPI void sl_audio_set_volume_music(float volume);

/**
 * @brief Set the sound effects volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
SLAPI void sl_audio_set_volume_sound(float volume);

/** @} */ // Audio

/* === Sound Functions === */

/** @defgroup Sound Sound Functions
 *  Sound effects with channel-based polyphony.
 *  @{
 */

/**
 * @brief Load a sound from a file
 * @param file_path Path to the sound file (supports WAV, FLAC, MP3, OGG)
 * @param channel_count Number of channels for polyphony (must be > 0)
 * @return Sound ID on success, 0 on failure
 */
SLAPI sl_sound_id sl_sound_load(const char* file_path, int channel_count);

/**
 * @brief Destroy a loaded sound and free all associated resources
 * @param sound_id Sound ID to destroy
 */
SLAPI void sl_sound_destroy(sl_sound_id sound_id);

/**
 * @brief Play a sound on a specific channel or automatically find a free one
 * @param sound_id Sound ID to play
 * @param channel Channel index (>= 0) or negative value to auto-select first available channel
 * @return Channel index where the sound is played, or -1 if no channel is available
 */
SLAPI int sl_sound_play(sl_sound_id sound_id, int channel);

/**
 * @brief Pause a sound on a specific channel or all channels
 * @param sound_id Sound ID to pause
 * @param channel Channel index (>= 0) to pause specific channel, or negative value to pause all channels
 */
SLAPI void sl_sound_pause(sl_sound_id sound_id, int channel);

/**
 * @brief Stop a sound on a specific channel or all channels
 * @param sound_id Sound ID to stop
 * @param channel Channel index (>= 0) to stop specific channel, or negative value to stop all channels
 */
SLAPI void sl_sound_stop(sl_sound_id sound_id, int channel);

/**
 * @brief Rewind a sound on a specific channel or all channels
 * @param sound_id Sound ID to rewind
 * @param channel Channel index (>= 0) to rewind specific channel, or negative value to rewind all channels
 */
SLAPI void sl_sound_rewind(sl_sound_id sound_id, int channel);

/**
 * @brief Check if a sound is playing on a specific channel or any channel
 * @param sound_id Sound ID to check
 * @param channel Channel index (>= 0) to check specific channel, or negative value to check if playing on any channel
 * @return true if sound is playing on the specified channel(s), false otherwise
 */
SLAPI bool sl_sound_is_playing(sl_sound_id sound_id, int channel);

/**
 * @brief Get the number of channels assigned to a sound
 * @param sound_id Sound ID
 * @return Number of channels, or 0 if sound ID is invalid
 */
SLAPI int sl_sound_get_channel_count(sl_sound_id sound_id);

/** @} */ // Sound

/* === Music Functions === */

/** @defgroup Music Music Functions
 *  Background music playback and control.
 *  @{
 */

/**
 * @brief Load music from a file
 * @param file_path Path to the music file
 * @return Music ID on success, 0 or invalid ID on failure
 */
SLAPI sl_music_id sl_music_load(const char* file_path);

/**
 * @brief Destroy loaded music
 * @param music Music ID
 */
SLAPI void sl_music_destroy(sl_music_id music);

/**
 * @brief Play music
 * @param music Music ID
 */
SLAPI void sl_music_play(sl_music_id music);

/**
 * @brief Pause music
 * @param music Music ID
 */
SLAPI void sl_music_pause(sl_music_id music);

/**
 * @brief Stop music
 * @param music Music ID
 */
SLAPI void sl_music_stop(sl_music_id music);

/**
 * @brief Rewind music to the beginning
 * @param music Music ID
 */
SLAPI void sl_music_rewind(sl_music_id music);

/**
 * @brief Check if music is currently playing
 * @param music Music ID
 * @return true if playing, false otherwise
 */
SLAPI bool sl_music_is_playing(sl_music_id music);

/**
 * @brief Enable or disable looping for music
 * @param music Music ID
 * @param loop true to loop continuously, false to stop at the end
 */
SLAPI void sl_music_loop(sl_music_id music, bool loop);

/** @} */ // Music

/* === Text Functions === */

/** @defgroup Text Text Functions
 *  Functions for string manipulation.
 *  @{
 */

/**
 * @brief Convert a string to uppercase
 * @param str Input string (modified in-place)
 */
SLAPI void sl_text_to_upper(char* str);

/**
 * @brief Convert a string to lowercase
 * @param str Input string (modified in-place)
 */
SLAPI void sl_text_to_lower(char* str);

/**
 * @brief Compare two strings
 * @param str1 First string
 * @param str2 Second string
 * @return true if equal, false otherwise
 */
SLAPI bool sl_text_compare(const char* str1, const char* str2);

/**
 * @brief Compare two strings ignoring case
 * @param str1 First string
 * @param str2 Second string
 * @return true if equal ignoring case
 */
SLAPI bool sl_text_compare_ignore_case(const char* str1, const char* str2);

/**
 * @brief Check if a string starts with a prefix
 * @param str Input string
 * @param prefix Prefix to check
 * @return true if string starts with prefix
 */
SLAPI bool sl_text_starts_with(const char* str, const char* prefix);

/**
 * @brief Check if a string ends with a suffix
 * @param str Input string
 * @param suffix Suffix to check
 * @return Pointer to the suffix in the string if found, NULL otherwise
 */
SLAPI char* sl_text_ends_with(const char* str, const char* suffix);

/**
 * @brief Check if a string contains a keyword
 * @param str Input string
 * @param keyword Keyword to search
 * @return Pointer to the first occurrence, NULL if not found
 */
SLAPI char* sl_text_contains(const char* str, const char* keyword);

/**
 * @brief Format a string with printf-like syntax using a static buffer
 * @param text Format string (printf-style)
 * @param ... Arguments for format string
 * @return Formatted string (max 512 chars, truncated with "..." if needed)
 * @note Not thread-safe. Result is overwritten on next call.
 */
SLAPI const char* sl_text_format(const char* text, ...);

/**
 * @brief Format a string with printf-like syntax using dynamic allocation
 * @param text Format string (printf-style)
 * @param ... Arguments for format string
 * @return Dynamically allocated formatted string (caller must free)
 * @note Returns NULL on allocation failure or if text is NULL.
 */
SLAPI char* sl_text_format_dynamic(const char* text, ...);

/**
 * @brief Format a timestamp as date and time (YYYY-MM-DD HH:MM:SS)
 * @param time Time in nanoseconds since epoch
 * @return Static formatted string (not thread-safe)
 */
SLAPI const char* sl_text_format_date_time(int64_t time);

/**
 * @brief Format a timestamp as date (YYYY-MM-DD)
 * @param time Time in nanoseconds since epoch
 * @return Static formatted string (not thread-safe)
 */
SLAPI const char* sl_text_format_date(int64_t time);

/**
 * @brief Format a timestamp as time (HH:MM:SS)
 * @param time Time in nanoseconds since epoch
 * @return Static formatted string (not thread-safe)
 */
SLAPI const char* sl_text_format_time(int64_t time);

/**
 * @brief Replace all occurrences of a word in a string
 * @param str Input string
 * @param old_word Word to replace
 * @param new_word Replacement word
 * @return Newly allocated string with replacements (caller must free)
 * @note Returns NULL on allocation failure or if any parameter is NULL.
 */
SLAPI char* sl_text_replace(const char* str, const char* old_word, const char* new_word);

/**
 * @brief Concatenate two strings
 * @param src1 First string
 * @param src2 Second string
 * @return Newly allocated concatenated string (caller must free)
 * @note Returns NULL on allocation failure or if any parameter is NULL.
 */
SLAPI char* sl_text_concat(const char* src1, const char* src2);

/**
 * @brief Copy a string with maximum length limit
 * @param src Source string
 * @param max_len Maximum number of characters to copy
 * @return Newly allocated copy (caller must free)
 * @note Uses SDL_strndup internally.
 */
SLAPI char* sl_text_copy(const char* src, size_t max_len);

/**
 * @brief Count occurrences of a keyword in a string
 * @param str Input string
 * @param keyword Keyword to count
 * @return Number of occurrences
 */
SLAPI int sl_text_count_occurences(const char* str, const char* keyword);

/**
 * @brief Count words in a string (separated by whitespace)
 * @param str Input string
 * @return Number of words
 */
SLAPI int sl_text_word_count(const char* str);

/**
 * @brief Extract a specific word from a string
 * @param str Input string
 * @param word_index Index of the word to extract (0-based)
 * @param out_word Output buffer
 * @param max_len Maximum length of output buffer
 * @note Sets out_word[0] = '\0' if word not found.
 */
SLAPI void sl_text_extract_word(const char* str, int word_index, char* out_word, size_t max_len);

/**
 * @brief Reverse a string in-place
 * @param str Input string (modified in-place)
 */
SLAPI void sl_text_reverse(char* str);

/**
 * @brief Trim whitespace from both ends of a string
 * @param str Input string (modified in-place)
 */
SLAPI void sl_text_trim(char* str);

/** @} */ // Text

/* === Codepoint Functions === */

/** @defgroup Codepoint Codepoint Functions
 *  Unicode codepoint manipulation and UTF-8 conversion.
 *  @{
 */

/**
 * @brief Get the next codepoint from a UTF-8 string
 * @param text Input string
 * @param codepointSize Output size of the codepoint in bytes
 * @return Unicode codepoint
 */
SLAPI int sl_codepoint_next(const char* text, int* codepointSize);

/**
 * @brief Get the previous codepoint from a UTF-8 string
 * @param text Input string
 * @param codepointSize Output size of the codepoint in bytes
 * @return Unicode codepoint
 */
SLAPI int sl_codepoint_prev(const char* text, int* codepointSize);

/**
 * @brief Count the number of codepoints in a UTF-8 string
 * @param text Input string
 * @return Number of codepoints
 */
SLAPI int sl_codepoint_count(const char* text);

/**
 * @brief Convert a codepoint to UTF-8 string
 * @param codepoint Unicode codepoint
 * @param utf8Size Output size in bytes
 * @return Pointer to UTF-8 string representation
 */
SLAPI const char* sl_codepoint_to_utf8(int codepoint, int* utf8Size);

/**
 * @brief Convert UTF-8 string to a single codepoint
 * @param text UTF-8 encoded string
 * @param codepointSize Output size of the codepoint in bytes
 * @return Unicode codepoint
 */
SLAPI int sl_codepoint_from_utf8(const char* text, int* codepointSize);

/**
 * @brief Convert an array of codepoints to a UTF-8 string
 * @param codepoints Array of Unicode codepoints
 * @param length Number of codepoints
 * @return Newly allocated UTF-8 string
 */
SLAPI char* sl_codepoint_to_ut8_str(const int* codepoints, int length);

/**
 * @brief Convert a UTF-8 string to an array of codepoints
 * @param text Input UTF-8 string
 * @param count Output number of codepoints
 * @return Newly allocated array of codepoints
 */
SLAPI int* sl_codepoint_from_utf8_str(const char* text, int* count);

/** @} */ // Codepoint

/* === Random Functions === */

/** @defgroup Random Random Functions
 *  Pseudo-random number generation using PCG32 algorithm.
 *  @{
 */

/**
 * @brief Seed the random number generator
 * @param seed 64-bit seed value
 */
SLAPI void sl_rand_seed(uint64_t seed);

/**
 * @brief Generate a random signed 32-bit integer
 * @return Random int32_t value
 */
SLAPI int32_t sl_randi(void);

/**
 * @brief Generate a random unsigned 32-bit integer
 * @return Random uint32_t value
 */
SLAPI uint32_t sl_randui(void);

/**
 * @brief Generate a random float in the range [0.0, 1.0)
 * @return Random float value
 */
SLAPI float sl_randf(void);

/**
 * @brief Generate a random integer in a given range [min, max]
 * @param min Minimum value
 * @param max Maximum value
 * @return Random integer in range
 */
SLAPI int sl_randi_range(int min, int max);

/**
 * @brief Generate a random unsigned integer in a given range [min, max]
 * @param min Minimum value
 * @param max Maximum value
 * @return Random unsigned integer in range
 */
SLAPI uint32_t sl_randui_range(uint32_t min, uint32_t max);

/**
 * @brief Generate a random float in a given range [min, max)
 * @param min Minimum value
 * @param max Maximum value
 * @return Random float in range
 */
SLAPI float sl_randf_range(float min, float max);

/**
 * @brief Shuffle an array of elements
 * 
 * Behavior depends on element size and total memory required:
 * - If element size is aligned to size_t, the shuffle is done by swapping elements.
 * - If element size <= 64 bytes, a temporary stack array is used.
 * - If element size > 64 bytes, a heap allocation is used for the shuffle.
 * 
 * @param array Pointer to array
 * @param element_size Size of each element in bytes
 * @param count Number of elements in the array
 */
SLAPI void sl_rand_shuffle(void* array, size_t element_size, size_t count);

/** @} */ // Random

/* === General Math Functions === */

/** @defgroup Math General Math Functions
 *  Inline utility functions for general math operations.
 *  @{
 */

/**
 * @brief Swap two integers
 * @param x Pointer to first integer
 * @param y Pointer to second integer
 */
static inline void sl_swapi(int* x, int* y)
{
    int t = *x; *x = *y; *y = t;
}

/**
 * @brief Swap two floats
 * @param x Pointer to first float
 * @param y Pointer to second float
 */
static inline void sl_swap(float* x, float* y)
{
    float t = *x; *x = *y; *y = t;
}

/**
 * @brief Clamp integer to [min, max]
 * @param x Value
 * @param min Minimum
 * @param max Maximum
 * @return Clamped integer
 */
static inline int sl_clampi(int x, int min, int max)
{
    return x < min ? min : (x > max ? max : x);
}

/**
 * @brief Clamp float to [min, max]
 * @param x Value
 * @param min Minimum
 * @param max Maximum
 * @return Clamped float
 */
static inline float sl_clamp(float x, float min, float max)
{
    return x < min ? min : (x > max ? max : x);
}

/**
 * @brief Clamp float to [0.0, 1.0]
 * @param x Value
 * @return Saturated float
 */
static inline float sl_saturate(float x)
{
    return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

/**
 * @brief Wrap integer to [min, max)
 */
static inline int sl_wrapi(int value, int min, int max)
{
    int range = max - min;
    return min + (value - min) % range;
}

/**
 * @brief Wrap float to [min, max)
 */
static inline float sl_wrap(float value, float min, float max)
{
    float range = max - min;
    return min + fmodf(value - min, range);
}

/**
 * @brief Wrap radians to [-Pi, Pi]
 */
static inline float sl_wrap_radians(float radians)
{
    float wrapped = fmodf(radians, SL_TAU);
    if (wrapped < -SL_PI) {
        wrapped += SL_TAU;
    }
    else if (wrapped > SL_PI) {
        wrapped -= SL_TAU;
    }
    return wrapped;
}

/**
 * @brief Normalize value from [start, end] to [0,1]
 */
static inline float sl_normalize(float value, float start, float end)
{
    return (value - start) / (end - start);
}

/**
 * @brief Remap value from [in_start, in_end] to [out_start, out_end]
 */
static inline float sl_remap(float value, float in_start, float in_end, float out_start, float out_end)
{
    return (value - in_start) / (in_end - in_start) * (out_end - out_start) + out_start;
}

/**
 * @brief Return fractional part of float
 */
static inline float sl_fract(float x)
{
    return x - floorf(x);
}

/**
 * @brief Step function: 0 if x < edge, else 1
 */
static inline float sl_step(float edge, float x)
{
    return (x < edge) ? 0.0f : 1.0f;
}

/**
 * @brief Sign of integer: -1, 0, 1
 */
static inline int sl_sign(int x)
{
    return (x > 0) - (x < 0);
}

/**
 * @brief Approximate equality for floats with epsilon
 */
static inline int sl_approx(float a, float b, float epsilon)
{
    return fabsf(a - b) < epsilon;
}

/**
 * @brief Linear interpolation
 */
static inline float sl_lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

/**
 * @brief Linear interpolation for radians
 */
static inline float sl_lerp_radians(float a, float b, float t)
{
    return a + sl_wrap_radians(b - a) * t;
}

/**
 * @brief Inverse linear interpolation
 */
static inline float sl_lerp_inverse(float a, float b, float value)
{
    return (value - a) / (b - a);
}

/**
 * @brief Smoothstep interpolation
 */
static inline float sl_smoothstep(float edge0, float edge1, float x)
{
    float t = (x - edge0) / (edge1 - edge0);
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    return t * t * t * (t * (t * 6 - 15) + 10);
}

/**
 * @brief Exponential decay
 */
static inline float sl_exp_decay(float initial, float decay_rate, float time)
{
    return initial * expf(-decay_rate * time);
}

/**
 * @brief Move current value toward target by max delta
 */
static inline float sl_move_toward(float current, float target, float max_delta)
{
    float delta = target - current;
    float distance = fabsf(delta);
    if (distance <= max_delta) return target;
    else return current + (delta / distance) * max_delta;
}

/**
 * @brief Next power of 2 for 32-bit integer
 */
static inline uint32_t sl_po2_next(uint32_t x)
{
    if (x == 0) return 1;
    if ((x & (x - 1)) == 0) return x << 1; // (x * 2)
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

/**
 * @brief Previous power of 2 for 32-bit integer
 */
static inline uint32_t sl_po2_prev(uint32_t x)
{
    if (x == 0) return 0;
    if ((x & (x - 1)) == 0) return x >> 1; // (x / 2)
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x - (x >> 1);
}

/**
 * @brief Nearest power of 2 for 32-bit integer
 */
static inline uint32_t sl_po2_near(uint32_t x)
{
    uint32_t next = sl_po2_next(x);
    uint32_t prev = sl_po2_prev(x);
    return (x - prev < next - x) ? prev : next;
}

/**
 * @brief Next power of 2 for 64-bit integer
 */
static inline uint64_t sl_po2_next64(uint64_t x)
{
    if (x == 0) return 1;
    if ((x & (x - 1)) == 0) return x << 1; // (x * 2)
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return x;
}

/**
 * @brief Previous power of 2 for 64-bit integer
 */
static inline uint64_t sl_po2_prev64(uint64_t x)
{
    if (x == 0) return 0;
    if ((x & (x - 1)) == 0) return x >> 1; // (x / 2)
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x - (x >> 1);
}

/**
 * @brief Nearest power of 2 for 64-bit integer
 */
static inline uint64_t sl_po2_near64(uint64_t x)
{
    uint64_t next = sl_po2_next64(x);
    uint64_t prev = sl_po2_prev64(x);
    return (x - prev < next - x) ? prev : next;
}

/** @} */ // Math

/* === Ease Functions === */

/** @defgroup Easing Ease Functions
 *  Inline functions for easing/interpolation.
 *  t is expected to be in [0,1].
 *  @{
 */

/**
 * @brief Sine easing in
 * @see https://easings.net/#easeInSine
 */
static inline float sl_ease_sine_in(float t)
{
    return sinf(SL_PI / 2 * t);
}

/**
 * @brief Sine easing out
 * @see https://easings.net/#easeOutSine
 */
static inline float sl_ease_sine_out(float t)
{
    return 1 + sinf(SL_PI / 2 * (--t));
}

/**
 * @brief Sine easing in-out
 * @see https://easings.net/#easeInOutSine
 */
static inline float sl_ease_sine_inout(float t)
{
    return 0.5f * (1 + sinf(SL_PI * (t - 0.5f)));
}

/**
 * @brief Quadratic easing in
 * @see https://easings.net/#easeInQuad
 */
static inline float sl_ease_quad_in(float t)
{
    return t * t;
}

/**
 * @brief Quadratic easing out
 * @see https://easings.net/#easeOutQuad
 */
static inline float sl_ease_quad_out(float t)
{
    return t * (2 - t);
}

/**
 * @brief Quadratic easing in-out
 * @see https://easings.net/#easeInOutQuad
 */
static inline float sl_ease_quad_inout(float t)
{
    return t < 0.5f ? 2 * t * t : t * (4 - 2 * t) - 1;
}

/**
 * @brief Cubic easing in
 * @see https://easings.net/#easeInCubic
 */
static inline float sl_ease_cubic_in(float t)
{
    return t * t * t;
}

/**
 * @brief Cubic easing out
 * @see https://easings.net/#easeOutCubic
 */
static inline float sl_ease_cubic_out(float t)
{
    --t; return 1 + t * t * t;
}

/**
 * @brief Cubic easing in-out
 * @see https://easings.net/#easeInOutCubic
 */
static inline float sl_ease_cubic_inout(float t)
{
    return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
}

/**
 * @brief Quartic easing in
 * @see https://easings.net/#easeInQuart
 */
static inline float sl_ease_quart_in(float t)
{
    t *= t;
    return t * t;
}

/**
 * @brief Quartic easing out
 * @see https://easings.net/#easeOutQuart
 */
static inline float sl_ease_quart_out(float t)
{
    --t; t = t * t;
    return 1 - t * t;
}

/**
 * @brief Quartic easing in-out
 * @see https://easings.net/#easeInOutQuart
 */
static inline float sl_ease_quart_inout(float t)
{
    if (t < 0.5f) {
        t *= t;
        return 8 * t * t;
    }
    else {
        --t; t = t * t;
        return 1 - 8 * t * t;
    }
}

/**
 * @brief Quintic easing in
 * @see https://easings.net/#easeInQuint
 */
static inline float sl_ease_quint_in(float t)
{
    float t2 = t * t;
    return t * t2 * t2;
}

/**
 * @brief Quintic easing out
 * @see https://easings.net/#easeInOutQuint
 */
static inline float sl_ease_quint_out(float t)
{
    --t; float t2 = t * t;
    return 1 + t * t2 * t2;
}

/**
 * @brief Quintic easing in-out
 * @see https://easings.net/#easeInOutQuint
 */
static inline float sl_ease_quint_inout(float t)
{
    float t2;
    if (t < 0.5f) {
        t2 = t * t;
        return 16 * t * t2 * t2;
    }
    --t; t2 = t * t;
    return 1 + 16 * t * t2 * t2;
}

/**
 * @brief Exponential easing in
 * @see https://easings.net/#easeInExpo
 */
static inline float sl_ease_expo_in(float t)
{
    return (powf(2, 8 * t) - 1) / 255;
}

/**
 * @brief Exponential easing out
 * @see https://easings.net/#easeOutExpo
 */
static inline float sl_ease_expo_out(float t)
{
    return 1 - powf(2, -8 * t);
}

/**
 * @brief Exponential easing in-out
 * @see https://easings.net/#easeInOutExpo
 */
static inline float sl_ease_expo_inout(float t)
{
    if (t < 0.5f) {
        return (powf(2, 16 * t) - 1) / 510;
    }
    return 1 - 0.5f * powf(2, -16 * (t - 0.5f));
}

/**
 * @brief Circular easing in
 * @see https://easings.net/#easeInCirc
 */
static inline float sl_ease_circ_in(float t)
{
    return 1 - sqrtf(1 - t);
}

/**
 * @brief Circular easing out
 * @see https://easings.net/#easeOutCirc
 */
static inline float sl_ease_circ_out(float t)
{
    return sqrtf(t);
}

/**
 * @brief Circular easing in-out
 * @see https://easings.net/#easeInOutCirc
 */
static inline float sl_ease_circ_inout(float t)
{
    if (t < 0.5f) {
        return (1 - sqrtf(1 - 2 * t)) * 0.5f;
    }
    return (1 + sqrtf(2 * t - 1)) * 0.5f;
}

/**
 * @brief Back easing in
 * @see https://easings.net/#easeInBack
 */
static inline float sl_ease_back_in(float t)
{
    return t * t * (2.70158f * t - 1.70158f);
}

/**
 * @brief Back easing out
 * @see https://easings.net/#easeOutBack
 */
static inline float sl_ease_back_out(float t)
{
    --t; return 1 + t * t * (2.70158f * t + 1.70158f);
}

/**
 * @brief Back easing in-out
 * @see https://easings.net/#easeInOutBack
 */
static inline float sl_ease_back_inout(float t)
{
    if (t < 0.5f) {
        return t * t * (7 * t - 2.5f) * 2;
    }
    --t; return 1 + t * t * 2 * (7 * t + 2.5f);
}

/**
 * @brief Elastic easing in
 * @see https://easings.net/#easeInElastic
 */
static inline float sl_ease_elastic_in(float t)
{
    float t2 = t * t;
    return t2 * t2 * sinf(t * SL_PI * 4.5f);
}

/**
 * @brief Elastic easing out
 * @see https://easings.net/#easeOutElastic
 */
static inline float sl_ease_elastic_out(float t)
{
    float t2 = (t - 1) * (t - 1);
    return 1 - t2 * t2 * cosf(t * SL_PI * 4.5f);
}

/**
 * @brief Elastic easing in-out
 * @see https://easings.net/#easeInOutElastic
 */
static inline float sl_ease_elastic_inout(float t)
{
    float t2;
    if (t < 0.45f) {
        t2 = t * t;
        return 8 * t2 * t2 * sinf(t * SL_PI * 9);
    }
    else if (t < 0.55f) {
        return 0.5f + 0.75f * sinf(t * SL_PI * 4);
    }
    t2 = (t - 1) * (t - 1);
    return 1 - 8 * t2 * t2 * sinf(t * SL_PI * 9);
}

/**
 * @brief Bounce easing in
 * @see https://easings.net/#easeInBounce
 */
static inline float sl_ease_bounce_in(float t)
{
    return powf(2, 6 * (t - 1)) * fabsf(sinf(t * SL_PI * 3.5f));
}

/**
 * @brief Bounce easing out
 * @see https://easings.net/#easeOutBounce
 */
static inline float sl_ease_bounce_out(float t)
{
    return 1 - powf(2, -6 * t) * fabsf(cosf(t * SL_PI * 3.5f));
}

/**
 * @brief Bounce easing in-out
 * @see https://easings.net/#easeInOutBounce
 */
static inline float sl_ease_bounce_inout(float t)
{
    if (t < 0.5f) {
        return 8 * powf(2, 8 * (t - 1)) * fabsf(sinf(t * SL_PI * 7));
    }
    return 1 - 8 * powf(2, -8 * t) * fabsf(sinf(t * SL_PI * 7));
}

/** @} */ // Easing

/* === 2D Vector Functions === */

/** @defgroup Vec2 2D Vector Functions
 *  Inline functions for 2D vector operations.
 *  @{
 */

 /**
 * @brief Swap two vectors
 */
static inline void sl_vec2_swap(sl_vec2_t* SL_RESTRICT v0, sl_vec2_t* SL_RESTRICT v1)
{
    float t;
    t = v0->x; v0->x = v1->x; v1->x = t;
    t = v0->y; v0->y = v1->y; v1->y = t;
}

/**
 * @brief Component-wise minimum of two vectors
 */
static inline sl_vec2_t sl_vec2_min(sl_vec2_t v, sl_vec2_t min)
{
    return SL_VEC2(SL_MIN(v.x, min.x), SL_MIN(v.y, min.y));
}

/**
 * @brief Component-wise maximum of two vectors
 */
static inline sl_vec2_t sl_vec2_max(sl_vec2_t v, sl_vec2_t max)
{
    return SL_VEC2(SL_MAX(v.x, max.x), SL_MAX(v.y, max.y));
}

/**
 * @brief Clamp vector components between min and max
 */
static inline sl_vec2_t sl_vec2_clamp(sl_vec2_t v, sl_vec2_t min, sl_vec2_t max)
{
    return SL_VEC2(SL_CLAMP(v.x, min.x, max.x), SL_CLAMP(v.y, min.y, max.y));
}

/**
 * @brief Absolute value of vector components
 */
static inline sl_vec2_t sl_vec2_abs(sl_vec2_t v)
{
    return SL_VEC2(fabsf(v.x), fabsf(v.y));
}

/**
 * @brief Negate vector components
 */
static inline sl_vec2_t sl_vec2_neg(sl_vec2_t v)
{
    return SL_VEC2(-v.x, -v.y);
}

/**
 * @brief Component-wise addition
 */
static inline sl_vec2_t sl_vec2_add(sl_vec2_t v0, sl_vec2_t v1)
{
    return SL_VEC2(v0.x + v1.x, v0.y + v1.y);
}

/**
 * @brief Component-wise subtraction
 */
static inline sl_vec2_t sl_vec2_sub(sl_vec2_t v0, sl_vec2_t v1)
{
    return SL_VEC2(v0.x - v1.x, v0.y - v1.y);
}

/**
 * @brief Component-wise multiplication
 */
static inline sl_vec2_t sl_vec2_mul(sl_vec2_t v0, sl_vec2_t v1)
{
    return SL_VEC2(v0.x * v1.x, v0.y * v1.y);
}

/**
 * @brief Component-wise division
 */
static inline sl_vec2_t sl_vec2_div(sl_vec2_t v0, sl_vec2_t v1)
{
    return SL_VEC2(v0.x / v1.x, v0.y / v1.y);
}

/**
 * @brief Offset vector by scalar
 */
static inline sl_vec2_t sl_vec2_offset(sl_vec2_t v, float s)
{
    return SL_VEC2(v.x + s, v.y + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline sl_vec2_t sl_vec2_scale(sl_vec2_t v, float s)
{
    return SL_VEC2(v.x * s, v.y * s);
}

/**
 * @brief Dot product
 */
static inline float sl_vec2_dot(sl_vec2_t v0, sl_vec2_t v1)
{
    return v0.x * v1.x + v0.y * v1.y;
}

/**
 * @brief Vector length
 */
static inline float sl_vec2_length(sl_vec2_t v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

/**
 * @brief Squared vector length
 */
static inline float sl_vec2_length_sq(sl_vec2_t v)
{
    return v.x * v.x + v.y * v.y;
}

/**
 * @brief Normalize vector
 */
static inline sl_vec2_t sl_vec2_normalize(sl_vec2_t v)
{
    float len = sl_vec2_length(v);
    if (len > 0.0f) {
        return sl_vec2_scale(v, 1.0f / len);
    }
    return SL_VEC2(0.0f, 0.0f);
}

/**
 * @brief Distance between two vectors
 */
static inline float sl_vec2_distance(sl_vec2_t v0, sl_vec2_t v1)
{
    return sl_vec2_length(sl_vec2_sub(v1, v0));
}

/**
 * @brief Squared distance between two vectors
 */
static inline float sl_vec2_distance_sq(sl_vec2_t v0, sl_vec2_t v1)
{
    sl_vec2_t diff = sl_vec2_sub(v1, v0);
    return sl_vec2_length_sq(diff);
}

/**
 * @brief CCW angle from X axis (radians)
 */
static inline float sl_vec2_angle_ccw(sl_vec2_t v)
{
    return atan2f(v.y, v.x);
}

/**
 * @brief CW angle from X axis (radians)
 */
static inline float sl_vec2_angle_cw(sl_vec2_t v)
{
    return -atan2f(v.y, v.x);
}

/**
 * @brief CCW angle between two vectors (radians)
 */
static inline float sl_vec2_line_angle_ccw(sl_vec2_t v0, sl_vec2_t v1)
{
    return atan2f(v1.y - v0.y, v1.x - v0.x);
}

/**
 * @brief CW angle between two vectors (radians)
 */
static inline float sl_vec2_line_angle_cw(sl_vec2_t v0, sl_vec2_t v1)
{
    return -atan2f(v1.y - v0.y, v1.x - v0.x);
}

/**
 * @brief Create unit vector from angle (radians)
 */
static inline sl_vec2_t sl_vec2_from_angle(float angle)
{
    return SL_VEC2(cosf(angle), sinf(angle));
}

/**
 * @brief Rotate vector by angle (radians)
 */
static inline sl_vec2_t sl_vec2_rotate(sl_vec2_t v, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return SL_VEC2(v.x * c - v.y * s, v.x * s + v.y * c);
}

/**
 * @brief Get direction from v0 to v1, normalized
 */
static inline sl_vec2_t sl_vec2_direction(sl_vec2_t v0, sl_vec2_t v1)
{
    return sl_vec2_normalize(sl_vec2_sub(v1, v0));
}

/**
 * @brief Linear interpolation between vectors
 */
static inline sl_vec2_t sl_vec2_lerp(sl_vec2_t v0, sl_vec2_t v1, float t)
{
    return SL_VEC2(
        v0.x + (v1.x - v0.x) * t,
        v0.y + (v1.y - v0.y) * t
    );
}

/**
 * @brief Move vector toward target without exceeding max_delta
 */
static inline sl_vec2_t sl_vec2_move_toward(sl_vec2_t from, sl_vec2_t to, float max_delta)
{
    sl_vec2_t delta = SL_VEC2(to.x - from.x, to.y - from.y);
    float dist = sqrtf(delta.x * delta.x + delta.y * delta.y);

    if (dist <= max_delta || dist < 1e-6f) {
        return to;
    }

    float scale = max_delta / dist;
    return SL_VEC2(from.x + delta.x * scale, from.y + delta.y * scale);
}

/**
 * @brief Reflect vector across normal
 */
static inline sl_vec2_t sl_vec2_reflect(sl_vec2_t v, sl_vec2_t n)
{
    float dot = sl_vec2_dot(v, n);
    return sl_vec2_sub(v, sl_vec2_scale(n, 2.0f * dot));
}

/**
 * @brief Perpendicular vector (rotated 90° CCW)
 */
static inline sl_vec2_t sl_vec2_perp(sl_vec2_t v)
{
    return SL_VEC2(-v.y, v.x);
}

/**
 * @brief Check approximate equality of two vectors
 */
static inline int sl_vec2_approx(sl_vec2_t v0, sl_vec2_t v1, float epsilon)
{
    return (fabsf(v0.x - v1.x) < epsilon) &&
           (fabsf(v0.y - v1.y) < epsilon);
}

/**
 * @brief Transform vector by 4x4 matrix
 */
static inline sl_vec2_t sl_vec2_transform(sl_vec2_t v, const sl_mat4_t* mat)
{
    sl_vec2_t result;
    result.x = mat->m00 * v.x + mat->m10 * v.y + mat->m30;
    result.y = mat->m01 * v.x + mat->m11 * v.y + mat->m31;

    return result;
}

/** @} */ // Vec2

/* === 3D Vector Functions === */

/** @defgroup Vec3 3D Vector Functions
 *  Inline functions for 3D vector operations.
 *  @{
 */

/**
 * @brief Swap two vectors
 */
static inline void sl_vec3_swap(sl_vec3_t* SL_RESTRICT v0, sl_vec3_t* SL_RESTRICT v1)
{
    float t;
    t = v0->x; v0->x = v1->x; v1->x = t;
    t = v0->y; v0->y = v1->y; v1->y = t;
    t = v0->z; v0->z = v1->z; v1->z = t;
}

/**
 * @brief Component-wise minimum
 */
static inline sl_vec3_t sl_vec3_min(sl_vec3_t v, sl_vec3_t min)
{
    return SL_VEC3(SL_MIN(v.x, min.x), SL_MIN(v.y, min.y), SL_MIN(v.z, min.z));
}

/**
 * @brief Component-wise maximum
 */
static inline sl_vec3_t sl_vec3_max(sl_vec3_t v, sl_vec3_t max)
{
    return SL_VEC3(SL_MAX(v.x, max.x), SL_MAX(v.y, max.y), SL_MAX(v.z, max.z));
}

/**
 * @brief Clamp each component between min and max
 */
static inline sl_vec3_t sl_vec3_clamp(sl_vec3_t v, sl_vec3_t min, sl_vec3_t max)
{
    return SL_VEC3(SL_CLAMP(v.x, min.x, max.x), SL_CLAMP(v.y, min.y, max.y), SL_CLAMP(v.z, min.z, max.z));
}

/**
 * @brief Component-wise absolute value
 */
static inline sl_vec3_t sl_vec3_abs(sl_vec3_t v)
{
    return SL_VEC3(fabsf(v.x), fabsf(v.y), fabsf(v.z));
}

/**
 * @brief Negate vector
 */
static inline sl_vec3_t sl_vec3_neg(sl_vec3_t v)
{
    return SL_VEC3(-v.x, -v.y, -v.z);
}

/**
 * @brief Vector addition
 */
static inline sl_vec3_t sl_vec3_add(sl_vec3_t v0, sl_vec3_t v1)
{
    return SL_VEC3(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

/**
 * @brief Vector subtraction
 */
static inline sl_vec3_t sl_vec3_sub(sl_vec3_t v0, sl_vec3_t v1)
{
    return SL_VEC3(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

/**
 * @brief Component-wise multiplication
 */
static inline sl_vec3_t sl_vec3_mul(sl_vec3_t v0, sl_vec3_t v1)
{
    return SL_VEC3(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z);
}

/**
 * @brief Component-wise division
 */
static inline sl_vec3_t sl_vec3_div(sl_vec3_t v0, sl_vec3_t v1)
{
    return SL_VEC3(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z);
}

/**
 * @brief Add scalar to each component
 */
static inline sl_vec3_t sl_vec3_offset(sl_vec3_t v, float s)
{
    return SL_VEC3(v.x + s, v.y + s, v.z + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline sl_vec3_t sl_vec3_scale(sl_vec3_t v, float s)
{
    return SL_VEC3(v.x * s, v.y * s, v.z * s);
}

/**
 * @brief Dot product
 */
static inline float sl_vec3_dot(sl_vec3_t v0, sl_vec3_t v1)
{
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

/**
 * @brief Cross product
 */
static inline sl_vec3_t sl_vec3_cross(sl_vec3_t v0, sl_vec3_t v1)
{
    return SL_VEC3(
        v0.y * v1.z - v0.z * v1.y,
        v0.z * v1.x - v0.x * v1.z,
        v0.x * v1.y - v0.y * v1.x
    );
}

/**
 * @brief Vector length (magnitude)
 */
static inline float sl_vec3_length(sl_vec3_t v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * @brief Squared length (avoids sqrt)
 */
static inline float sl_vec3_length_sq(sl_vec3_t v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

/**
 * @brief Distance between two vectors
 */
static inline float sl_vec3_distance(sl_vec3_t v0, sl_vec3_t v1)
{
    return sl_vec3_length(sl_vec3_sub(v1, v0));
}

/**
 * @brief Squared distance
 */
static inline float sl_vec3_distance_sq(sl_vec3_t v0, sl_vec3_t v1)
{
    return sl_vec3_length_sq(sl_vec3_sub(v1, v0));
}

/**
 * @brief Normalize vector
 */
static inline sl_vec3_t sl_vec3_normalize(sl_vec3_t v)
{
    float len = sl_vec3_length(v);
    return (len > 0.0f) ? sl_vec3_scale(v, 1.0f / len) : SL_VEC3(0.0f, 0.0f, 0.0f);
}

/**
 * @brief Direction vector from 'from' to 'to'
 */
static inline sl_vec3_t sl_vec3_direction(sl_vec3_t from, sl_vec3_t to)
{
    return sl_vec3_normalize(sl_vec3_sub(to, from));
}

/**
 * @brief Linear interpolation between two vectors
 */
static inline sl_vec3_t sl_vec3_lerp(sl_vec3_t v0, sl_vec3_t v1, float t)
{
    return SL_VEC3(
        v0.x + (v1.x - v0.x) * t,
        v0.y + (v1.y - v0.y) * t,
        v0.z + (v1.z - v0.z) * t
    );
}

/**
 * @brief Move vector toward target by max_delta
 */
static inline sl_vec3_t sl_vec3_move_toward(sl_vec3_t from, sl_vec3_t to, float max_delta)
{
    sl_vec3_t delta = SL_VEC3(to.x - from.x, to.y - from.y, to.z - from.z);
    float dist = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

    if (dist <= max_delta || dist < 1e-6f)
        return to;

    float scale = max_delta / dist;
    return SL_VEC3(from.x + delta.x * scale, from.y + delta.y * scale, from.z + delta.z * scale);
}

/**
 * @brief Approximate equality within epsilon
 */
static inline int sl_vec3_approx(sl_vec3_t v0, sl_vec3_t v1, float epsilon)
{
    return (fabsf(v0.x - v1.x) < epsilon) &&
           (fabsf(v0.y - v1.y) < epsilon) &&
           (fabsf(v0.z - v1.z) < epsilon);
}

/**
 * @brief Reflect vector v around normal
 */
static inline sl_vec3_t sl_vec3_reflect(sl_vec3_t v, sl_vec3_t normal)
{
    return sl_vec3_sub(v, sl_vec3_scale(normal, 2.0f * sl_vec3_dot(v, normal)));
}

/**
 * @brief Project vector v onto another vector
 */
static inline sl_vec3_t sl_vec3_project(sl_vec3_t v, sl_vec3_t onto)
{
    return sl_vec3_scale(onto, sl_vec3_dot(v, onto) / sl_vec3_length_sq(onto));
}

/**
 * @brief Reject vector v from another vector (component perpendicular)
 */
static inline sl_vec3_t sl_vec3_reject(sl_vec3_t v, sl_vec3_t onto)
{
    return sl_vec3_sub(v, sl_vec3_project(v, onto));
}

/**
 * @brief Transform vector by 4x4 matrix (ignores w)
 */
static inline sl_vec3_t sl_vec3_transform(sl_vec3_t v, const sl_mat4_t* mat)
{
    sl_vec3_t result;
    result.x = mat->m00 * v.x + mat->m10 * v.y + mat->m20 * v.z + mat->m30;
    result.y = mat->m01 * v.x + mat->m11 * v.y + mat->m21 * v.z + mat->m31;
    result.z = mat->m02 * v.x + mat->m12 * v.y + mat->m22 * v.z + mat->m32;

    return result;
}

/** @} */ // Vec3

/* === 4D Vector Function === */

/**
 * @brief Swap two 4D vectors in place.
 */
static inline void sl_vec4_swap(sl_vec4_t* SL_RESTRICT a, sl_vec4_t* SL_RESTRICT b)
{
    for (int i = 0; i < 4; ++i) {
        float tmp = a->v[i];
        a->v[i] = b->v[i];
        b->v[i] = tmp;
    }
}

/**
 * @brief Clamp each component of vector x to be >= min.
 */
static inline sl_vec4_t sl_vec4_min(sl_vec4_t x, sl_vec4_t min)
{
    for (int i = 0; i < 4; ++i) {
        if (x.v[i] < min.v[i]) x.v[i] = min.v[i];
    }
    return x;
}

/**
 * @brief Clamp each component of vector x to be <= max.
 */
static inline sl_vec4_t sl_vec4_max(sl_vec4_t x, sl_vec4_t max)
{
    for (int i = 0; i < 4; ++i) {
        if (x.v[i] < max.v[i]) x.v[i] = max.v[i];
    }
    return x;
}

/**
 * @brief Clamp each component of vector x to [min,max].
 */
static inline sl_vec4_t sl_vec4_clamp(sl_vec4_t x, sl_vec4_t min, sl_vec4_t max)
{
    for (int i = 0; i < 4; ++i) {
        if (x.v[i] < min.v[i]) x.v[i] = min.v[i];
        else
        if (x.v[i] > max.v[i]) x.v[i] = max.v[i];
    }
    return x;
}

/**
 * @brief Absolute value of each component.
 */
static inline sl_vec4_t sl_vec4_abs(sl_vec4_t v)
{
    for (int i = 0; i < 4; ++i) {
        v.v[i] = fabsf(v.v[i]);
    }
    return v;
}

/**
 * @brief Negate each component.
 */
static inline sl_vec4_t sl_vec4_neg(sl_vec4_t v)
{
    for (int i = 0; i < 4; ++i) {
        v.v[i] = -v.v[i];
    }
    return v;
}

/**
 * @brief Add two vectors (component-wise).
 */
static inline sl_vec4_t sl_vec4_add(sl_vec4_t v1, sl_vec4_t v2)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] += v2.v[i];
    }
    return v1;
}

/**
 * @brief Subtract two vectors (component-wise).
 */
static inline sl_vec4_t sl_vec4_sub(sl_vec4_t v1, sl_vec4_t v2)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] -= v2.v[i];
    }
    return v1;
}

/**
 * @brief Multiply two vectors (component-wise).
 */
static inline sl_vec4_t sl_vec4_mul(sl_vec4_t v1, sl_vec4_t v2)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] *= v2.v[i];
    }
    return v1;
}

/**
 * @brief Divide two vectors (component-wise).
 */
static inline sl_vec4_t sl_vec4_div(sl_vec4_t v1, sl_vec4_t v2)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] /= v2.v[i];
    }
    return v1;
}

/**
 * @brief Add scalar to each component.
 */
static inline sl_vec4_t sl_vec4_offset(sl_vec4_t v, float scalar)
{
    for (int i = 0; i < 4; ++i) {
        v.v[i] += scalar;
    }
    return v;
}

/**
 * @brief Multiply each component by scalar.
 */
static inline sl_vec4_t sl_vec4_scale(sl_vec4_t v, float scalar)
{
    for (int i = 0; i < 4; ++i) {
        v.v[i] *= scalar;
    }
    return v;
}

/**
 * @brief Normalize vector (length = 1). Returns SL_VEC4_ZERO if length is too small.
 */
static inline sl_vec4_t sl_vec4_normalize(sl_vec4_t v)
{
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    if (len < 1e-4f) return SL_VEC4_ZERO;

    float inv_len = 1.0f / len;
    for (int i = 0; i < 4; ++i) {
        v.v[i] *= inv_len;
    }

    return v;
}

/**
 * @brief Compute vector length.
 */
static inline float sl_vec4_length(sl_vec4_t v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

/**
 * @brief Compute squared vector length.
 */
static inline float sl_vec4_length_sq(sl_vec4_t v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

/**
 * @brief Compute dot product between v1 and v2.
 */
static inline float sl_vec4_dot(sl_vec4_t v1, sl_vec4_t v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

/**
 * @brief Test approximate equality between two vectors, within epsilon tolerance.
 */
static inline int sl_vec4_approx(sl_vec4_t v0, sl_vec4_t v1, float epsilon)
{
    for (int i = 0; i < 4; ++i) {
        if (!(fabsf(v0.x - v1.x) < epsilon)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Move current vector toward target vector by at most max_delta.
 */
static inline sl_vec4_t sl_vec4_move_toward(sl_vec4_t current, sl_vec4_t target, float max_delta)
{
    sl_vec4_t delta;
    for (int i = 0; i < 4; ++i) {
        delta.v[i] = target.v[i] - current.v[i];
    }

    float dist = sqrtf(delta.x * delta.x + delta.y * delta.y);
    if (dist <= max_delta) {
        return target;
    }

    float ratio = max_delta / dist;
    for (int i = 0; i < 4; ++i) {
        current.v[i] = delta.v[i] * ratio;
    }

    return current;
}

/**
 * @brief Linear interpolation between v1 and v2.
 */
static inline sl_vec4_t sl_vec4_lerp(sl_vec4_t v1, sl_vec4_t v2, float t)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] += t * (v2.v[i] - v1.v[i]);
    }
    return v1;
}

/**
 * @brief Transform vector by a 4x4 matrix.
 */
static inline sl_vec4_t sl_vec4_transform(sl_vec4_t v, const sl_mat4_t* mat)
{
    sl_vec4_t result;
    result.x = mat->m00 * v.x + mat->m10 * v.y + mat->m20 * v.z + mat->m30 * v.w;
    result.y = mat->m01 * v.x + mat->m11 * v.y + mat->m21 * v.z + mat->m31 * v.w;
    result.z = mat->m02 * v.x + mat->m12 * v.y + mat->m22 * v.z + mat->m32 * v.w;
    result.w = mat->m03 * v.x + mat->m13 * v.y + mat->m23 * v.z + mat->m33 * v.w;

    return result;
}

/** @} */ // Vec4

/* === Quaternion Functions === */

/** @defgroup Quat Quaternion Functions
 *  Functions for quaternion creation, manipulation and interpolation.
 *  @{
 */

/**
 * @brief Create a quaternion from an axis and an angle in radians.
 */
static inline sl_quat_t sl_quat_from_axis_angle(sl_vec3_t axis, float radians)
{
    sl_quat_t result;

    float half = radians * 0.5f;
    float s = sinf(half);
    float c = cosf(half);

    result.w = c;
    result.x = axis.x * s;
    result.y = axis.y * s;
    result.z = axis.z * s;

    return result;
}

/**
 * @brief Create a quaternion from Euler angles (pitch, yaw, roll).
 */
SLAPI sl_quat_t sl_quat_from_euler(sl_vec3_t v);

/**
 * @brief Convert a quaternion to Euler angles (pitch, yaw, roll).
 */
SLAPI sl_vec3_t sl_quat_to_euler(sl_quat_t q);

/**
 * @brief Create a quaternion from a 4x4 rotation matrix.
 */
SLAPI sl_quat_t sl_quat_from_mat4(const sl_mat4_t* m);

/**
 * @brief Convert a quaternion to a 4x4 rotation matrix.
 */
SLAPI sl_mat4_t sl_quat_to_mat4(sl_quat_t q);

/**
 * @brief Compute the length (magnitude) of a quaternion.
 */
static inline float sl_quat_length(sl_quat_t q)
{
    return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

/**
 * @brief Normalize a quaternion to unit length.
 */
static inline sl_quat_t sl_quat_normalize(sl_quat_t q)
{
    float len_sq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (len_sq < 1e-4f) return SL_QUAT_IDENTITY;

    float inv_len = 1.0f / sqrtf(len_sq);
    for (int i = 0; i < 4; ++i) {
        q.v[i] *= inv_len;
    }

    return q;
}

/**
 * @brief Conjugate of a quaternion.
 * Equivalent to negating the vector part (x,y,z).
 */
static inline sl_quat_t sl_quat_conjugate(sl_quat_t q)
{
    return SL_QUAT(q.w, -q.x, -q.y, -q.z);
}

/**
 * @brief Inverse of a quaternion.
 */
static inline sl_quat_t sl_quat_inverse(sl_quat_t q)
{
    float len_sq = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;
    if (len_sq < 1e-4f) {
        return q;
    }

    float inv_len_sq = 1.0f / len_sq;

    q.w = q.w * +inv_len_sq;
    q.x = q.x * -inv_len_sq;
    q.y = q.y * -inv_len_sq;
    q.z = q.z * -inv_len_sq;

    return q;
}

/**
 * @brief Multiply two quaternions (Hamilton product).
 */
static inline sl_quat_t sl_quat_mul(sl_quat_t a, sl_quat_t b)
{
    sl_quat_t result;
    result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;

    return result;
}

/**
 * @brief Transform a quaternion by a 4x4 matrix.
 */
static inline sl_quat_t sl_quat_transform(sl_quat_t q, const sl_mat4_t* mat)
{
    sl_quat_t result;
    result.x = mat->m00 * q.x + mat->m01 * q.y + mat->m02 * q.z + mat->m30 * q.w;
    result.y = mat->m10 * q.x + mat->m11 * q.y + mat->m12 * q.z + mat->m31 * q.w;
    result.z = mat->m20 * q.x + mat->m21 * q.y + mat->m22 * q.z + mat->m32 * q.w;
    result.w = mat->m30 * q.x + mat->m31 * q.y + mat->m23 * q.z + mat->m33 * q.w;

    return result;
}

/**
 * @brief Linear interpolation between two quaternions.
 */
SLAPI sl_quat_t sl_quat_lerp(sl_quat_t a, sl_quat_t b, float t);

/**
 * @brief Spherical linear interpolation (slerp) between two quaternions.
 */
SLAPI sl_quat_t sl_quat_slerp(sl_quat_t a, sl_quat_t b, float t);

/** @} */ // Quat

/* === Color Functions === */

/** @defgroup Color Color Functions
 *  Inline utility functions for manipulating and converting colors.
 *  @{
 */

/**
 * @brief Add two colors component-wise (clamped to 255).
 */
static inline sl_color_t sl_color_add(sl_color_t c0, sl_color_t c1)
{
    return SL_COLOR(
        SL_MIN(c0.r + c1.r, 255),
        SL_MIN(c0.g + c1.g, 255),
        SL_MIN(c0.b + c1.b, 255),
        SL_MIN(c0.a + c1.a, 255)
    );
}

/**
 * @brief Subtract two colors component-wise (clamped to 0).
 */
static inline sl_color_t sl_color_sub(sl_color_t c0, sl_color_t c1)
{
    return SL_COLOR(
        SL_MAX(c0.r - c1.r, 0),
        SL_MAX(c0.g - c1.g, 0),
        SL_MAX(c0.b - c1.b, 0),
        SL_MAX(c0.a - c1.a, 0)
    );
}

/**
 * @brief Multiply two colors component-wise (normalized to [0,255]).
 */
static inline sl_color_t sl_color_mod(sl_color_t c0, sl_color_t c1)
{
    return SL_COLOR(
        ((c0.r * c1.r) / 255),
        ((c0.g * c1.g) / 255),
        ((c0.b * c1.b) / 255),
        ((c0.a * c1.a) / 255)
    );
}

/**
 * @brief Scale a color by a scalar factor (clamped to 255).
 */
static inline sl_color_t sl_color_scale(sl_color_t c, float s)
{
    return SL_COLOR(
        SL_MIN((int)(c.r * s), 255),
        SL_MIN((int)(c.g * s), 255),
        SL_MIN((int)(c.b * s), 255),
        SL_MIN((int)(c.a * s), 255)
    );
}

/**
 * @brief Blend between two colors using linear interpolation.
 * @param c0 First color
 * @param c1 Second color
 * @param t Interpolation factor in [0,1]
 */
static inline sl_color_t sl_color_blend(sl_color_t c0, sl_color_t c1, float t)
{
    return SL_COLOR(
        (uint8_t)(c0.r * (1.0f - t) + c1.r * t),
        (uint8_t)(c0.g * (1.0f - t) + c1.g * t),
        (uint8_t)(c0.b * (1.0f - t) + c1.b * t),
        (uint8_t)(c0.a * (1.0f - t) + c1.a * t)
    );
}

/**
 * @brief Convert HSV color (given as a vector) to RGB color.
 * @param hsv HSV vector: x = hue [0..360], y = saturation [0..1], z = value [0..1]
 * @return RGB color with full alpha (255)
 */
static inline sl_color_t sl_color_from_hsv_vec(sl_vec3_t hsv)
{
    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;

    float c = v * s;
    float h_prime = h / 60.0f;
    float x = c * (1.0f - fabsf(fmodf(h_prime, 2.0f) - 1.0f));
    float r1, g1, b1;

    if (h_prime < 0) h_prime = 0;
    if (h_prime >= 6) h_prime = 5.999f;

    if (h_prime < 1)       { r1 = c; g1 = x; b1 = 0; }
    else if (h_prime < 2)  { r1 = x; g1 = c; b1 = 0; }
    else if (h_prime < 3)  { r1 = 0; g1 = c; b1 = x; }
    else if (h_prime < 4)  { r1 = 0; g1 = x; b1 = c; }
    else if (h_prime < 5)  { r1 = x; g1 = 0; b1 = c; }
    else                   { r1 = c; g1 = 0; b1 = x; }

    float m = v - c;
    return SL_COLOR(
        (uint8_t)((r1 + m) * 255.0f),
        (uint8_t)((g1 + m) * 255.0f),
        (uint8_t)((b1 + m) * 255.0f),
        255
    );
}

/**
 * @brief Convert RGB color to HSV representation (as a vector).
 * @param c RGB color
 * @return HSV vector: x = hue [0..360], y = saturation [0..1], z = value [0..1]
 */
static inline sl_vec3_t sl_color_to_hsv_vec(sl_color_t c)
{
    float r = c.r / 255.0f;
    float g = c.g / 255.0f;
    float b = c.b / 255.0f;

    float max = fmaxf(r, fmaxf(g, b));
    float min = fminf(r, fminf(g, b));
    float delta = max - min;

    sl_vec3_t hsv;
    hsv.z = max; // V

    if (delta < 1e-6f) {
        hsv.x = 0.0f; // H
        hsv.y = 0.0f; // S
        return hsv;
    }

    hsv.y = delta / max; // S

    if (max == r) hsv.x = 60.0f * fmodf(((g - b) / delta), 6.0f);
    else if (max == g) hsv.x = 60.0f * (((b - r) / delta) + 2.0f);
    else hsv.x = 60.0f * (((r - g) / delta) + 4.0f);

    if (hsv.x < 0) hsv.x += 360.0f;

    return hsv;
}

/** @} */ // Color

/* === Matrix 4x4 Functions  */

/** @defgroup Mat4 Matrix 4x4 Functions
 *  Functions for creating and manipulating 4x4 matrices (sl_mat4_t).
 *  @{
 */

/**
 * @brief Create a translation matrix
 * @param v Translation vector
 * @return Translation matrix
 */
SLAPI sl_mat4_t sl_mat4_translate(sl_vec3_t v);

/**
 * @brief Create a rotation matrix around an arbitrary axis
 * @param axis Axis of rotation (normalized)
 * @param radians Angle in radians
 * @return Rotation matrix
 */
SLAPI sl_mat4_t sl_mat4_rotate(sl_vec3_t axis, float radians);

/**
 * @brief Create a rotation matrix around the X axis
 * @param radians Angle in radians
 * @return Rotation matrix
 */
SLAPI sl_mat4_t sl_mat4_rotate_x(float radians);

/**
 * @brief Create a rotation matrix around the Y axis
 * @param radians Angle in radians
 * @return Rotation matrix
 */
SLAPI sl_mat4_t sl_mat4_rotate_y(float radians);

/**
 * @brief Create a rotation matrix around the Z axis
 * @param radians Angle in radians
 * @return Rotation matrix
 */
SLAPI sl_mat4_t sl_mat4_rotate_z(float radians);

/**
 * @brief Create a rotation matrix from Euler angles (XYZ order)
 * @param radians Rotation angles for each axis (x, y, z)
 * @return Rotation matrix
 */
SLAPI sl_mat4_t sl_mat4_rotate_xyz(sl_vec3_t radians);

/**
 * @brief Create a rotation matrix from Euler angles (ZYX order)
 * @param radians Rotation angles for each axis (z, y, x)
 * @return Rotation matrix
 */
SLAPI sl_mat4_t sl_mat4_rotate_zyx(sl_vec3_t radians);

/**
 * @brief Create a scaling matrix
 * @param scale Scaling vector
 * @return Scaling matrix
 */
SLAPI sl_mat4_t sl_mat4_scale(sl_vec3_t scale);

/**
 * @brief Create a perspective frustum projection matrix
 */
SLAPI sl_mat4_t sl_mat4_frustum(float left, float right, float bottom, float top, float znear, float zfar);

/**
 * @brief Create a perspective projection matrix
 */
SLAPI sl_mat4_t sl_mat4_perspective(float fovy, float aspect, float znear, float zfar);

/**
 * @brief Create an orthographic projection matrix
 */
SLAPI sl_mat4_t sl_mat4_ortho(float left, float right, float bottom, float top, float znear, float zfar);

/**
 * @brief Create a look-at view matrix
 * @param eye Position of the camera
 * @param target Target point
 * @param up Up vector
 * @return View matrix
 */
SLAPI sl_mat4_t sl_mat4_look_at(sl_vec3_t eye, sl_vec3_t target, sl_vec3_t up);

/**
 * @brief Compute the determinant of a matrix
 * @param mat Pointer to the matrix
 * @return Determinant
 */
SLAPI float sl_mat4_determinant(const sl_mat4_t* mat);

/**
 * @brief Transpose a matrix
 */
SLAPI sl_mat4_t sl_mat4_transpose(const sl_mat4_t* mat);

/**
 * @brief Invert a matrix
 */
SLAPI sl_mat4_t sl_mat4_inverse(const sl_mat4_t* mat);

/**
 * @brief Compute the trace of a matrix
 * @param mat Pointer to the matrix
 * @return Trace (sum of diagonal elements)
 */
SLAPI float sl_mat4_trace(const sl_mat4_t* mat);

/**
 * @brief Add two matrices
 */
SLAPI sl_mat4_t sl_mat4_add(const sl_mat4_t* left, const sl_mat4_t* right);

/**
 * @brief Subtract two matrices
 */
SLAPI sl_mat4_t sl_mat4_sub(const sl_mat4_t* left, const sl_mat4_t* right);

/**
 * @brief Multiply two matrices
 */
SLAPI sl_mat4_t sl_mat4_mul(const sl_mat4_t* left, const sl_mat4_t* right);

/** @} */ // Mat4

#if defined(__cplusplus)
} // extern "C"
#endif

/* === C++ Operators === */

#if defined(__cplusplus)

inline sl_vec2 operator+(const sl_vec2& lhs, const sl_vec2& rhs)
{
    return sl_vec2_add(lhs, rhs);
}

inline const sl_vec2& operator+=(sl_vec2& lhs, const sl_vec2& rhs)
{
    lhs = sl_vec2_add(lhs, rhs);
    return lhs;
}

inline sl_vec2 operator-(const sl_vec2& lhs, const sl_vec2& rhs)
{
    return sl_vec2_sub(lhs, rhs);
}

inline const sl_vec2& operator-=(sl_vec2& lhs, const sl_vec2& rhs)
{
    lhs = sl_vec2_sub(lhs, rhs);
    return lhs;
}

inline sl_vec2 operator*(const sl_vec2& lhs, float rhs)
{
    return sl_vec2_scale(lhs, rhs);
}

inline const sl_vec2& operator*=(sl_vec2& lhs, float rhs)
{
    lhs = sl_vec2_scale(lhs, rhs);
    return lhs;
}

inline sl_vec2 operator*(const sl_vec2& lhs, const sl_vec2& rhs)
{
    return sl_vec2_mul(lhs, rhs);
}

inline const sl_vec2& operator*=(sl_vec2& lhs, const sl_vec2& rhs)
{
    lhs = sl_vec2_mul(lhs, rhs);
    return lhs;
}

inline sl_vec2 operator*(const sl_vec2& lhs, const sl_mat4& rhs)
{
    return sl_vec2_transform(lhs, &rhs);
}

inline const sl_vec2& operator*=(sl_vec2& lhs, const sl_mat4& rhs)
{
    lhs = sl_vec2_transform(lhs, &rhs);
    return lhs;
}

inline sl_vec2 operator/(const sl_vec2& lhs, float rhs)
{
    return sl_vec2_scale(lhs, 1.0f / rhs);
}

inline const sl_vec2& operator/=(sl_vec2& lhs, float rhs)
{
    lhs = sl_vec2_scale(lhs, 1.0f / rhs);
    return lhs;
}

inline sl_vec2 operator/(const sl_vec2& lhs, const sl_vec2& rhs)
{
    return sl_vec2_div(lhs, rhs);
}

inline const sl_vec2& operator/=(sl_vec2& lhs, const sl_vec2& rhs)
{
    lhs = sl_vec2_div(lhs, rhs);
    return lhs;
}

inline bool operator==(const sl_vec2& lhs, const sl_vec2& rhs)
{
    return sl_vec2_approx(lhs, rhs, 1e-6f);
}

inline bool operator!=(const sl_vec2& lhs, const sl_vec2& rhs)
{
    return !sl_vec2_approx(lhs, rhs, 1e-6f);
}

inline sl_vec2 operator+(const sl_vec2& lhs, float rhs)
{
    return sl_vec2_offset(lhs, rhs);
}

inline const sl_vec2& operator+=(sl_vec2& lhs, float rhs)
{
    lhs = sl_vec2_offset(lhs, rhs);
    return lhs;
}

inline sl_vec2 operator-(const sl_vec2& lhs, float rhs)
{
    return sl_vec2_offset(lhs, -rhs);
}

inline const sl_vec2& operator-=(sl_vec2& lhs, float rhs)
{
    lhs = sl_vec2_offset(lhs, -rhs);
    return lhs;
}

inline sl_vec3 operator+(const sl_vec3& lhs, const sl_vec3& rhs)
{
    return sl_vec3_add(lhs, rhs);
}

inline const sl_vec3& operator+=(sl_vec3& lhs, const sl_vec3& rhs)
{
    lhs = sl_vec3_add(lhs, rhs);
    return lhs;
}

inline sl_vec3 operator-(const sl_vec3& lhs, const sl_vec3& rhs)
{
    return sl_vec3_sub(lhs, rhs);
}

inline const sl_vec3& operator-=(sl_vec3& lhs, const sl_vec3& rhs)
{
    lhs = sl_vec3_sub(lhs, rhs);
    return lhs;
}

inline sl_vec3 operator*(const sl_vec3& lhs, float rhs)
{
    return sl_vec3_scale(lhs, rhs);
}

inline const sl_vec3& operator*=(sl_vec3& lhs, float rhs)
{
    lhs = sl_vec3_scale(lhs, rhs);
    return lhs;
}

inline sl_vec3 operator*(const sl_vec3& lhs, const sl_vec3& rhs)
{
    return sl_vec3_mul(lhs, rhs);
}

inline const sl_vec3& operator*=(sl_vec3& lhs, const sl_vec3& rhs)
{
    lhs = sl_vec3_mul(lhs, rhs);
    return lhs;
}

inline sl_vec3 operator*(const sl_vec3& lhs, const sl_mat4& rhs)
{
    return sl_vec3_transform(lhs, &rhs);
}

inline const sl_vec3& operator*=(sl_vec3& lhs, const sl_mat4& rhs)
{
    lhs = sl_vec3_transform(lhs, &rhs);
    return lhs;
}

inline sl_vec3 operator/(const sl_vec3& lhs, float rhs)
{
    return sl_vec3_scale(lhs, 1.0f / rhs);
}

inline const sl_vec3& operator/=(sl_vec3& lhs, float rhs)
{
    lhs = sl_vec3_scale(lhs, 1.0f / rhs);
    return lhs;
}

inline sl_vec3 operator/(const sl_vec3& lhs, const sl_vec3& rhs)
{
    return sl_vec3_div(lhs, rhs);
}

inline const sl_vec3& operator/=(sl_vec3& lhs, const sl_vec3& rhs)
{
    lhs = sl_vec3_div(lhs, rhs);
    return lhs;
}

inline bool operator==(const sl_vec3& lhs, const sl_vec3& rhs)
{
    return sl_vec3_approx(lhs, rhs, 1e-6f);
}

inline bool operator!=(const sl_vec3& lhs, const sl_vec3& rhs)
{
    return !sl_vec3_approx(lhs, rhs, 1e-6f);
}

inline sl_vec3 operator+(const sl_vec3& lhs, float rhs)
{
    return sl_vec3_offset(lhs, rhs);
}

inline const sl_vec3& operator+=(sl_vec3& lhs, float rhs)
{
    lhs = sl_vec3_offset(lhs, rhs);
    return lhs;
}

inline sl_vec3 operator-(const sl_vec3& lhs, float rhs)
{
    return sl_vec3_offset(lhs, -rhs);
}

inline const sl_vec3& operator-=(sl_vec3& lhs, float rhs)
{
    lhs = sl_vec3_offset(lhs, -rhs);
    return lhs;
}

inline sl_vec4 operator+(const sl_vec4& lhs, const sl_vec4& rhs)
{
    return sl_vec4_add(lhs, rhs);
}

inline const sl_vec4& operator+=(sl_vec4& lhs, const sl_vec4& rhs)
{
    lhs = sl_vec4_add(lhs, rhs);
    return lhs;
}

inline sl_vec4 operator-(const sl_vec4& lhs, const sl_vec4& rhs)
{
    return sl_vec4_sub(lhs, rhs);
}

inline const sl_vec4& operator-=(sl_vec4& lhs, const sl_vec4& rhs)
{
    lhs = sl_vec4_sub(lhs, rhs);
    return lhs;
}

inline sl_vec4 operator*(const sl_vec4& lhs, float rhs)
{
    return sl_vec4_scale(lhs, rhs);
}

inline const sl_vec4& operator*=(sl_vec4& lhs, float rhs)
{
    lhs = sl_vec4_scale(lhs, rhs);
    return lhs;
}

inline sl_vec4 operator*(const sl_vec4& lhs, const sl_vec4& rhs)
{
    return sl_vec4_mul(lhs, rhs);
}

inline const sl_vec4& operator*=(sl_vec4& lhs, const sl_vec4& rhs)
{
    lhs = sl_vec4_mul(lhs, rhs);
    return lhs;
}

inline sl_vec4 operator/(const sl_vec4& lhs, float rhs)
{
    return sl_vec4_scale(lhs, 1.0f / rhs);
}

inline const sl_vec4& operator/=(sl_vec4& lhs, float rhs)
{
    lhs = sl_vec4_scale(lhs, 1.0f / rhs);
    return lhs;
}

inline sl_vec4 operator/(const sl_vec4& lhs, const sl_vec4& rhs)
{
    return sl_vec4_div(lhs, rhs);
}

inline const sl_vec4& operator/=(sl_vec4& lhs, const sl_vec4& rhs)
{
    lhs = sl_vec4_div(lhs, rhs);
    return lhs;
}

inline bool operator==(const sl_vec4& lhs, const sl_vec4& rhs)
{
    return sl_vec4_approx(lhs, rhs, 1e-6f);
}

inline bool operator!=(const sl_vec4& lhs, const sl_vec4& rhs)
{
    return !sl_vec4_approx(lhs, rhs, 1e-6f);
}

inline sl_vec4 operator+(const sl_vec4& lhs, float rhs)
{
    return sl_vec4_offset(lhs, rhs);
}

inline const sl_vec4& operator+=(sl_vec4& lhs, float rhs)
{
    lhs = sl_vec4_offset(lhs, rhs);
    return lhs;
}

inline sl_vec4 operator-(const sl_vec4& lhs, float rhs)
{
    return sl_vec4_offset(lhs, -rhs);
}

inline const sl_vec4& operator-=(sl_vec4& lhs, float rhs)
{
    lhs = sl_vec4_offset(lhs, -rhs);
    return lhs;
}

inline sl_quat operator*(const sl_quat& lhs, const sl_mat4& rhs)
{
    return sl_quat_transform(lhs, &rhs);
}

inline const sl_quat& operator*=(sl_quat& lhs, const sl_mat4& rhs)
{
    lhs = sl_quat_transform(lhs, &rhs);
    return lhs;
}

inline sl_mat4 operator+(const sl_mat4& lhs, const sl_mat4& rhs)
{
    return sl_mat4_add(&lhs, &rhs);
}

inline const sl_mat4& operator+=(sl_mat4& lhs, const sl_mat4& rhs)
{
    lhs = sl_mat4_add(&lhs, &rhs);
    return lhs;
}

inline sl_mat4 operator-(const sl_mat4& lhs, const sl_mat4& rhs)
{
    return sl_mat4_sub(&lhs, &rhs);
}

inline const sl_mat4& operator-=(sl_mat4& lhs, const sl_mat4& rhs)
{
    lhs = sl_mat4_sub(&lhs, &rhs);
    return lhs;
}

inline sl_mat4 operator*(const sl_mat4& lhs, const sl_mat4& rhs)
{
    return sl_mat4_mul(&lhs, &rhs);
}

inline const sl_mat4& operator*=(sl_mat4& lhs, const sl_mat4& rhs)
{
    lhs = sl_mat4_mul(&lhs, &rhs);
    return lhs;
}

#endif // __cplusplus

#endif // SL_H

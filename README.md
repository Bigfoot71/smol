# Smol

Smol is not really *small* itself, but it is perfect for *small things*.

It is a C11 library designed for making 2D games and prototypes.

The main goal is wide compatibility, so the rendering layer uses OpenGL ES2 with SDL3.  
Using ANGLE as a backend can be automated as well.  

> [!WARNING]
> The library is still highly experimental!
> Currently this has only been tested on Linux, no web, no Windows.
> So I do not recommend using Smol in production unless you enjoy surprises.

> [!NOTE]
> Smol is primarily developed for my own personal needs.
> I decided to share it so others may benefit, but any new features or changes will always be motivated by what I personally require first.

## Features

### Rendering

- Batched rendering of 2D primitives  
- Textures can be applied to ALL drawable primitives  
- Font loading and text rendering  
- Transformation matrix stack support  
- Custom shaders (similar to Löve2D), where you only need to define:
  ```glsl
  vec4 vertex(mat4 mvp, vec3 position);
  vec4 pixel(vec4 color, sampler2D tex, vec2 uv, vec2 screen_pos);
  ```
- Canvas support (framebuffers)
- Mesh support (single attribute pattern `sl_vertex`: position, texcoord, normal, color)
- Simplified stencil support
- Control over depth test/write/range, and even viewport, scissors, etc

### Audio

- Support for sound and music loading (WAV/FLAC/MP3/OGG)
- Built on [mojoAL](https://github.com/icculus/mojoAL) (OpenAL 1.1)
- Polyphony for sounds (choose the number of 'channels' at load time)
- Music decoding is asynchronous and automatically updated
- Volume control for sounds, music, and a master factor (all globaly applied)

### Resources

Most resources in Smol are managed by **ID handles**:
  - `sl_texture_id`
  - `sl_canvas_id`
  - `sl_shader_id`
  - `sl_mesh_id`
  - `sl_font_id`
  - `sl_sound_id`
  - `sl_music_id`

These resources are automatically destroyed on shutdown, so you don’t need to free them manually.

**Exception:** `sl_image_t` is not ID-based and must be freed explicitly, as images are often used for intermediate manipulation and need more flexibility.

### Math and Utilities

- General math and helper functions
- Easing functions (all the classics are included)
- Vectors
- Quaternions
- 4x4 matrices
- String manipulation utilities
- PCG32 based random number generator

## Philosophy

Smol is a small *...* toolkit that gathers many of the things you usually need to build a **prototype**, a **2D game**, or just a small program.

What Smol will not include:

- Physics
- A full 3D engine
- Highly specialized features

These depend too much on the type of project you want to build. The goal is to keep Smol simple, flexible, and portable.

## About 3D

Yes, you can technically do 3D with Smol.
There is even an example with a cube and basic lighting.
However, you will quickly hit limitations.

This library was designed with **2D in mind**.
If you want complete support for mesh loading, 3D asset handling, or advanced rendering, you will have to extend it yourself.

## In short

Smol provides:

* A collection of small, useful building blocks
* A straightforward approach to 2D rendering and utilities
* Enough to get prototypes running quickly

And if it is not enough for you... well, you can always look elsewhere! :p

## License

This project is licensed under the **zlib license**.

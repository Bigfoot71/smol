#ifdef PIXEL

// If GL_OES_standard_derivatives is not available you could
// replace 'fwidth(sdf)' with a fixed value (e.g., 0.04) or
// calculate the gradient manually using neighboring pixels

#extension GL_OES_standard_derivatives : enable

vec4 pixel(vec4 color, sampler2D tex, vec2 uv, vec2 screen_pos)
{
    float sdf = texture2D(tex, uv).r;

    float smoothing = fwidth(sdf);
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, sdf);

    return color * alpha; // the blend mode used for fonts is alpha pre-multiplied
}

#endif

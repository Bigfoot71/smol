#if defined(VERTEX)

uniform mat4 u_mat_normal;

vec4 vertex(mat4 mvp, vec3 position)
{
    v_normal = (u_mat_normal * vec4(v_normal, 0.0)).xyz;
    return mvp * vec4(position, 1.0);
}

#elif defined(PIXEL)

uniform vec3 u_light_pos;
uniform vec3 u_view_pos;

vec4 pixel(vec4 color, sampler2D tex, vec2 uv, vec2 screen_pos)
{
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_light_pos - v_position);
    vec3 V = normalize(u_view_pos - v_position);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 32.0);

    vec3 texColor = texture2D(tex, uv).rgb;
    vec3 ambient  = color.rgb * 0.1 * texColor;
    vec3 diffuse  = color.rgb * diff * texColor;
    vec3 specular = spec * vec3(1.0);

    vec3 finalColor = ambient + diffuse + specular;

    return vec4(finalColor, color.a);
}

#endif

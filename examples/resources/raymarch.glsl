#ifdef PIXEL

/* === Uniforms === */

uniform float u_time;

/* === Constants === */

#define CAM_MIN_DIST 4.0
#define CAM_MAX_DIST 6.0
#define MAX_STEPS 64
#define MAX_DIST 15.0
#define MIN_DIST 0.01
#define MAX_REFLECTIONS 2

#define MAT_BLOB 0
#define MAT_MIRROR 1

/* === Blending === */

float smin(float a, float b, float k)
{
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}

/* === Scene === */

struct Hit {
    float dist;
    int material;
};

Hit scene(vec3 pos)
{
    Hit result;

    /* --- Blobs --- */

    float blob = length(pos) - 1.2;

    for(int i = 0; i < 3; i++)
    {
        float angle = float(i) * 2.094 + u_time * 0.5;

        float radiusPhase = u_time * 0.8 + float(i) * 1.2;
        float radius = 1.0 + 1.5 * sin(radiusPhase);
        
        vec3 bubblePos = vec3(
            cos(angle) * radius,
            sin(u_time * 0.4 + float(i)) * 0.8,
            sin(angle) * radius
        );
        
        float bubble = length(pos - bubblePos) - 0.5;
        blob = smin(blob, bubble, 0.6);
    }

    /* --- Choose blob or mirror (nearest) --- */

    float ground = pos.y + 2.5;

    if (blob < ground) {
        result.dist = blob;
        result.material = MAT_BLOB;
    } else {
        result.dist = ground;
        result.material = MAT_MIRROR;
    }

    return result;
}

float quickScene(vec3 pos)
{
    return scene(pos).dist;
}

vec3 getNormal(vec3 p)
{
    vec2 e = vec2(0.002, 0.0);
    return normalize(vec3(
        quickScene(p + e.xyy) - quickScene(p - e.xyy),
        quickScene(p + e.yxy) - quickScene(p - e.yxy),
        quickScene(p + e.yyx) - quickScene(p - e.yyx)
    ));
}

/* === Raymarching === */

struct RayHit {
    bool hit;
    float dist;
    vec3 pos;
    vec3 normal;
    int material;
};

RayHit rayMarch(vec3 ro, vec3 rd)
{
    RayHit result;
    result.hit = false;

    float dO = 0.0;
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ro + rd * dO;
        Hit sceneHit = scene(p);
        dO += sceneHit.dist;

        if (abs(sceneHit.dist) < MIN_DIST) {
            result.hit = true;
            result.dist = dO;
            result.pos = p;
            result.normal = getNormal(p);
            result.material = sceneHit.material;
            break;
        }

        if (dO > MAX_DIST) break;
    }

    return result;
}

/* === Lighting === */

vec3 lighting(vec3 pos, vec3 normal, vec3 viewDir, int material)
{
    vec3 lightDir = normalize(vec3(2.0, 3.0, 1.0));
    vec3 lightColor = vec3(1.0, 0.9, 0.8);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 color = vec3(0.0);

    if (material == MAT_BLOB) {
        float phase = u_time * 0.5 + pos.x + pos.z;
        vec3 baseColor = vec3(
            0.6 + 0.4 * sin(phase),
            0.3 + 0.3 * sin(phase + 2.0),
            0.8 + 0.2 * sin(phase + 4.0)
        );
        color = baseColor * (0.3 + 0.7 * diff) + vec3(0.5) * spec * 0.5;
    }
    else if (material == MAT_MIRROR) {
        color = vec3(0.15, 0.15, 0.25) * diff;
    }

    return color;
}

/* === Program === */

vec4 pixel(vec4 color, sampler2D tex, vec2 uv, vec2 screen_pos)
{
    vec2 st = vec2(uv.x, 1.0 - uv.y) * 2.0 - 1.0;

    float dist = mix(CAM_MIN_DIST, CAM_MAX_DIST, 0.5 + sin(u_time) * 0.5);
    float angle = u_time * 0.3;

    vec3 camPos = vec3(dist * cos(angle), 1.0, dist * sin(angle));
    vec3 camTar = vec3(0.0, -0.5, 0.0);

    vec3 forward = normalize(camTar - camPos);
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = normalize(cross(right, forward));

    vec3 rd = normalize(forward + st.x * right + st.y * up);

    vec3 finalColor = vec3(0.0);
    vec3 rayOrigin = camPos;
    vec3 rayDir = rd;
    float reflectionStrength = 1.0;

    RayHit hit1 = rayMarch(rayOrigin, rayDir);

    if (hit1.hit) {
        vec3 viewDir = normalize(rayOrigin - hit1.pos);
        vec3 color1 = lighting(hit1.pos, hit1.normal, viewDir, hit1.material);
        finalColor += color1 * reflectionStrength;

        // Reflection (on the mirror)
        if (hit1.material == MAT_MIRROR) {
            rayOrigin = hit1.pos + hit1.normal * 0.02;
            rayDir = reflect(rayDir, hit1.normal);
            reflectionStrength = 0.8;

            // Second ray (reflection)
            RayHit hit2 = rayMarch(rayOrigin, rayDir);

            if (hit2.hit) {
                vec3 viewDir2 = normalize(rayOrigin - hit2.pos);
                vec3 color2 = lighting(hit2.pos, hit2.normal, viewDir2, hit2.material);
                finalColor += color2 * reflectionStrength;
            }
            else {
                // sky
                finalColor += vec3(0.2, 0.3, 0.5) * reflectionStrength;
            }
        }
    }
    else {
        // sky
        finalColor = vec3(0.2, 0.3, 0.5);
    }

    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    return vec4(finalColor, 1.0);
}

#endif
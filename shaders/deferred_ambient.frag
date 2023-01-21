#version 450

#include "lib/lighting.glsl"
#include "lib/gbuffer.glsl"
#include "lib/ibl.glsl"
#include "lib/shadow.glsl"

// -------------------------------- DEFINES --------------------------------

#define USE_IBL
#define USE_AO

// -------------------------------- I/O --------------------------------

layout(set = 0, binding = 0) uniform sampler2D in_depth;
layout(set = 0, binding = 1) uniform sampler2D in_rt0;
layout(set = 0, binding = 2) uniform sampler2D in_rt1;
layout(set = 0, binding = 3) uniform sampler2DShadow in_shadows;
layout(set = 0, binding = 4) uniform sampler2D in_ao;

layout(set = 0, binding = 5) uniform samplerCube in_envmap;
layout(set = 0, binding = 6) uniform sampler2D brdf_lut;

layout(set = 0, binding = 7) uniform CameraData {
    Camera camera;
};

layout(set = 0, binding = 8) readonly buffer Lights {
    DirectionalLight lights[];
};

layout(set = 0, binding = 9) readonly buffer Shadows {
    ShadowMapParams shadow_params[];
};

layout(set = 0, binding = 10) uniform Params {
    uint light_count;
    uint display_sky;
    float ibl_intensity;
    float padding_;
};

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;


// -------------------------------- MAIN --------------------------------

float ambient_occlusion() {
#ifdef USE_AO
    return texture(in_ao, in_uv).r;
#else
    return 1.0;
#endif
}

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    const float depth = texelFetch(in_depth, coord, 0).x;
    vec3 irradiance = vec3(0.0);

    if(is_OOB(depth)) {
#if defined(USE_IBL)
        if(display_sky != 0) {
            const vec3 forward = normalize(unproject(in_uv, 1.0, camera.inv_view_proj) - camera.position);
            irradiance = texture(in_envmap, forward).rgb;
        }
#endif
    } else {
        const SurfaceInfo surface = read_gbuffer(texelFetch(in_rt0, coord, 0), texelFetch(in_rt1, coord, 0));

        const vec3 world_pos = unproject(in_uv, depth, camera.inv_view_proj);
        const vec3 depth_normal = compute_depth_normal(world_pos, surface);
        const vec3 view_dir = normalize(camera.position - world_pos);

        // directional lights
        for(uint i = 0; i != light_count; ++i) {
            const DirectionalLight light = lights[i];
            const vec3 light_dir = light.direction; // assume normalized

            float att = 1.0;
            if(light.shadow_map_index < 0xFFFFFFFF) {
                const ShadowMapParams params = shadow_params[light.shadow_map_index];
                const float bias = compute_automatic_bias(params, depth_normal, light_dir);
                att = compute_shadow_pcf(in_shadows, params, world_pos, bias);
            }

            if(att > 0.0) {
                const vec3 radiance = light.color * att;
                irradiance += radiance * L0(light_dir, view_dir, surface);
            }
        }

#ifdef USE_IBL
        irradiance += eval_ibl(in_envmap, brdf_lut, view_dir, surface) * ambient_occlusion() * ibl_intensity;
#endif
    }

    out_color = vec4(irradiance, 1.0);
}


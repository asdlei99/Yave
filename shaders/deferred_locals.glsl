#include "lib/compute_derivatives.glsl"

#include "lib/lighting.glsl"
#include "lib/gbuffer.glsl"
#include "lib/shadow.glsl"
#include "lib/debug.glsl"


// -------------------------------- DEFINES --------------------------------

// #define DEBUG


// -------------------------------- I/O --------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 0, binding = 0) uniform sampler2D in_depth;
layout(set = 0, binding = 1) uniform sampler2D in_rt0;
layout(set = 0, binding = 2) uniform sampler2D in_rt1;
layout(set = 0, binding = 3) uniform sampler2DShadow in_shadows;

layout(set = 0, binding = 4) uniform CameraData {
    Camera camera;
};

layout(set = 0, binding = 5) readonly buffer PointLights {
    PointLight point_lights[];
};

layout(set = 0, binding = 6) readonly buffer SpotLights {
    SpotLight spot_lights[];
};

layout(set = 0, binding = 7) readonly buffer Shadows {
    ShadowMapParams shadow_params[];
};

layout(r11f_g11f_b10f, set = 0, binding = 8) uniform image2D out_color;

layout(set = 1, binding = 0) uniform LightCount_Inline {
    uint point_count;
    uint spot_count;
};


// -------------------------------- SHARED --------------------------------

shared uint light_indices[max_tile_lights];
shared uint tile_point_count;
shared uint tile_spot_count;

shared vec3 tile_dir;
shared float tile_sin_alpha;

shared uvec2 tile_dist_bounds;


// -------------------------------- HELPERS --------------------------------

uvec2 dispatch_range(uint num) {
    const uint thread_count = gl_WorkGroupSize.x * gl_WorkGroupSize.y;
    const uint range = 1 + (num / thread_count);
    const uint begin = min(gl_LocalInvocationIndex * range, num);
    const uint end = min(begin + range, num);
    return uvec2(begin, end);
}


// -------------------------------- TILE INFOS --------------------------------

void compute_tile_culling_info(vec2 image_size) {
    const vec2 corner_uv = (gl_WorkGroupID.xy * gl_WorkGroupSize.xy) / image_size;
    const vec3 corner_unproj = unproject_ndc(vec3(corner_uv * 2.0 - 1.0, 1.0), camera.inv_view_proj);
    const vec3 corner_dir = normalize(corner_unproj - camera.position);


    const vec2 center_uv = ((gl_WorkGroupID.xy + 0.5) * gl_WorkGroupSize.xy) / image_size;
    const vec3 unproj = unproject_ndc(vec3(center_uv * 2.0 - 1.0, 1.0), camera.inv_view_proj);
    tile_dir = normalize(unproj - camera.position);

    const float tile_cos_alpha = dot(tile_dir, corner_dir);
    tile_sin_alpha = sqrt(1.0 - sqr(tile_cos_alpha));
}

void compute_tile_infos(vec2 image_size, float view_dist, bool is_oob) {
    if(gl_LocalInvocationIndex == 0) {
        compute_tile_culling_info(image_size);
        tile_point_count = 0;
        tile_spot_count = 0;
        tile_dist_bounds =  uvec2(floatBitsToUint(max_float), 0);
    }

    barrier();

    if(!is_oob) {
        const uint ui_dist = floatBitsToUint(view_dist);
        atomicMin(tile_dist_bounds.x, ui_dist);
        atomicMax(tile_dist_bounds.y, ui_dist);
    }

    barrier();
}


// -------------------------------- CULLING --------------------------------

bool is_inside(PointLight light) {
    const vec3 to_light = light.position - camera.position;
    const float cos_alpha = max(0.0, dot(to_light, tile_dir));

    if(cos_alpha + light.range < uintBitsToFloat(tile_dist_bounds.x) ||
       cos_alpha - light.range > uintBitsToFloat(tile_dist_bounds.y)) {
        return false;
    }

    const vec3 closest = camera.position + tile_dir * cos_alpha;
    const float tile_radius = cos_alpha * tile_sin_alpha;
    const vec3 to_closest = closest - light.position;
    return dot(to_closest, to_closest) <= sqr(light.range + tile_radius);
}

bool is_inside(SpotLight light) {
    const float cos_alpha = max(0.0, dot(light.encl_sphere_center - camera.position, tile_dir));

    if(cos_alpha + light.encl_sphere_radius < uintBitsToFloat(tile_dist_bounds.x) ||
       cos_alpha - light.encl_sphere_radius > uintBitsToFloat(tile_dist_bounds.y)) {
        return false;
    }

    // https://iquilezles.org/articles/intersectors/
    // return iRoundedCone(camera.position, tile_dir, light.position, light.position + light.forward * light.range, tile_radius, light.sin_angle * light.range + tile_radius);

    // Doesn't cull "behind" the cone
    // We still run the bound test on the enclosing sphere, but it's not great

    const float tile_radius = (cos_alpha + light.encl_sphere_radius) * tile_sin_alpha;
    const vec3 begin = light.position;
    const vec3 end = light.position + light.forward * light.range;
    const float begin_radius = tile_radius;
    const float end_radius = light.sin_angle * light.range + tile_radius;
    const vec3 cone_vec = end - begin;
    const vec3 to_begin = camera.position - begin;
    const vec3 to_end = camera.position - end;
    const float taper = begin_radius - end_radius;

    const float m0 = dot(cone_vec, cone_vec);
    const float m1 = dot(cone_vec, to_begin);
    const float m2 = dot(cone_vec, tile_dir);
    const float m3 = dot(to_begin, tile_dir);
    const float m5 = dot(to_begin, to_begin);
    const float m6 = dot(to_end, tile_dir);
    const float m7 = dot(to_end, to_end);

    const float d2 = m0 - sqr(taper);
    const float k2 = d2 - m2 * m2;
    const float k1 = d2 * m3 - m1 * m2 + m2 * taper * begin_radius;
    const float k0 = d2 * m5 - m1 * m1 + m1 * taper * begin_radius * 2.0 - m0 * sqr(begin_radius);
    const float h = sqr(k1) - k0 * k2;
    if(h < 0.0) {
        return false;
    }

    const float t = (-sqrt(h) - k1) / k2;
    if(t > uintBitsToFloat(tile_dist_bounds.y)) {
        return false;
    }

    const float y = m1 - begin_radius * taper + t * m2;
    if(y > 0.0 && y < d2) {
        return true;
    }

    const float h1 = sqr(m3) - m5 + sqr(begin_radius);
    const float h2 = sqr(m6) - m7 + sqr(end_radius);
    return max(h1, h2) > 0.0;
}


void cull_points() {
    const uvec2 range = dispatch_range(point_count);

    for(uint i = range.x; i != range.y; ++i) {
        if(is_inside(point_lights[i])) {
            uint light_index = atomicAdd(tile_point_count, 1);
            if(light_index >= max_tile_lights) {
                break;
            }
            light_indices[light_index] = i;
        }
    }
}

void cull_spots() {
    const uvec2 range = dispatch_range(spot_count);

    for(uint i = range.x; i != range.y; ++i) {
        if(is_inside(spot_lights[i])) {
            uint light_index = atomicAdd(tile_spot_count, 1) + tile_point_count;
            if(light_index >= max_tile_lights) {
                break;
            }
            light_indices[light_index] = i;
        }
    }
}

void cull_lights() {
    cull_points();
    barrier();

    cull_spots();
    barrier();
}


// -------------------------------- MAIN --------------------------------

void main() {
    const ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    const vec2 image_size = vec2(imageSize(out_color).xy);
    const vec2 uv = vec2(gl_GlobalInvocationID.xy) / image_size;

    const float depth = texelFetch(in_depth, coord, 0).x;

    const vec3 world_pos = unproject(uv, depth, camera.inv_view_proj);

    vec3 view_dir = (camera.position - world_pos);
    const float view_dist = length(view_dir);
    view_dir /= view_dist;

    compute_tile_infos(image_size, view_dist, is_OOB(depth));

    cull_lights();

    if(is_OOB(depth)) {
        return;
    }

    vec3 irradiance = imageLoad(out_color, coord).rgb;

    const SurfaceInfo surface = read_gbuffer(texelFetch(in_rt0, coord, 0), texelFetch(in_rt1, coord, 0));


    // -------------------------------- POINTS --------------------------------
    const uint point_begin = 0;
    const uint point_end = min(max_tile_lights, tile_point_count);
    for(uint i = point_begin; i != point_end; ++i) {
        const PointLight light = point_lights[light_indices[i]];
        const AreaLightInfo area = karis_area_light(surface, light, world_pos, view_dir);
        const float att = attenuation(area.orig_light_dist * light.falloff, light.range * light.falloff, light.min_radius * light.falloff);

        if(att > 0.0) {
            const vec3 radiance = light.color * att;
            irradiance += radiance * L0(area.light_dir, view_dir, alpha_corrected(surface, area));
        }
    }

    // -------------------------------- SPOTS --------------------------------
    const uint spot_begin = tile_point_count;
    const uint spot_end = min(max_tile_lights, spot_begin + tile_spot_count);
    for(uint i = spot_begin; i != spot_end; ++i) {
        const SpotLight light = spot_lights[light_indices[i]];
        const AreaLightInfo area = karis_area_light(surface, light, world_pos, view_dir);
        float att = attenuation(area.orig_light_dist * light.falloff, light.range * light.falloff, light.min_radius * light.falloff);

        if(att > 0.0) {
            const float spot_cos_alpha = max(0.0, -dot(area.light_dir, light.forward));
            att *= spot_attenuation(spot_cos_alpha, light.att_scale_offset);
        }

        if(att > 0.0 && light.shadow_map_index < 0xFFFFFFFF) {
            const ShadowMapParams params = shadow_params[light.shadow_map_index];
            att *= compute_shadow_pcf(in_shadows, params, world_pos);
        }

        if(att > 0.0) {
            const vec3 radiance = light.color * att;
            irradiance += radiance * L0(area.light_dir, view_dir, alpha_corrected(surface, area));
        }
    }

#ifdef DEBUG
    {
        const float total_lights = float(tile_point_count + tile_spot_count);
        vec3 heat = heat_spectrum(total_lights / 16.0f);
        heat = mix(heat, vec3(1.0) - heat, print_value(gl_LocalInvocationID.xy * 2.0, vec2(0.0), vec2(8.0, 15.0), total_lights, 2.0, 0.0));
        irradiance = mix(heat, irradiance, 0.8);
    }
#endif

    imageStore(out_color, coord, vec4(irradiance, 1.0));
}


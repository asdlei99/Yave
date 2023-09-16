#ifndef BRDF_GLSL
#define BRDF_GLSL

#include "utils.glsl"


// -------------------------------- FRESNEL --------------------------------

vec3 F_Schlick(float cos_theta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

vec3 F_Schlick(float cos_theta, vec3 F0, float roughness) {
    const float a = 1.0 - roughness;
    return F0 + (max(vec3(a), F0) - F0) * pow(1.0 - cos_theta, 5.0);
}


// -------------------------------- VISIBILITY --------------------------------

float V_Smith(float NoV, float NoL, float sqr_alpha) {
    const float V_Smith_V = NoV + sqrt(NoV * (NoV - NoV * sqr_alpha) + sqr_alpha);
    const float V_Smith_L = NoL + sqrt(NoL * (NoL - NoL * sqr_alpha) + sqr_alpha);
    return 1.0 / (V_Smith_V * V_Smith_L);
}

float V_Smith_approx(float NoV, float NoL, float alpha) {
    const float V_Smith_V = NoL * (NoV * (1.0 - alpha) + alpha);
    const float V_Smith_L = NoV * (NoL * (1.0 - alpha) + alpha);
    return 0.5 / min(epsilon, V_Smith_V + V_Smith_L);
}


// -------------------------------- GEOMETRY --------------------------------

float G_Schlick_GGX(float NoV, float alpha) {
    const float k = alpha * 0.5;
    const float denom = NoV * (1.0 - k) + k;
    return NoV / denom;
}

float G_Smith(float NoV, float NoL, float alpha) {
    return G_Schlick_GGX(NoV, alpha) * G_Schlick_GGX(NoL, alpha);
}


// -------------------------------- DISTRIB --------------------------------

float D_GGX(float NoH, float sqr_alpha) {
    const float denom = (NoH * sqr_alpha - NoH) * NoH + 1.0;
    return sqr_alpha / (pi * sqr(denom));
}


// -------------------------------- DIFFUSE -------------------------------

vec3 Lambert_diffuse_brdf(SurfaceInfo surface) {
    return surface.albedo.rgb * inv_pi;
}

vec3 OrenNayar_diffuse_brdf(SurfaceInfo surface, float NoV, float NoL, float VoH) {
    const float a2 = surface.sqr_alpha;
    float s     = surface.alpha;                    // ( 1.29 + 0.5 * a );
    float s2    = s * s;
    const float VoL   = 2.0 * VoH * VoH - 1.0;      // double angle identity
    const float cos_r = VoL - NoV * NoL;
    const float r = cos_r >= 0.0 ? 1.0 / max(NoL, NoV + epsilon) : 1.0;
    float c_1    = 1.0 - 0.5 * a2 / (a2 + 0.33);
    float c_2    = 0.45 * a2 / (a2 + 0.09) * cos_r * r;
    return surface.albedo * inv_pi * (c_1 + c_2 ) * (1.0 + surface.perceptual_roughness * 0.5);
}


#endif // BRDF_GLSL


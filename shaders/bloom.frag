#version 450

#include "lib/utils.glsl"

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler2D in_color;
layout(set = 0, binding = 1) uniform BloomParams_Inline {
    float power;
    float threshold;
    float rev_threshold;
    float intensity;
};

layout(location = 0) in vec2 in_uv;

void main() {
    const vec3 hdr = texture(in_color, in_uv).rgb;
    const vec3 thresholded = bloom_threshold(hdr, power, threshold, rev_threshold);
    out_color = vec4(thresholded, 1.0);
}


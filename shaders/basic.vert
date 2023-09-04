#version 450

#include "lib/utils.glsl"

layout(set = 0, binding = 0) uniform CameraData {
    Camera camera;
};

layout(set = 1, binding = 0) readonly buffer Transforms {
    mat4 transforms[];
};

layout(set = 1, binding = 1) readonly buffer Materials {
    MaterialData materials[];
};

layout(set = 1, binding = 2) readonly buffer Indices {
    uvec4 mesh_indices[];
};

layout(location = 0) in vec3 in_position;
layout(location = 1) in uvec2 in_packed_normal_tangent_sign;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec3 out_tangent;
layout(location = 2) out vec3 out_bitangent;
layout(location = 3) out vec2 out_uv;
layout(location = 4) out vec2 out_motion;
layout(location = 5) out flat uint out_instance_index;

void main() {
    out_instance_index = gl_InstanceIndex;
    out_uv = in_uv;

    const vec3 in_normal = unpack_2_10_10_10(in_packed_normal_tangent_sign.x).xyz;
    const vec4 in_tangent_sign = unpack_2_10_10_10(in_packed_normal_tangent_sign.y);

    const mat4 transform = transforms[mesh_indices[gl_InstanceIndex].x];
    const mat4 last_transform = transforms[mesh_indices[gl_InstanceIndex].y];

    const mat3 model = mat3(transform);

    const vec4 current_position = (camera.unjittered_view_proj * transform * vec4(in_position, 1.0));
    const vec4 last_position = (camera.unjittered_view_proj * last_transform * vec4(in_position, 1.0));

    out_motion = ((last_position.xy / last_position.w) - (current_position.xy / current_position.w)) * 0.5;

    out_normal = normalize(model * in_normal);
    out_tangent = normalize(model * in_tangent_sign.xyz);
    out_bitangent = cross(out_tangent, out_normal) * in_tangent_sign.w;

    gl_Position = camera.view_proj * transform * vec4(in_position, 1.0);
}


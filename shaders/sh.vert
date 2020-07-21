#version 450

#include "lib/utils.glsl"

layout(set = 0, binding = 0) uniform CameraData {
	Camera camera;
};

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out flat uint out_index;

void main() {
	out_index = gl_InstanceIndex;

	gl_Position = camera.view_proj * in_model * vec4(in_position, 1.0);
}

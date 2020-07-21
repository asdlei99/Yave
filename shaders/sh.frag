#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in flat uint in_index;

layout(set = 0, binding = 1) readonly buffer SHBuffer {
	SH probes[];
};

void main() {
	const SH sh = probes[in_index];
	out_color = vec4(eval_sh(sh, in_normal), 1.0);
}

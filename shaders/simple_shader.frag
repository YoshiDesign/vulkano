#version 450

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat2 transform;
	vec2 offset;
	vec3 color;
	uniform float u_time;

} push;

//void main() {

	//outColor = vec4(, 1.0);
//}
layout(location = 0) in vec3 fragColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
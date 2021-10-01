#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input from the vertex buffer
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

// First descriptor set 0
layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionViewMatrix;
	mat4 normalMatrix;
	vec3 directionToLight;
} ubo;

layout(push_constant) uniform PushVert {
	mat4 modelMatrix;
	int imDex;
} push;

const float AMBIENT = 0.02;

void main() {
  gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);

  vec3 normalWorldSpace = normalize(mat3(ubo.normalMatrix) * normal);

  float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);

  fragColor = lightIntensity * color;
  fragTexCoord = uv;
}

#version 450

// Input from the vertex buffer
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 v_fragColor;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 v_fragTexCoord;

layout(location = 0) out vec3 f_fragColor;
layout(location = 1) out vec2 f_fragTexCoord;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionViewMatrix;
	vec4 ambientLightColor; // w is intensity
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

// Model matrix and a pretty normal matrix
layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projectionViewMatrix * positionWorld;

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	vec3 directionToLight = ubo.lightPosition - positionWorld.xyz;

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0);

	f_fragColor = (diffuseLight + ambientLight) * v_fragColor;
	f_fragTexCoord = v_fragTexCoord;
}

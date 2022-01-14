#version 450

layout(set = 0, binding = 1) uniform sampler2D texSampler[8];
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionViewMatrix;
	vec4 ambientLightColor; // w is intensity
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

layout(set = 1, binding = 0) uniform FragUbo {
    uint imDex;
} fubo;

void main() {

    vec4 result = vec4(fragColor, 1.0);

    if (fubo.imDex != 8) {  // 8 will omit texture and default to vertex colors
        result = texture(texSampler[fubo.imDex], fragTexCoord);
    }

    // Gamma correction
    float gamma = 1.1;
    result.rgb = pow(result.rgb, vec3(1.0 / gamma));

	vec3 directionToLight = ubo.lightPosition - fragPosWorld;
	float attenuation = 1.0 / dot(directionToLight, directionToLight); 

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

    outColor = vec4((diffuseLight + ambientLight) * result.rgb, 1.0);
    
}
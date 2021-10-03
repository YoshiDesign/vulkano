#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform sampler2D texSampler[4];
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform FragUbo {
    uint info;
} fubo;

void main() {

    // Gamma correction
    vec4 result = texture(texSampler[fubo.info], fragTexCoord);
    float gamma = 1.1;
    result.rgb = pow(result.rgb, vec3(1.0 / gamma));

    outColor = result;
    
}
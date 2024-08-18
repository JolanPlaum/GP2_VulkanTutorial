#version 450

layout(binding = 1) uniform sampler2D texSampler[2];
#define tex1 texSampler[0]
#define tex2 texSampler[1]

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = (texture(tex1, fragTexCoord) + texture(tex2, fragTexCoord)) / 2.0;
}
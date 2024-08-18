#version 450
//---------------------------------------------------
// Input Variables
//---------------------------------------------------
layout(binding = 0) uniform CameraData {
    mat4 invView;
    mat4 view;
    mat4 proj;
} cam;

layout(binding = 2) uniform Model {
    mat4 pos;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

//---------------------------------------------------
// Output Variables
//---------------------------------------------------
layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragTangent;
layout(location = 3) out vec2 fragTexCoord;

//---------------------------------------------------
// Main Vertex Shader
//---------------------------------------------------
void main() {
    gl_Position = cam.proj * cam.view * model.pos * vec4(inPosition, 1.0);

    fragPosition = (model.pos * vec4(inPosition, 1.0)).xyz;
    fragNormal = mat3(model.pos) * normalize(inNormal);
    fragTangent = mat3(model.pos) * normalize(inTangent);
    fragTexCoord = inTexCoord;
}
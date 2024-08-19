#version 450
//---------------------------------------------------
// Global Variables
//---------------------------------------------------
float gPI = 3.14159265358979323846;
float gLightIntensity = 7.0;
float gShininess = 25.0;
vec3 gLightDirection = vec3(0.577, 0.577, -0.577);

//---------------------------------------------------
// Input Variables
//---------------------------------------------------
layout(binding = 1) uniform sampler2D texSampler[4];
#define diffuse     texSampler[0]
#define normal      texSampler[1]
#define specular    texSampler[2]
#define glossiness  texSampler[3]

layout(binding = 0) uniform CameraData {
    mat4 invView;
    mat4 view;
    mat4 proj;
} cam;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec2 fragTexCoord;

//---------------------------------------------------
// Output Variables
//---------------------------------------------------
layout(location = 0) out vec4 outColor;

//---------------------------------------------------
// Helper functions
//---------------------------------------------------
vec3 Lambert(float kd, vec3 cd) {
	float temp = kd / gPI;
	return vec3(cd.x * temp, cd.y * temp, cd.z * temp);
}

vec3 Phong(vec3 ks, float exp, vec3 l, vec3 v, vec3 n) {
	vec3 result = reflect(l, n);
	float dotProduct = clamp(dot(result, v), 0.0, 1.0);
	return ks * pow(dotProduct, exp);
}

//---------------------------------------------------
// Main Fragment Shader
//---------------------------------------------------
void main() {
	// final color
	vec3 finalColor = vec3(0.01, 0.01, 0.01);

	// view direction
	vec3 viewDirection = normalize(fragPosition - vec3(cam.invView[3].xyz));

	// normal map
	vec3 binormal = cross(fragNormal, fragTangent);
	mat4 tangentSpaceAxis = mat4(vec4(fragTangent, 0.0), vec4(binormal, 0.0), vec4(fragNormal, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
	vec4 sampledColor = texture(normal, fragTexCoord);
	vec3 partialColor = (2.0 * sampledColor.rgb) - vec3(1.0, 1.0, 1.0);
	vec3 normalResult = (tangentSpaceAxis * vec4(partialColor, 0.0)).xyz;

	// observed area (lambert cosine law)
	float dotProduct = clamp(dot(normalResult, -gLightDirection), 0.0, 1.0);

	// sampled textures
	vec4 sampledDiffuse = texture(diffuse, fragTexCoord);
	vec4 sampledSpecular = texture(specular, fragTexCoord);
	vec4 sampledGlossiness = texture(glossiness, fragTexCoord);

	finalColor += Lambert(gLightIntensity, sampledDiffuse.rgb) * dotProduct;
	finalColor += Phong(sampledSpecular.rgb, gShininess * sampledGlossiness.r, -gLightDirection, viewDirection, normalResult) * dotProduct;
	
	outColor = vec4(finalColor, 1.0);
}
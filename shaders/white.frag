#version 450
#define M_PI 3.1415926535897932384626433832795

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D envSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec3 fragToEyeVec;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
	
layout( push_constant ) uniform constants
{
	float metallic;
	float roughness;
} PushConstants;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {

	vec3 lightPos = vec3(5, 5, 10);

	vec3 toLight = normalize(lightPos - fragPosition.xyz);
	float diffuseMultiplier = max(dot(fragNorm, toLight), 0);
	
	vec3 normal = normalize(fragNorm);

	vec3 mirrorDir = normalize(2 * dot(fragToEyeVec, normal) * normal - fragToEyeVec);
	
	float mirror_tex_y = -asin(mirrorDir.z) / M_PI + 0.5;
	vec2 mirrorNormalizeFlatDir = normalize(mirrorDir.xy);
	float mirror_tex_x = (mirrorNormalizeFlatDir.x > 0 ? acos(mirrorNormalizeFlatDir.y) : (2*M_PI - acos(mirrorNormalizeFlatDir.y))) / (2 * M_PI);
	
	vec2 mirrorTexCoord = vec2(mirror_tex_x, mirror_tex_y);
	mirrorTexCoord = mirrorTexCoord + 0.01 * rand(mirrorTexCoord);

	vec4 mirror = vec4(texture(envSampler,mirrorTexCoord).rgb, 1.0) * (0.1 + 0.9 * diffuseMultiplier);
	float mirrorMultiplier = 1 - dot(normal, normalize(fragToEyeVec));

	vec4 diffuse = vec4(texture(texSampler, fragTexCoord).rgb, 1.0) * (0.1 + 0.9 * diffuseMultiplier);
	
	outColor = diffuse * (1-mirrorMultiplier) + mirrorMultiplier*mirror;

	//outColor = vec4(mirror_tex_x, mirror_tex_y, 0,0);
	//outColor = vec4(normalize(fragToEyeVec),0);
	//outColor = vec4(0,0,mirrorDir.b,0);
	//outColor = vec4(0,0,fragNorm.b,0);
	//outColor = vec4(normal,0);
}
#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
	
void main() {

	vec3 lightPos = vec3(5, 5, 10);

	int noize = int(100 *(fragPosition.x + fragPosition.y + fragPosition.z));
	noize = (noize * 73) % 100;

	float noizeMultiplier = ((1.0 * noize) / 100);

	vec3 toLight = normalize(lightPos - fragPosition.xyz);
	float diffuseMultiplier = max(dot(fragNorm, toLight), 0);

	outColor = vec4(texture(texSampler,fragTexCoord).rgb, 1.0) * (0.1 + noizeMultiplier * 0.9 * diffuseMultiplier);
	//outColor = vec4(1.0, 1.0, 1.0, 1.0) * (0.1 + 0.9 * diffuseMultiplier);
}
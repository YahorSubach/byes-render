#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec3 fragToEyeVec;
layout(location = 3) out vec2 fragTexCoord;

layout( push_constant ) uniform constants
{
	mat4 project_matrix;
	mat4 view_model_matrix;
} PushConstants;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	mat4 inversedEye = inverse(ubo.view);
	vec4 eyePos = inversedEye * vec4(0, 0, 0, 1.0);

	fragPosition = (ubo.model * vec4(inPosition, 1.0)).xyz;
	fragToEyeVec = eyePos.xyz - fragPosition;

	fragNorm = (mat3(ubo.model) * vec3(inNormal));
	
	fragTexCoord = inTexCoord;
}
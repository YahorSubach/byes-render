#version 450

layout(set = 0, binding = 0) uniform CameraPositionAndViewProjMat_0 {
    vec4 position;
    mat4 projViewMatrix;
} camera;

layout(set = 1, binding = 0) uniform ModelMatrix_0 {
    mat4 modelMatrix;
} object;

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

	vec4 offset = vec4(0,0,0,0);

    gl_Position = camera.projViewMatrix * object.modelMatrix * vec4(inPosition, 1.0) + offset;

	fragPosition = (object.modelMatrix * vec4(inPosition, 1.0)).xyz +  + offset.xyz;
	fragToEyeVec = camera.position.xyz - fragPosition;

	fragNorm = (mat3(object.modelMatrix) * vec3(inNormal));
	
	fragTexCoord = inTexCoord;
}
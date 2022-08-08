#version 450

layout(set = 0, binding = 0) uniform CameraPositionAndViewProjMat {
    vec4 position;
    mat4 projViewMatrix;
} camera;

layout(set = 1, binding = 0) uniform ModelMatrix {
    mat4 modelMatrix;
} object;


layout(set = 5, binding = 0) uniform Skeleton {
    mat4 matrices[32];
} skeleton;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uvec4 inJoints;
layout(location = 4) in vec4 inWeights;

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

	mat4 joint_transform = skeleton.matrices[0] * inWeights.x + skeleton.matrices[1] * inWeights.y + skeleton.matrices[2] * inWeights.z + skeleton.matrices[3] * inWeights.a;


    gl_Position = camera.projViewMatrix * object.modelMatrix * joint_transform * vec4(inPosition, 1.0); 

	fragPosition = (object.modelMatrix * vec4(inPosition, 1.0)).xyz;
	fragToEyeVec = camera.position.xyz - fragPosition;

	fragNorm = (mat3(object.modelMatrix) * vec3(inNormal));
	
	fragTexCoord = inTexCoord;
}
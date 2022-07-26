#version 450

layout(set = 0, binding = 0) uniform LightPositionAndViewProjMat {
	vec4 position;
	mat4 view_mat;
	mat4 proj_mat;
	float near;
	float far;
} light;


layout(set = 1, binding = 0) uniform ModelMatrix {
    mat4 modelMatrix;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout( push_constant ) uniform constants
{
	mat4 project_matrix;
	mat4 view_model_matrix;
} PushConstants;

void main() {
    gl_Position = light.proj_mat * light.view_mat * object.modelMatrix * vec4(inPosition, 1.0);
}
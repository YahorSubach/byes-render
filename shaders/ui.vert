#version 450

layout(set = 0, binding = 0) uniform ModelMatrix_0 {
    mat4 matrix;
} transformUBO;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 fragPosition;
layout(location = 1) out vec2 fragTexCoord;


void main() {
    //gl_Position = vec4(positions[gl_VertexIndex], 0, 1);
    gl_Position = transformUBO.matrix * vec4(inPosition.xy, 0, 1);

    gl_Position.xy = gl_Position.xy * 2 - 1;
    gl_Position.z = 0;
    gl_Position.w = 1;

	fragPosition = gl_Position;
	fragTexCoord = inTexCoord;
}
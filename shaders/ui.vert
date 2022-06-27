#version 450


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 fragPosition;
layout(location = 1) out vec2 fragTexCoord;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    //gl_Position = vec4(positions[gl_VertexIndex], 0, 1);
    gl_Position = vec4(inPosition.xy, 0, 1);
	fragPosition = gl_Position;
	fragTexCoord = inTexCoord;
}
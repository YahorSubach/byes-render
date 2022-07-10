#version 450

vec2 positions[6] = vec2[](
    vec2(-0.5, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, 0.5),

    vec2(0.5, 0.5),
    vec2(0.5, -0.5),
    vec2(-0.5, -0.5)
);

layout(location = 0) out vec3 fragPosition;

void main() {

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragPosition = gl_Position.xyz;
}
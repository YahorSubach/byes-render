#version 450

layout(set = 0, binding = 0) uniform sampler2D gAlbedo;

layout(location = 0) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

void main() {

	outColor = texture(gAlbedo, (fragPosition.xy + 0.5));
}
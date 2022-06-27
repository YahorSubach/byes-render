#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;


layout(location = 0) in vec4 fragPosition;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
	
void main() {

	outColor = vec4(texture(texSampler, fragTexCoord).r,texture(texSampler, fragTexCoord).r,texture(texSampler, fragTexCoord).r,texture(texSampler, fragTexCoord).r);
}
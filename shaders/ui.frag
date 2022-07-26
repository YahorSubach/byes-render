#version 450

layout(set = 1, binding = 0) uniform sampler2D Texture_texSampler;


layout(location = 0) in vec4 fragPosition;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
	
void main() {

	outColor = vec4(texture(Texture_texSampler, fragTexCoord).r,texture(Texture_texSampler, fragTexCoord).r,texture(Texture_texSampler, fragTexCoord).r,texture(Texture_texSampler, fragTexCoord).r);
}
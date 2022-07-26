#version 450

layout(set = 0, binding = 0) uniform sampler2D GBuffers_Albedo;
layout(set = 0, binding = 1) uniform sampler2D GBuffers_Position;
layout(set = 0, binding = 2) uniform sampler2D GBuffers_Normal;



layout(location = 0) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

void main() {

	vec4 texColor = texture(GBuffers_Albedo, (fragPosition.xy + 1));

	if(fragPosition.x > 0)
		texColor = texture(GBuffers_Position, (fragPosition.xy ));

	if(fragPosition.y > 0)
		texColor = texture(GBuffers_Normal, (fragPosition.xy));

	outColor =  texColor;
}
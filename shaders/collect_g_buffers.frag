#version 450

#include "ggx.glsl"

layout(set = 0, binding = 0) uniform sampler2D GBuffers_Albedo;
layout(set = 0, binding = 1) uniform sampler2D GBuffers_Position;
layout(set = 0, binding = 2) uniform sampler2D GBuffers_Normal;

layout(set = 1, binding = 0) uniform CameraPositionAndViewProjMat_0 {
    vec4 position;
    mat4 projViewMatrix;
} camera;


layout(location = 0) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;




void main() {

	vec4 texColor = texture(GBuffers_Albedo, (fragPosition.xy + 1));

	if(fragPosition.x > 0)
		texColor = texture(GBuffers_Position, (fragPosition.xy ));

	if(fragPosition.y > 0)
		texColor = texture(GBuffers_Normal, (fragPosition.xy));

	if(fragPosition.x > 0 && fragPosition.y > 0)
	{
		
		vec3 light_pos = vec3(1,3,4);
		
		vec3 unit_normal = normalize(texture(GBuffers_Normal, (fragPosition.xy)).xyz); 
		
		vec3 position = texture(GBuffers_Position, (fragPosition.xy)).xyz;
		vec3 albedo = texture(GBuffers_Albedo, (fragPosition.xy)).xyz;
		
		vec3 unit_view_direction = normalize(camera.position.xyz - position);
		vec3 unit_light_direction = normalize(light_pos - position);

		float roughness = 0.5;
		vec3 R0 = vec3(0.1,0.1,0.1);

		

		vec3 Reflected = F(unit_light_direction, unit_normal, R0);
		vec3 Spek = CookTorrance_GGX(unit_view_direction, unit_light_direction, unit_normal, roughness, R0);
		vec3 Diffused = 1 - Reflected;

		texColor = vec4(albedo * Diffused * clamp(dot(unit_normal, unit_light_direction),0,1) / M_PI + Spek, 1); 

	}

	outColor =  texColor;
}
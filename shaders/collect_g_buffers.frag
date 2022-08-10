#version 450

#include "ggx.glsl"

layout(set = 0, binding = 0) uniform sampler2D GBuffers_Albedo;
layout(set = 0, binding = 1) uniform sampler2D GBuffers_Position;
layout(set = 0, binding = 2) uniform sampler2D GBuffers_Normal;

layout(set = 1, binding = 0) uniform CameraPositionAndViewProjMat_0 {
    vec4 position;
    mat4 projViewMatrix;
} camera;

layout(set = 2, binding = 0) uniform sampler2D Environement_envSampler;


layout(location = 0) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

void main() {

	vec4 texColor = texture(GBuffers_Albedo, (fragPosition.xy + 1));

	if(fragPosition.x > 0)
		texColor = texture(GBuffers_Position, (fragPosition.xy ));

	if(fragPosition.y > 0)
		texColor = texture(GBuffers_Normal, (fragPosition.xy));

	float delta;
	int delta_ind;

	if(fragPosition.x > 0 && fragPosition.y > 0)
	{
		
		vec3 light_pos_1 = vec3(1,3,1);
		vec3 light_pos_2 = vec3(-2,1,2);
		vec3 light_pos_3 = vec3(0,-2,1.5);
		
		vec3 unit_normal = normalize(texture(GBuffers_Normal, (fragPosition.xy)).xyz); 
		
		vec3 position = texture(GBuffers_Position, (fragPosition.xy)).xyz;
		vec3 albedo = texture(GBuffers_Albedo, (fragPosition.xy)).xyz;
		
		vec3 unit_view_direction = normalize(camera.position.xyz - position);


		float roughness = 0.2;
		vec3 R0 = vec3(0.01, 0.01, 0.01);

		


		vec3 mirror_dir = 2 * dot(unit_normal, unit_view_direction) * unit_normal - unit_view_direction;
		

		float mirror_tex_y = -asin(mirror_dir.z) / M_PI + 0.5;
		vec2 mirrorNormalizeFlatDir = normalize(mirror_dir.xy);
		float mirror_tex_x = (mirrorNormalizeFlatDir.x > 0 ? acos(mirrorNormalizeFlatDir.y) : (2*M_PI - acos(mirrorNormalizeFlatDir.y))) / (2 * M_PI);
	
		vec2 mirrorTexCoord = vec2(mirror_tex_x, mirror_tex_y);

		//mirrorTexCoord = mirrorTexCoord + 0.003*(rand2(mirrorTexCoord) - 0.5);

		vec4 mirror_color = 0.3 * vec4(texture(Environement_envSampler,mirrorTexCoord).rgb, 1.0);

		texColor = vec4(0);

		float metalness = 0.1;
		vec3 unit_env_light_direction = normalize(mirror_dir);
		float mirror_brigthness = pow(length(mirror_color.xyz), 5);
		vec3 env_color = metalness * mirror_color.xyz * albedo + (1 - metalness) * vec3(mirror_brigthness);


		{

		vec3 kr = F(unit_view_direction, unit_env_light_direction, R0);

		vec3 krfr = clamp(CookTorrance_GGX(unit_view_direction, unit_env_light_direction, unit_normal, roughness, R0), 0, 1);

		texColor += vec4(env_color * krfr, 1);

		}


		{
		vec3 unit_light_direction = normalize(light_pos_1 - position);

		float light_distance = length(light_pos_1 - position);
		float attenuation = 1 / (0.001 + light_distance * light_distance);

		vec3 kr = F(unit_view_direction, unit_env_light_direction, R0);
		vec3 kd = 1- kr;
		vec3 krfr = attenuation * albedo * CookTorrance_GGX(unit_view_direction, unit_light_direction, unit_normal, roughness, R0);

		vec3 fd = (1 - metalness) * attenuation * albedo * clamp(dot(unit_normal, unit_light_direction),0,1);

		texColor += vec4(kd*fd + krfr, 1);
		}
//
//				{
//		vec3 unit_light_direction = normalize(light_pos_2 - position);
//
//		float light_distance = length(light_pos_2 - position);
//		float attenuation = 1 / (0.001 + light_distance * light_distance);
//
//		vec3 kr = F(unit_light_direction, unit_normal, R0);
//		vec3 kd = 1- kr;
//		vec3 krfr = attenuation * albedo * CookTorrance_GGX(unit_view_direction, unit_light_direction, unit_normal, roughness, R0);
//
//		vec3 fd = (1 - metalness) * attenuation * albedo * clamp(dot(unit_normal, unit_light_direction),0,1);
//
//		texColor += vec4(kd*fd + krfr, 1);
//		}
//
//				{
//		vec3 unit_light_direction = normalize(light_pos_3 - position);
//
//		float light_distance = length(light_pos_3 - position);
//		float attenuation = 1 / (0.001 + light_distance * light_distance);
//
//		vec3 kr = F(unit_light_direction, unit_normal, R0);
//		vec3 kd = 1- kr;
//		vec3 krfr = attenuation * albedo * CookTorrance_GGX(unit_view_direction, unit_light_direction, unit_normal, roughness, R0);
//
//		vec3 fd = (1 - metalness) * attenuation * albedo * clamp(dot(unit_normal, unit_light_direction),0,1);
//
//		texColor += vec4(kd*fd + krfr, 1);
//		}

		

	}

	outColor =  texColor;
}
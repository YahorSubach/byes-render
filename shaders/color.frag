#version 450
#define M_PI 3.1415926535897932384626433832795

layout(set = 2, binding = 0) uniform MaterialParam {
	int emit;
} material;

layout(set = 2, binding = 1) uniform sampler2D texSampler;


layout(set = 3, binding = 0) uniform CameraUniformBufferObject {
	vec4 position;
	mat4 view_mat;
	mat4 proj_mat;
	float near;
	float far;
} shadow_light;

layout(set = 4, binding = 0) uniform sampler2D envSampler;
layout(set = 4, binding = 1) uniform sampler2D shadowSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec3 fragToEyeVec;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
	
layout( push_constant ) uniform constants
{
	float metallic;
	float roughness;
} PushConstants;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float rand2(vec2 co){
    return fract(sin(dot(co, vec2(11, 78))) * 137456);
}

vec3 GetShadowMapCoords(vec3 position)
{
	vec4 shadow_map_pos = shadow_light.proj_mat * shadow_light.view_mat * vec4(position, 1);
	return shadow_map_pos.xyz / shadow_map_pos.w;
}

float LinearizeShadow(float near, float far, float shadow)
{
	return (-2 * far * near) / (far + near - shadow * (far - near));
}

float GetShadowMapValue(vec2 coords)
{
	//return (texture(shadowSampler, coords + vec2(0,0.001)).r + texture(shadowSampler, coords + vec2(-0.001, -0.001)).r + texture(shadowSampler, coords + vec2(0.001, -0.001)).r) / 3;
	
	return texture(shadowSampler, coords + 0.01*(rand2(coords) - 0.5)).r;
	//return texture(shadowSampler, coords).r;
}

void main() {

	vec3 lightPos = vec3(5, 5, 10);

	vec3 normal = normalize(fragNorm);

	vec3 to_light_unit = normalize(lightPos - fragPosition.xyz); 
	vec3 light_refl_unit =  2 * dot(to_light_unit, normal) * normal - to_light_unit;
	vec3 frag_to_eye_unit = normalize(fragToEyeVec);


	vec3 mirror_dir = 2 * dot(normal, frag_to_eye_unit) * normal - frag_to_eye_unit;
	
	float mirror_tex_y = -asin(mirror_dir.z) / M_PI + 0.5;
	vec2 mirrorNormalizeFlatDir = normalize(mirror_dir.xy);
	float mirror_tex_x = (mirrorNormalizeFlatDir.x > 0 ? acos(mirrorNormalizeFlatDir.y) : (2*M_PI - acos(mirrorNormalizeFlatDir.y))) / (2 * M_PI);
	
	vec2 mirrorTexCoord = vec2(mirror_tex_x, mirror_tex_y);

//	vec4 mirror = (
//	vec4(texture(envSampler,mirrorTexCoord + vec2(0.02, 0.02)).rgb, 1.0) +
//	vec4(texture(envSampler,mirrorTexCoord + vec2(-0.02, 0.02)).rgb, 1.0) +
//	vec4(texture(envSampler,mirrorTexCoord + vec2(0.02, -0.02)).rgb, 1.0) +
//	vec4(texture(envSampler,mirrorTexCoord + vec2(-0.02, -0.02)).rgb, 1.0)) / 4
//	* (0.1 + 0.9 * diffuseMultiplier);

	mirrorTexCoord = mirrorTexCoord + 0.003*(rand2(mirrorTexCoord) - 0.5);

	vec4 mirror_color =vec4(texture(envSampler,mirrorTexCoord).rgb, 1.0);
	float mirrorMultiplier = (1 - dot(normal, frag_to_eye_unit)) * length(texture(texSampler, fragTexCoord).rgb) / sqrt(3);

	vec4 diffuse_color = texture(texSampler, fragTexCoord).rgba;
	
	float light_multiplier = max(dot(fragNorm, to_light_unit), 0);
	
	float spec_multiplier = max(dot(frag_to_eye_unit, light_refl_unit), 0);
	spec_multiplier = spec_multiplier * spec_multiplier * spec_multiplier * spec_multiplier;




	vec3 shadow_map_coord = GetShadowMapCoords(fragPosition);
	
	vec4 light_space_frag_pos = shadow_light.view_mat * vec4(fragPosition, 1);

	float shadow_map_value = GetShadowMapValue(vec2(shadow_map_coord.x/2+0.5,shadow_map_coord.y/2+0.5));
	shadow_map_value = LinearizeShadow(shadow_light.near, shadow_light.far, shadow_map_value);

	//float shadow_value = clamp((shadow_map_value - light_space_frag_pos.z) * 3, 0.0, 1.0);
	float shadow_value = shadow_map_value - 0.1 > light_space_frag_pos.z ? 1.0 : 0.0;

	float diffuse_multiplier = (1 - shadow_value) * (light_multiplier + spec_multiplier); 

	//float shadow_multiplier = clamp(shadow_map_value+0. - shadow_map_coord.z, 0, 1);


	//float color = ((shadow_map_value+0.01) < shadow_map_coord.z) ? 0.f : 1.f; 
	//vec4 diffuse = vec4(color,color,color, 1.0) * (0.1 + 0.9 * diffuseMultiplier);
	
	if(material.emit == 1)
		outColor = diffuse_color;
	else
		outColor = diffuse_color * (0.1 + 0.9*diffuse_multiplier) + mirror_color * mirrorMultiplier;

		outColor.a = 2;
}
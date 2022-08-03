
#define M_PI 3.1415926535897932384626433832795
#define M_EPS5 0.00001

float Hi(vec3 direction, vec3 normal)
{
	return dot(direction, normal) > 0.01 ? 1 : 0;
}

float G_Partial(vec3 unit_direction, vec3 unit_normal, float roughness)
{
	float cos_theta = dot(unit_direction, unit_normal);
	float tan2 = 1 / (M_EPS5 + cos_theta * cos_theta) - 1;
	float roughness2 = roughness * roughness;

	return  Hi(unit_direction, unit_normal) * (2 / (1 + sqrt(1 + roughness2*roughness2*tan2)));
}

float G(vec3 unit_view_direction, vec3 unit_light_direction,  vec3 unit_normal, float roughness)
{
	return G_Partial(unit_view_direction, unit_normal, roughness) * G_Partial(unit_light_direction, unit_normal, roughness);
}

float D(vec3 unit_view_direction, vec3 unit_light_direction,  vec3 unit_normal, float roughness)
{
	float roughness2 = roughness * roughness;
	float roughness4 = roughness2 * roughness2;

	vec3 H = normalize((unit_view_direction + unit_light_direction) / 2);

	float cos_theta_m = dot(unit_normal, H);
	float cos2 = cos_theta_m*cos_theta_m;
	float cos4 = M_EPS5 + cos2*cos2;
	float tan2 = 1 / (M_EPS5 + cos_theta_m * cos_theta_m) - 1;

	float res = roughness4 / (M_PI * cos4 * (roughness4 + tan2) * (roughness4 + tan2));

	return res;
}

// FresnelSchlick
vec3 F(vec3 unit_light_direction,  vec3 unit_normal, vec3 R0)
{
	float cos_theta = clamp(dot(unit_light_direction, unit_normal), 0, 1);
	return R0 + (1 - R0)*pow(1 - cos_theta, 5);
}

vec3 CookTorrance_GGX(vec3 unit_view_direction, vec3 unit_light_direction,  vec3 unit_normal, float roughness, vec3 Fresnel_R0)
{
	float D = D(unit_view_direction, unit_light_direction, unit_normal, roughness);
	vec3 F = F(unit_light_direction, unit_normal, Fresnel_R0);
	float G = G(unit_view_direction, unit_light_direction, unit_normal, roughness);

	return D * F * G / vec3(M_EPS5 + 4 * dot(unit_view_direction, unit_normal));
}
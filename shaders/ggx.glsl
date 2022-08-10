
#define M_PI 3.1415926535897932384626433832795
#define M_EPS5 0.00001
#define M_EPS3 0.001

float Hi(vec3 direction, vec3 normal)
{
	return dot(direction, normal) > 0 ? 1 : 0;
}

float Hi(float v)
{
	return v > 0 ? 1 : 0;
}

float G_Partial(vec3 unit_direction, vec3 unit_normal, vec3 unit_half, float roughness)
{
	float cos_theta = dot(unit_direction, unit_normal);
	float tan2 = 1 / (M_EPS5 + cos_theta * cos_theta) - 1;
	float roughness2 = roughness * roughness;

	return  Hi(dot(unit_direction, unit_half) / dot(unit_direction, unit_normal)) * (2 / (1 + sqrt(1 + roughness2*roughness2*tan2)));
}

float G(vec3 unit_view_direction, vec3 unit_light_direction,  vec3 unit_normal, float roughness)
{
	vec3 unit_half = normalize(unit_light_direction + unit_view_direction);
	return G_Partial(unit_view_direction, unit_normal, unit_half, roughness) * G_Partial(unit_light_direction, unit_normal, unit_half, roughness);
}

float D(vec3 unit_view_direction, vec3 unit_light_direction,  vec3 unit_normal, float roughness)
{
	float roughness2 = roughness * roughness;
	float roughness4 = roughness2 * roughness2;

	vec3 H = normalize(unit_view_direction + unit_light_direction);

	float cos_theta_m = dot(unit_normal, H);
	float cos2 = cos_theta_m*cos_theta_m;
	float cos4 = cos2*cos2;
	float tan2 = 1.0 / (M_EPS5 + cos2) - 1.0;

	float res = Hi(H, unit_normal) * roughness4 / (M_PI * cos4 * (roughness4 + tan2) * (roughness4 + tan2));

	return res;
}

float D(vec3 unit_halfway,  vec3 unit_normal, float roughness)
{
	float roughness2 = roughness * roughness;
	float roughness4 = roughness2 * roughness2;

	float cos_theta_m = dot(unit_normal, unit_halfway);
	float cos2 = cos_theta_m*cos_theta_m;
	float cos4 = cos2*cos2;
	float tan2 = 1.0 / (M_EPS5 + cos2) - 1.0;

	float res = Hi(unit_halfway, unit_normal) * roughness4 / (M_PI * cos4 * (roughness4 + tan2) * (roughness4 + tan2));

	return res;
}

// FresnelSchlick
vec3 F(vec3 unit_view_direction,vec3 unit_light_direction, vec3 R0)
{
	vec3 halfway = normalize(unit_view_direction + unit_light_direction);
	float cos_theta = clamp(dot(unit_light_direction, halfway), 0, 1);
	return R0 + (1.0 - R0)*pow(1.0 - cos_theta, 5.0);
}

vec3 CookTorrance_GGX(vec3 unit_view_direction, vec3 unit_light_direction,  vec3 unit_normal, float roughness, vec3 fresnel_R0)
{
	float D = D(unit_view_direction,  unit_light_direction, unit_normal, roughness);
	float G = G(unit_view_direction,  unit_light_direction, unit_normal, roughness);
	vec3  F = F(unit_light_direction, unit_light_direction, fresnel_R0);

	return D * G * F / (4 * dot(unit_view_direction, unit_normal));
}
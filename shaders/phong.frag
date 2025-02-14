#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 oColor;

layout(binding = 1) uniform sampler2D texSampler;

struct PointLight
{
	vec3 position;
	vec3 diffuseColor;
	float diffusePower;
	vec3 specularColor;
	float specularPower;
};

struct Lighting
{
	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

PointLight light0 = PointLight(vec3(-1.0, 0.0, 0.0), vec3(0.4, 1.0, 0.2), 1.0, vec3(1.0), 1.0);

void main()
{
	vec3 normal = normalize(fragNormal);

	Lighting fragLighting;

	fragLighting.ambient = vec3(0.1);

	vec3 lightDir = normalize(light0.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	fragLighting.diffuse = diff * light0.diffuseColor * light0.diffusePower;

	fragLighting.specular = vec3(0.0);

	oColor = texture(texSampler, fragUV);
	oColor *= vec4(fragLighting.ambient + fragLighting.diffuse + fragLighting.specular, 1.0);
}
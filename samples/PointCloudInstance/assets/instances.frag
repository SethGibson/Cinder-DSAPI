#version 150
uniform vec3 u_EyeDir;
uniform float u_LightPos;
uniform float u_AmbientStrength;
uniform float u_SpecularPower;
uniform float u_SpecularStrength;

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

void main()
{
	vec3 normal = normalize(Normal);
	vec3 eyeDir = normalize(FragPos - u_EyeDir);
	vec3 lightDir = normalize(vec3(0,-u_LightPos,0) - FragPos);
	vec3 reflectDir = reflect(lightDir, normal);

	float diffuse = max(dot(normal, lightDir),0);
	float specular = pow(max(dot(reflectDir, eyeDir), 0), u_SpecularPower)*u_SpecularStrength;
	FragColor=vec4(Color*diffuse+(Color*u_AmbientStrength)+(Color*specular*diffuse),1.0);
}
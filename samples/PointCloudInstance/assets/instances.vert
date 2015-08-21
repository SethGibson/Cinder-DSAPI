#version 150

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;

in vec4 ciPosition;
in vec4 ciNormal;

in vec3 iPosition;
in vec3 iColor;
in float iSize;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;

void main()
{
	vec4 mPosition = ciPosition;
	mPosition.x*=iSize;
	mPosition.y*=iSize;
	mPosition.z*=iSize;

	gl_Position = ciModelViewProjection * (mPosition+vec4(iPosition,1.0));
	Color = iColor;
	FragPos = (ciModelMatrix * (mPosition+vec4(iPosition,1.0))).xyz;
	Normal = (ciModelMatrix*ciNormal).xyz;
}
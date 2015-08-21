#version 150
uniform mat4 ciModelViewProjection;
uniform float u_PointSize;

in vec4 ciPosition;
in vec2 ciTexCoord0;

out vec2 UV;

void main()
{
	UV = ciTexCoord0;
	gl_Position = ciModelViewProjection * ciPosition;
	gl_PointSize = u_PointSize;	
}
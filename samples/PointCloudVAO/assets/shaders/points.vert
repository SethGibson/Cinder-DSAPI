#version 150 core
uniform mat4 ciModelViewProjection;
uniform float u_PointSize;

in vec4 vPosition;
in vec4 vColor;

out vec4 Color;

void main()
{
	Color = vColor;
	gl_Position = ciModelViewProjection*vPosition;
	gl_PointSize = u_PointSize;
}
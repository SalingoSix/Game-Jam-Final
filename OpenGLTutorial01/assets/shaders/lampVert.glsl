#version 430 core

layout (location = 0) in vec3 lightPosition;
layout (location = 1) in mat4 modelMatrix;
layout (location = 5) in vec4 lightColor;

uniform mat4 lightModel;
uniform mat4 view;
uniform mat4 projection;

out vec4 outColor;

void main()
{
	outColor = lightColor;
	mat4 MVP = projection * view * modelMatrix;
	gl_Position = MVP * vec4(lightPosition, 1.0);
}
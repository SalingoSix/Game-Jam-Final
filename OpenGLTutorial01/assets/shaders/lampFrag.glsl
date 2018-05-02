#version 430 core

out vec4 FragColor;

in vec4 outColor;

void main()
{
	FragColor = vec4(outColor.rgb, 1.0);
}
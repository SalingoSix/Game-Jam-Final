#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragNormal;
layout (location = 2) out vec4 FragVertex;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;

void main()
{    
	vec2 flippedTexCoords;
	flippedTexCoords.x = TexCoords.x;
	flippedTexCoords.y = 1.0 - TexCoords.y;
	
	vec4 textureColour = texture(texture_diffuse1, flippedTexCoords);
	
	if (textureColour.a < 0.1)
		discard;
	
    FragColor = textureColour;
	FragNormal = vec4(Normal.xyz, 1.0);
	FragVertex = vec4(FragPos.xyz, 1.0);
}
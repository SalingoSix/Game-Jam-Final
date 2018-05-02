#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

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
}
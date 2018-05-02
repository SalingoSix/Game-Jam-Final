#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (location = 3) in vec3 offset;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float rotationValue;

void main()
{
    TexCoords = aTexCoords; 
	
	vec3 axis = vec3(0.0, 1.0, 0.0);
	float s = sin(rotationValue);
    float c = cos(rotationValue);
    float oc = 1.0 - c;
	
	mat4 yRotationMatrix = mat4(c,   0.0, -s,   0.0,
								0.0, 1.0, 0.0, 0.0,
								s,  0.0, c,   0.0,
								0.0, 0.0, 0.0, 1.0);
	
	mat4 translationMatrix = mat4(1.0, 0.0, 0.0, 0.0,
									0.0, 1.0, 0.0, 0.0,
									0.0, 0.0, 1.0, 0.0,
									offset.x, offset.y, offset.z, 1.0);
									
	vec4 offsetVector = vec4(offset, 1.0);
	
	vec4 alteredPos = translationMatrix * vec4(aPos, 1.0);
	alteredPos = yRotationMatrix * alteredPos;
	
	mat4 adjustedModel = model;
	adjustedModel = adjustedModel * translationMatrix;
	adjustedModel = adjustedModel * yRotationMatrix;

    gl_Position = projection * view * adjustedModel * vec4(aPos, 1.0);
	//gl_Position = projection * view * model * vec4(aPos + offset, 1.0);
}
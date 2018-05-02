#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 7) in mat4 modelMatrix;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 reflectedVector;
out vec3 refractedVector;

uniform bool isInstanced;

void main()
{
	if(isInstanced)
	{
		vec3 worldPosition = vec3(modelMatrix * vec4(aPos, 1.0));
		gl_Position = projection * view * modelMatrix * vec4(aPos, 1.0);
		FragPos = worldPosition;
		//Inverse transpose removes the "translation" effects of transformation
		//leaving only rotation and scale
		vec3 outNormal = mat3(transpose(inverse(modelMatrix))) * aNormal;
		Normal = outNormal;
		vec4 reflectNormal = modelMatrix * vec4(aNormal, 1.0);
		TexCoords = aTexCoord;
		
		vec3 viewVector = normalize(worldPosition - cameraPos);
		
		reflectedVector = reflect(viewVector, normalize(outNormal));
		refractedVector = refract(viewVector, normalize(outNormal), 1.00/1.33);
	}
	else
	{

	vec3 worldPosition = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = worldPosition;
	//Inverse transpose removes the "translation" effects of transformation
	//leaving only rotation and scale
	vec3 outNormal = mat3(transpose(inverse(model))) * aNormal;
	Normal = outNormal;
	vec4 reflectNormal = model * vec4(aNormal, 1.0);
	TexCoords = aTexCoord;
	
	vec3 viewVector = normalize(worldPosition - cameraPos);
	
	reflectedVector = reflect(viewVector, normalize(outNormal));
	refractedVector = refract(viewVector, normalize(outNormal), 1.00/1.33);
	}
}
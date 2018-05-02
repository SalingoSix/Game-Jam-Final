#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 fTangent;		// For bump (or normal) mapping
in vec3 fBitangent;
in vec4 ClipSpacePos0;
in vec4 PrevClipSpacePos0;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragNormal;
layout (location = 2) out vec4 FragVertex;
layout (location = 3) out vec2 FragMotion;

uniform int activeBoy;

void main()
{	
	vec3 normalizedPos = (ClipSpacePos0 / ClipSpacePos0.w).xyz;
	vec3 prevNormalizedPos = (PrevClipSpacePos0 / PrevClipSpacePos0.w).xyz;
	FragMotion = (normalizedPos - prevNormalizedPos).xy;


	if (activeBoy == 0)
	{	//Inactive boy is yellow
		FragColor = vec4(1.0, 1.0, 0.0, 1.0);
		FragNormal = vec4(Normal.xyz, 1.0);		
		FragVertex = vec4(FragPos.xyz, 1.0);
	}
	else
	{	//Active boy is blue
		FragColor = vec4(70.0 / 255.0, 130.0 / 255.0, 180.0 / 255.0, 1.0);
		FragNormal = vec4(Normal.xyz, 1.0);		
		FragVertex = vec4(FragPos.xyz, 1.0);
	}
}
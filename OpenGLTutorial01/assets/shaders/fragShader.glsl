#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragNormal;
layout (location = 2) out vec4 FragVertex;

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;
	sampler2D texture_diffuse4;
	sampler2D texture_diffuse5;
	sampler2D texture_specular1;
	sampler2D texture_specular2;
	sampler2D texture_specular3;
	sampler2D texture_specular4;
	sampler2D texture_specular5;
	//vec3 specular;
	float shininess;
};

struct DirLight
{
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;
	
	float constant;
	float linear;
	float quadratic;
	float radius;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
	
	float constant;
	float linear;
	float quadratic;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 reflectedVector;
in vec3 refractedVector;

uniform samplerCube skybox;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform int reflectRefract;

#define NUM_POINT_LIGHTS 4

uniform int numPointLights;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[100];
uniform SpotLight spotLight;
uniform bool checkDistance;

float near = 0.1;
float far = 100.0;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float LinearizeDepth(float depth);

void main()
{
	//Displaying the depth buffer
	//float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
	vec4 textureColor = texture(material.texture_diffuse1, TexCoords);
	if(textureColor.a < 0.1)
		discard;
	
	if (reflectRefract == 0)
	{		
		FragColor = vec4(texture(material.texture_diffuse1, TexCoords).rgb, 1.0);
		FragNormal = vec4(Normal.xyz, 1.0);
		FragVertex = vec4(FragPos, 1.0);
	}	
	else if (reflectRefract == 1)
	{
		//Using a reflected skybox
		vec4 reflectedColour = vec4(texture(skybox, reflectedVector).rgb, 1.0);
		FragColor = reflectedColour;
		FragNormal = vec4(Normal.xyz, 1.0);
		FragVertex = vec4(FragPos, 1.0);
	}
	else if (reflectRefract == 2)
	{
		//Refracting the skybox
		vec4 refractedColour = vec4(texture(skybox, refractedVector).rgb, 1.0);
		FragColor = refractedColour;
		FragNormal = vec4(Normal.xyz, 1.0);
		FragVertex = vec4(FragPos, 1.0);
	}

}


float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}
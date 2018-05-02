#ifndef _LIGHT_MANAGER_
#define _LIGHT_MANAGER_

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include <vector>

class cShaderProgram;


enum eLightType
{
	DIRECTIONAL_LIGHT,
	POINT_LIGHT,
	SPOT_LIGHT,	
	UNKNOWN
};

//struct sLight
//{
//	glm::vec4 ambient;
//	glm::vec4 diffuse;
//	glm::vec4 specular;
//
//	glm::vec3 attenuation;
//
//	glm::vec3 position;
//	glm::vec3 direction;
//	glm::vec2 cutoff;
//
//	float radius;
//
//	eLightType lightType;
//
//	sLight()
//	{
//		this->ambient = glm::vec3(1.0f);
//		this->diffuse = glm::vec3(1.0f);
//		this->specular = glm::vec3(1.0f, 0.0f, 0.0f, 1.0f);
//		this->attenuation = glm::vec3(1.0f, 0.35f, 0.2f);
//		this->position = glm::vec3(0.0f);
//		this->direction = glm::vec3(0.0f);
//		this->cutoff = glm::vec2(0.0f);
//		this->lightType = eLightType::UNKNOWN;
//	}
//};

struct sDirectionalLight
{
	glm::vec3 direction;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	sDirectionalLight() : direction(glm::vec3(0.0f)), ambient(glm::vec3(1.0f)), diffuse(glm::vec3(1.0)), specular(glm::vec3(1.0f)) {}
};
struct sPointLight
{
	glm::vec3 position;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	glm::vec3 attenuation;

	float radius;

	sPointLight() : position(glm::vec3(0.0f)), ambient(glm::vec3(1.0f)), diffuse(glm::vec3(1.0)), specular(glm::vec3(1.0f)), attenuation(glm::vec3(1.0f)) {}
};
struct sSpotLight
{
	glm::vec3 position;
	glm::vec3 direction;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	glm::vec3 attenuation;

	float radius;
};


struct sLightVAO
{
	GLuint lightVAO;
	GLuint lightVBO;

	sLightVAO()
	{
		this->lightVAO = 0;
		this->lightVBO = 0;
	}
};


class cLightManager
{
public:
	cLightManager();

	void CreateLights(int numRows, int numCols);
	void CreateLights();
	void LoadLampsIntoShader(cShaderProgram Shader);
	void LoadLightsIntoShader(cShaderProgram Shader);
	void DrawLightsIntoScene(cShaderProgram Shader);

	int NumPointLights;
	int NumSpotLights;



	std::vector<sPointLight> PointLights;
	std::vector<sSpotLight> SpotLights;
	sDirectionalLight DirectionalLight;


	std::vector<sLightVAO> LightVAOs;

private:
	glm::mat4* lightModels;
	glm::vec4* lightColors;
};
#endif // !_LIGHT_MANAGER_

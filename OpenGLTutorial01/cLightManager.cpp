#include "cLightManager.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "cShaderProgram.h"

#include "nUtilities.h"

std::string createArrayStringPointLights(int index, std::string type);
std::string createArrayStringDirectionalLights(int index, std::string type);
std::string createArrayStringSpotLights(int index, std::string type);

cLightManager::cLightManager()
{
	this->NumPointLights = 0;
	this->NumSpotLights = 0;
}

//void cLightManager::CreateLights()
//{
//	std::ifstream lightFile("assets/lighting/Lights.txt");
//	std::string temp;
//	
//
//	lightFile >> this->NumLights;
//	for (int i = 0; i < this->NumLights; i++)
//	{
//		sLight tempLight;
//		lightFile >> temp;
//		lightFile >> temp;
//		if (temp == "DIRECTIONAL")
//			tempLight.lightType = eLightType::DIRECTIONAL_LIGHT;
//		else if (temp == "SPOT")
//			tempLight.lightType = eLightType::SPOT_LIGHT;
//		else if (temp == "POINT")
//			tempLight.lightType = eLightType::POINT_LIGHT;
//		else
//			tempLight.lightType = eLightType::UNKNOWN;
//
//		lightFile >> temp;
//		lightFile >> tempLight.ambient.x;
//		lightFile >> tempLight.ambient.y;
//		lightFile >> tempLight.ambient.z;
//		lightFile >> tempLight.ambient.w;
//
//		lightFile >> temp;
//		lightFile >> tempLight.diffuse.x;
//		lightFile >> tempLight.diffuse.y;
//		lightFile >> tempLight.diffuse.z;
//		lightFile >> tempLight.diffuse.w;
//
//		lightFile >> temp;
//		lightFile >> tempLight.specular.x;
//		lightFile >> tempLight.specular.y;
//		lightFile >> tempLight.specular.z;
//		lightFile >> tempLight.specular.w;
//
//		lightFile >> temp;
//		lightFile >> tempLight.attenuation.x;
//		lightFile >> tempLight.attenuation.y;
//		lightFile >> tempLight.attenuation.z;
//
//		lightFile >> temp;
//		lightFile >> tempLight.position.x;
//		lightFile >> tempLight.position.y;
//		lightFile >> tempLight.position.z;
//
//		lightFile >> temp;
//		lightFile >> tempLight.direction.x;
//		lightFile >> tempLight.direction.y;
//		lightFile >> tempLight.direction.z;
//
//		lightFile >> temp;
//		lightFile >> tempLight.cutoff.x;
//		lightFile >> tempLight.cutoff.y;
//
//		this->Lights.push_back(tempLight);
//	}
//
//}

void cLightManager::CreateLights(int numRows, int numCols)
{
	this->NumPointLights = numRows * numCols;

	{
		sDirectionalLight tempLight;

		tempLight.ambient = glm::vec3(1.0f);
		tempLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		tempLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
		tempLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);

		this->DirectionalLight = tempLight;
	}
	for (int i = 0; i != numRows; i++)
	{
		for(int j = 0; j != numCols; j++)
		{
			sPointLight tempLight;

			//tempLight.ambient = glm::vec4(1.0f);
			if (nUtility::getRandInRange(0.0f, 100.0f) < 10.0f)
				tempLight.diffuse = glm::vec4(nUtility::getRandInRange(0.0f, 20.0f), nUtility::getRandInRange(0.0f, 20.0f), nUtility::getRandInRange(0.0f, 20.0f), 1.0f);
			else
				tempLight.diffuse = glm::vec4(nUtility::getRandInRange(0.0f, 1.0f), nUtility::getRandInRange(0.0f, 1.0f), nUtility::getRandInRange(0.0f, 1.0f), 1.0f);

			tempLight.ambient = tempLight.diffuse;
			tempLight.specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			tempLight.attenuation = glm::vec3(0.1f, 0.01f, 0.1f);
			tempLight.position = glm::vec3(-25.0f + i * 10.0f, 5.f, -25.0f  + j * 10.0f);


			float constant = tempLight.attenuation.x;
			float linear = tempLight.attenuation.y;
			float quadratic = tempLight.attenuation.z;
			float lightMax = std::fmaxf(std::fmaxf(tempLight.diffuse.r, tempLight.diffuse.g), tempLight.diffuse.b);
			tempLight.radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * lightMax))) / (2 * quadratic);

			this->PointLights.push_back(tempLight);
		}
	}

}

void cLightManager::LoadLampsIntoShader(cShaderProgram Shader)
{
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	lightModels = new glm::mat4[this->NumPointLights];
	lightColors = new glm::vec4[this->NumPointLights];
	for (int i = 0; i != this->NumPointLights; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, this->PointLights[i].position);
		if(i == 0)
			model = glm::scale(model, glm::vec3(2.0f));
		else
			model = glm::scale(model, glm::vec3(0.5f));

		lightModels[i] = model;
		lightColors[i] = glm::vec4(this->PointLights[i].diffuse, 1.0f);
	}

	
	sLightVAO tempVAO;

	GLuint lightVAO, lightVBO;
	GLuint instanceVBO, colorVBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);
	glGenBuffers(1, &instanceVBO);
	glGenBuffers(1, &colorVBO);
	// Bind Buffer for vertices
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(lightVAO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	// Bind Buffer for model matrices
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, NumPointLights * sizeof(glm::mat4), &lightModels[0], GL_STATIC_DRAW);


	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	// Bind Buffer for colors
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, NumPointLights * sizeof(glm::vec4), &lightColors[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glVertexAttribDivisor(5, 1);


	glBindVertexArray(0);

	tempVAO.lightVAO = lightVAO;
	tempVAO.lightVBO = lightVBO;
	this->LightVAOs.push_back(tempVAO);
	
}

void cLightManager::DrawLightsIntoScene(cShaderProgram Shader)
{
	glBindVertexArray(this->LightVAOs[0].lightVAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, NumPointLights);
}

void cLightManager::LoadLightsIntoShader(cShaderProgram Shader)
{
	//"pointLights[2].position"
	Shader.useProgram();
	for (int i = 0; i < this->PointLights.size(); i++)
	{
		Shader.setVec3(createArrayStringPointLights(i, "position").c_str(), this->PointLights[i].position);
		Shader.setFloat(createArrayStringPointLights(i, "constant").c_str(), this->PointLights[i].attenuation.x);
		Shader.setFloat(createArrayStringPointLights(i, "linear").c_str(), this->PointLights[i].attenuation.y);
		Shader.setFloat(createArrayStringPointLights(i, "quadratic").c_str(), this->PointLights[i].attenuation.z);
		Shader.setFloat(createArrayStringPointLights(i, "radius").c_str(), this->PointLights[i].radius);
		Shader.setVec3(createArrayStringPointLights(i, "ambient").c_str(), this->PointLights[i].ambient);
		Shader.setVec3(createArrayStringPointLights(i, "diffuse").c_str(), this->PointLights[i].diffuse);
		Shader.setVec3(createArrayStringPointLights(i, "specular").c_str(), this->PointLights[i].specular);
		//Shader.setVec3(createArrayStringForGLSL(i, "Attenuation").c_str(), this->PointLights[i].attenuation);
	}
	{		
		Shader.setVec3(createArrayStringDirectionalLights(0, "direction").c_str(), this->DirectionalLight.direction);
		Shader.setVec3(createArrayStringDirectionalLights(0, "ambient").c_str(), this->DirectionalLight.ambient);
		Shader.setVec3(createArrayStringDirectionalLights(0, "diffuse").c_str(), this->DirectionalLight.diffuse);
		Shader.setVec3(createArrayStringDirectionalLights(0, "specular").c_str(), this->DirectionalLight.specular);
	}
	Shader.setInt("numPointLights", this->NumPointLights);
}

std::string createArrayStringPointLights(int index, std::string type)
{
	//std::cout << "pointLights[" + std::to_string(index) + std::string("].") + type << std::endl;
	return ("pointLights[" + std::to_string(index) + std::string("].") + type);
}
std::string createArrayStringDirectionalLights(int index, std::string type)
{
	//std::cout << "Lights[" + std::to_string(index) + std::string("].") + type << std::endl;
	return ("dirLight.") + type;
}
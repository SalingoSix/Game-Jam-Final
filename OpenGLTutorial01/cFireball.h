#ifndef _HG_cFireball_
#define _HG_cFireball_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "cParticleEmitter.h"

class cFireball
{
public:

	glm::vec3 Position;
	glm::vec3 Velocity;
	float Life;
	float Speed;
	cParticleEmitter* myEmitter;
	bool deleteMe;

	cFireball(glm::vec3 spawnPoint, glm::vec3 spawnVelocity, cParticleEmitter* newEmitter);
	~cFireball();
	void Update(float deltaTime);
};


#endif
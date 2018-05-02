#ifndef _HG_cParticleEmitter_
#define _HG_cParticleEmitter_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <stdlib.h>
#include <time.h>

struct sParticle
{
	sParticle() : Position(0.0f), Velocity(0.0f), Colour(1.0f), Life(0.0f)
	{}

	glm::vec3 Position, Velocity;
	glm::vec4 Colour;
	float Life;
};

class cParticleEmitter
{
public:
	//To add: speed (velocity is more "direction" than actual velocity) and duration, spawns per frame, and a kill timer
	cParticleEmitter(int numParticles, glm::vec3 position, int TexID, glm::vec3 minVelocity, glm::vec3 maxVelocity, float speed = 1, float lifeSpan = 1.0f, float spawnsPerFrame = 2.0f, float killTimer = 0.0f);

	void Update(float deltaTime);
	int findFirstUnused();
	void respawnParticle(int chosenParticle);
	void updateEmitterPos(glm::vec3 newPos);

	std::vector<sParticle*> particles;
	unsigned int textureID;
	float numNewParticles;
	int lastUsedParticle;
	bool staticOrObject;
	float accumulatedTime;

	glm::vec3 emitterPos;
	
	float particleSpeed;
	float particleLifeSpan;
	float killTimer;
	bool timed;
	bool firstFrame;
	bool deleteMe;

	glm::vec3 minOffset;
	glm::vec3 maxOffset;

};


#endif
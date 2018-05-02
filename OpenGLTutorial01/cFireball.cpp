#include "cFireball.h"

cFireball::cFireball(glm::vec3 spawnPoint, glm::vec3 spawnVelocity, cParticleEmitter* newEmitter)
{
	myEmitter = newEmitter;
	Position = spawnPoint;
	Velocity = spawnVelocity;
	Speed = 12.0f;
	Life = 5.0f;
	deleteMe = false;
}

cFireball::~cFireball()
{

}

void cFireball::Update(float deltaTime)
{
	if (Life > 0.0f)
	{
		Position += Velocity * deltaTime * Speed;
		myEmitter->updateEmitterPos(Position);
	}


	Life -= deltaTime;

	if (Life <= 0.0f)
	{
		myEmitter->timed = true;
		deleteMe = true;
	}
}
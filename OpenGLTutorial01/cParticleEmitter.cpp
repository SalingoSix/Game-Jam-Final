#include "cParticleEmitter.h"

cParticleEmitter::cParticleEmitter(int numParticles, glm::vec3 position, int TexID, glm::vec3 minVelocity, glm::vec3 maxVelocity, float speed, float lifeSpan, float spawnsPerFrame, float killTimer)
{
	for (int i = 0; i < numParticles; i++)
	{
		particles.push_back(new sParticle());
	}

	emitterPos = position;
	minOffset = minVelocity;
	maxOffset = maxVelocity;

	particleSpeed = speed;
	particleLifeSpan = lifeSpan;
	accumulatedTime = 0;
	firstFrame = true;
	deleteMe = false;
	if (killTimer == 0.0f)
	{
		this->timed = false;
		this->killTimer = -1.0f;
	}
	else
	{
		this->timed = true;
		this->killTimer = killTimer;
	}

	this->textureID = TexID;
	this->numNewParticles = 1.0f / spawnsPerFrame;
	this->lastUsedParticle = 0;

	if (minOffset.x < -1.0f)
		minOffset.x = -1.0f;
	else if (minOffset.x > 1.0f)
		minOffset.x = 1.0f;

	if (maxOffset.x < -1.0f)
		maxOffset.x = -1.0f;
	else if (maxOffset.x > 1.0f)
		maxOffset.x = 1.0f;

	if (minOffset.y < -1.0f)
		minOffset.y = -1.0f;
	else if (minOffset.y > 1.0f)
		minOffset.y = 1.0f;

	if (maxOffset.y < -1.0f)
		maxOffset.y = -1.0f;
	else if (maxOffset.y > 1.0f)
		maxOffset.y = 1.0f;

	if (minOffset.z < -1.0f)
		minOffset.z = -1.0f;
	else if (minOffset.z > 1.0f)
		minOffset.z = 1.0f;

	if (maxOffset.z < -1.0f)
		maxOffset.z = -1.0f;
	else if (maxOffset.z > 1.0f)
		maxOffset.z = 1.0f;

	if (maxOffset.x < minOffset.x)
	{	//Make sure the maxOffset is actually larger than the min offset
		float temp = minOffset.x;
		minOffset.x = maxOffset.x;
		maxOffset.x = temp;
	}

	if (maxOffset.y < minOffset.y)
	{	//Make sure the maxOffset is actually larger than the min offset
		float temp = minOffset.y;
		minOffset.y = maxOffset.y;
		maxOffset.y = temp;
	}

	if (maxOffset.z < minOffset.z)
	{	//Make sure the maxOffset is actually larger than the min offset
		float temp = minOffset.z;
		minOffset.z = maxOffset.z;
		maxOffset.z = temp;
	}

	srand(time(NULL));
}

void cParticleEmitter::updateEmitterPos(glm::vec3 newPos)
{
	this->emitterPos = newPos;
}

void cParticleEmitter::Update(float deltaTime)
{
	//Add new particles
	if (!timed || this->killTimer > 0.0f)
	{
		int someNewParticles = 0;
		while (accumulatedTime >= numNewParticles)
		{
			someNewParticles++;
			accumulatedTime -= numNewParticles;
		}
		for (int i = 0; i < someNewParticles; i++)
		{
			int firstUnusedParticle = findFirstUnused();
			respawnParticle(firstUnusedParticle);
		}
		accumulatedTime += deltaTime;
	}

	//Now update all particles positions
	for (int i = 0; i < particles.size(); i++)
	{
		sParticle* part = particles[i];
		part->Life -= deltaTime;
		if (part->Life > 0.0f)
		{
			part->Position += part->Velocity * deltaTime * this->particleSpeed;
		}
		else if (timed && killTimer < 0.0f)
		{
			part = NULL;
		}
	}

	//Update the kill timer
	if (timed)
	{
		if (firstFrame)
		{
			firstFrame = false;
		}
		else
		{
			killTimer -= deltaTime;
		}
	}

	//Lastly, if the kill timer has reached 0, wait for a full particle lifespan to pass, then call for the emitter to be deleted
	if (timed && killTimer <= 0.0f)
	{
		float negLifeSpan = -particleLifeSpan;
		if (killTimer <= negLifeSpan)
		{
			deleteMe = true;
		}
	}
}

int cParticleEmitter::findFirstUnused()
{
	//Start searching with the last particle you used
	for (int i = lastUsedParticle; i < particles.size(); i++)
	{
		if (particles[i]->Life <= 0.0f)
		{
			lastUsedParticle = i;
			return i;
		}
	}

	//If you can't find, start from the beginning of the array
	for (int i = 0; i < lastUsedParticle; i++)
	{
		if (particles[i]->Life <= 0.0f)
		{
			lastUsedParticle = i;
			return i;
		}
	}

	//If we can't find anything, override the first particle
	lastUsedParticle = 0;
	return 0;
}

void cParticleEmitter::respawnParticle(int chosenParticle)
{
	//Get a random direction vector
	int randIntX, randIntY, randIntZ;
	float randFloatX, randFloatY, randFloatZ;

	//If they're the same, don't do any randomization
	if (maxOffset.x == minOffset.x)
		randFloatX = maxOffset.x;
	else
	{	
		float xDiff = (maxOffset.x - minOffset.x) * 1000.0f;
		int intXDiff = (int)xDiff + 1;
		randIntX = (rand() % intXDiff);
		randFloatX = (float)randIntX / 1000.0f;
		randFloatX += minOffset.x;
	}

	//If they're the same, don't do any randomization
	if (maxOffset.y == minOffset.y)
		randFloatY = maxOffset.y;
	else
	{
		float yDiff = (maxOffset.y - minOffset.y) * 1000.0f;
		int intYDiff = (int)yDiff + 1;
		randIntY = (rand() % intYDiff);
		randFloatY = (float)randIntY / 1000.0f;
		randFloatY += minOffset.y;
	}

	//If they're the same, don't do any randomization
	if (maxOffset.z == minOffset.z)
		randFloatZ = maxOffset.z;
	else
	{
		float zDiff = (maxOffset.z - minOffset.z) * 1000.0f;
		int intZDiff = (int)zDiff + 1;
		randIntZ = (rand() % intZDiff);
		randFloatZ = (float)randIntZ / 1000.0f;
		randFloatZ += minOffset.z;
	}

	particles[chosenParticle] = new sParticle();
	particles[chosenParticle]->Life = this->particleLifeSpan;
	particles[chosenParticle]->Position = this->emitterPos;
	particles[chosenParticle]->Colour = glm::vec4(1.0f);
	glm::vec3 theVelocity = glm::normalize(glm::vec3(randFloatX, randFloatY, randFloatZ));
	particles[chosenParticle]->Velocity = theVelocity;

}
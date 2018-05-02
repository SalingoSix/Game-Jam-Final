#include "cSkinnedGameObject.h"
#include "cShaderProgram.h"
#include "cCamera3rdPerson.h"

#define _USE_MATH_DEFINES
#include <stack>
#include <math.h>
#include <glm\gtc\matrix_transform.hpp>

cSkinnedGameObject::cSkinnedGameObject(std::string modelName, std::string modelDir, glm::vec3 position, glm::vec3 scale, glm::vec3 orientationEuler, std::vector<std::string> charAnimations)
{
	this->Model = new cSkinnedMesh(modelDir.c_str());

	this->Position = position;
	this->Scale = scale;
	this->OrientationQuat = glm::quat(orientationEuler);
	this->OrientationEuler.x = glm::radians(orientationEuler.x);
	this->OrientationEuler.y = glm::radians(orientationEuler.y);
	this->OrientationEuler.z = glm::radians(orientationEuler.z);

	this->Forward = glm::vec3(0.0f, 0.0f, 0.0f);
	this->Forward.z = cos(this->OrientationEuler.y);
	this->Forward.x = sin(this->OrientationEuler.y);
	this->Forward = glm::normalize(this->Forward);

	this->defaultAnimState = new sAnimationState();
	this->defaultAnimState->defaultAnimation.name = modelDir;
	this->defaultAnimState->defaultAnimation.frameStepTime = 0.01f;
	this->defaultAnimState->defaultAnimation.totalTime = this->Model->GetDuration();

	this->curAnimState = new sAnimationState();
	this->curAnimState->defaultAnimation.name = modelDir;
	this->curAnimState->defaultAnimation.frameStepTime = 0.01f;
	//this->curAnimState->defaultAnimation.frameStepTime = 0.005f;
	this->curAnimState->defaultAnimation.totalTime = this->Model->GetDuration();

	this->vecCharacterAnimations = charAnimations;

	for (int i = 0; i != this->vecCharacterAnimations.size(); i++)
	{
		this->Model->LoadMeshAnimation(this->vecCharacterAnimations[i]);
	}

	this->Speed = 4.0f;
	this->animToPlay = this->defaultAnimState->defaultAnimation.name;
	oneFrameEvent = false;
	oneFrameStandWalkRun = 0;

	lastXAxis = 0.0f;
	lastYAxis = 0.0f;
}

void cSkinnedGameObject::setCamera(cCamera3rdPerson* theCamera)
{
	myCamera = theCamera;
}

void cSkinnedGameObject::Draw(cShaderProgram Shader, float deltaTime)
{
	float curFrameTime = 0.0f;

	if (this->defaultAnimState->defaultAnimation.name == this->animToPlay)
	{
		this->defaultAnimState->defaultAnimation.IncrementTime();
		curFrameTime = this->defaultAnimState->defaultAnimation.currentTime;
	}
	else
	{
		this->curAnimState->defaultAnimation.totalTime = this->Model->MapAnimationNameToScene[this->animToPlay]->mAnimations[0]->mDuration /
			this->Model->MapAnimationNameToScene[this->animToPlay]->mAnimations[0]->mTicksPerSecond;
		this->curAnimState->defaultAnimation.name = this->animToPlay;
		if ("assets/modelsFBX/RunningRobaxacet.fbx" == this->curAnimState->defaultAnimation.name)
		{
			this->curAnimState->defaultAnimation.frameStepTime = deltaTime / 2.f;
		}
		else
		{
			this->curAnimState->defaultAnimation.frameStepTime = deltaTime / 2.0f;
		}
		this->curAnimState->defaultAnimation.IncrementTime();
		curFrameTime = this->curAnimState->defaultAnimation.currentTime;
	}


	std::vector<glm::mat4> vecOffsets;

	this->Model->BoneTransform(curFrameTime, animToPlay, vecFinalTransformation, this->vecBoneTransformation, vecOffsets);


	GLuint numBonesUsed = static_cast<GLuint>(vecFinalTransformation.size());
	Shader.useProgram();
	Shader.setInt("numBonesUsed", numBonesUsed);
	glm::mat4* boneMatrixArray = &(vecFinalTransformation[0]);
	if (vecPrevTransformation.size() == 0)
	{
		vecPrevTransformation.resize(numBonesUsed);
	}
	glm::mat4* prevMatrixArray = &(vecPrevTransformation[0]);
	Shader.setMat4("bones", numBonesUsed, boneMatrixArray);
	Shader.setMat4("prevBones", numBonesUsed, prevMatrixArray);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, this->Position);
	model = glm::rotate(model, this->OrientationEuler.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, this->OrientationEuler.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, this->OrientationEuler.z, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, this->Scale);
	Shader.setMat4("model", model);

	this->Model->Draw(Shader);

	if (oneFrameEvent)
	{
		if (oneFrameStandWalkRun == 0)
		{
			//Animation happens while standing
		}
		else if (oneFrameStandWalkRun == 1)
		{	//Animation was called while walking
			glm::vec3 modifiedForward = this->Forward;
			modifiedForward.z = -modifiedForward.z;
			this->controllerMovement(this->lastXAxis, this->lastYAxis, deltaTime, false, false);
		}
		else if (oneFrameStandWalkRun == 2)
		{
			glm::vec3 modifiedForward = this->Forward;
			modifiedForward.z = -modifiedForward.z;
			this->controllerMovement(this->lastXAxis, this->lastYAxis, deltaTime, true, false);
		}
		if (curFrameTime == 0)
		{
			this->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
			oneFrameEvent = false;
		}
	}

	vecPrevTransformation = vecFinalTransformation;
}

void cSkinnedGameObject::Movement(Direction direction, float deltaTime, bool shiftKey, bool ctrlKey)
{
	float mySpeed = this->Speed;
	if (shiftKey)
		mySpeed += 8.0f;

	if (direction == FORWARD)
	{
		if (ctrlKey)
			return;
		this->Position += this->Forward * deltaTime * mySpeed;
	}

	else if (direction == BACKWARD)
	{
		if (ctrlKey)
			return;
		this->Position -= this->Forward * deltaTime * mySpeed;
	}

	else if (direction == LEFT)
	{
		if (ctrlKey)
		{
			float leftDirection = this->OrientationEuler.y + (M_PI / 2.0f);
			float strafeLeftZ = cos(leftDirection);
			float strafeLeftX = sin(leftDirection);
			glm::vec3 strafeLeft = glm::normalize(glm::vec3(strafeLeftX, 0.0f, strafeLeftZ));
			this->Position += strafeLeft * deltaTime * mySpeed;
		}
		else
		{
			this->OrientationEuler.y += this->Speed * deltaTime;
			this->Forward.z = cos(this->OrientationEuler.y);
			this->Forward.x = sin(this->OrientationEuler.y);
			this->Forward = glm::normalize(this->Forward);
		}
	}

	else if (direction == RIGHT)
	{
		if (ctrlKey)
		{
			float rightDirection = this->OrientationEuler.y - (M_PI / 2.0f);
			float strafeRightZ = cos(rightDirection);
			float strafeRightX = sin(rightDirection);
			glm::vec3 strafeRight = glm::normalize(glm::vec3(strafeRightX, 0.0f, strafeRightZ));
			this->Position += strafeRight * deltaTime * mySpeed;
		}
		else
		{
			this->OrientationEuler.y -= this->Speed * deltaTime;
			this->Forward.z = cos(this->OrientationEuler.y);
			this->Forward.x = sin(this->OrientationEuler.y);
			this->Forward = glm::normalize(this->Forward);
		}
	}

	float boundaries = 9.5f;

	if (Position.x >= boundaries)
	{
		Position.x = boundaries;
	}
	else if (Position.x <= -boundaries)
	{
		Position.x = -boundaries;
	}
	if (Position.z >= boundaries)
	{
		Position.z = boundaries;
	}
	else if (Position.z <= -boundaries)
	{
		Position.z = -boundaries;
	}
}

void cSkinnedGameObject::Movement(glm::vec3 movementDirection, float deltaTime, bool shiftKey, bool ctrlKey)
{
	//Determine if we're walking or running
	float mySpeed = this->Speed;
	if (shiftKey)
		mySpeed += 8.0f;

	//First, get the angle determined by the directional buttons pressed
	float theAngle = acos((glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), movementDirection)));

	glm::mat4 rotationMatrix(1.0f);

	//Adjust the x-axis if it's over 180 degrees
	if (movementDirection.x < 0)
	{
		theAngle = (2 * M_PI) - theAngle;
	}

	//Make a rotation matrix around this angle
	rotationMatrix = glm::rotate(rotationMatrix, theAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	//Rotate the front direction on the camera to find the vector the player should be travelling along
	glm::vec4 rotatedForward = rotationMatrix * glm::vec4(myCamera->front, 1.0f);

	//Create a forward vector using the rotated angle
	glm::vec3 newForward(-rotatedForward.x, rotatedForward.y, -rotatedForward.z);

	//Set this as the new forward
	this->Forward = newForward;
	this->Forward.y = 0.0f;

	this->Position += this->Forward * deltaTime * mySpeed;

	//Remember to add the camera's rotation to the object rotation
	this->OrientationEuler.y = theAngle + myCamera->cameraRotAngle;
}

void cSkinnedGameObject::controllerMovement(float xAxis, float yAxis, float deltaTime, bool running, bool LTDown)
{
	//Determine if we're walking or running
	float mySpeed = this->Speed;
	if (running)
		mySpeed += 8.0f;

	glm::vec3 normalizedMove = glm::normalize(glm::vec3(xAxis, 0.0f, yAxis));

	float theAngle = acos((glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), normalizedMove)));

	glm::mat4 rotationMatrix(1.0f);

	//Adjust the x-axis if it's over 180 degrees
	if (normalizedMove.x < 0)
	{
		theAngle = (2 * M_PI) - theAngle;
	}

	//Make a rotation matrix around this angle
	rotationMatrix = glm::rotate(rotationMatrix, theAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	//Rotate the front direction on the camera to find the vector the player should be travelling along
	glm::vec4 rotatedForward;
	if (LTDown)
	{	//We don't want to adhere to the camera's position during a strafe
		glm::vec3 forwardCamera;
		forwardCamera.x = sin(OrientationEuler.y);
		forwardCamera.y = 0.0f;
		forwardCamera.z = cos(OrientationEuler.y);
		forwardCamera = glm::normalize(forwardCamera);

		rotatedForward = rotationMatrix * glm::vec4(forwardCamera, 1.0f);
	}
	else
	{
		rotatedForward = rotationMatrix * glm::vec4(myCamera->front, 1.0f);
	}

	//Create a forward vector using the rotated angle
	glm::vec3 newForward(-rotatedForward.x, rotatedForward.y, rotatedForward.z);

	//Set this as the new forward

	this->Forward = newForward;
	this->Forward.y = 0.0f;

	//Yeah! This totally fixes it. Nothing will ever go wrong again!
	glm::vec3 alteredForward = this->Forward;
	alteredForward.z = -alteredForward.z;

	this->Position += alteredForward * deltaTime * mySpeed;

	//Remember to add the camera's rotation to the object rotation
	if (!LTDown)
	{
		this->OrientationEuler.y = theAngle + myCamera->cameraRotAngle;
	}
}
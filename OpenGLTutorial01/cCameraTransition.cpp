#include "cCameraTransition.h"

#define _USE_MATH_DEFINES
#include <math.h>

cCameraTransition::cCameraTransition(cCamera3rdPerson* third, cCameraStrafe* strafe, glm::vec3 Up)
{
	this->thirdPersonCam = third;
	this->strafeCam = strafe;

	worldUp = Up;
	cameraRotAngle = 0.0f;

	zoom = 45.0f;
	rotSpeed = 1.0f;
	moveSpeed = 25.0f;

	updateCameraVectors();
}

glm::mat4 cCameraTransition::getViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

void cCameraTransition::updateCamera(float deltaTime)
{
	if (transitionType == 0)
	{
		return;
	}

	else if (transitionType == 1)
	{	//Third person to strafe
		if (!angleReady)
		{
			//if (this->cameraRotAngle < strafeCam->cameraRotAngle)
			//{
			//	this->cameraRotAngle += rotSpeed * deltaTime;
			//	if (this->cameraRotAngle >= strafeCam->cameraRotAngle)
			//	{
			//		this->cameraRotAngle = strafeCam->cameraRotAngle;
			//		angleReady = true;
			//	}
			//}
			//else if (this->cameraRotAngle >= strafeCam->cameraRotAngle)
			//{
			//	this->cameraRotAngle -= rotSpeed * deltaTime;
			//	if (this->cameraRotAngle <= strafeCam->cameraRotAngle)
			//	{
			//		this->cameraRotAngle = strafeCam->cameraRotAngle;
			//		angleReady = true;
			//	}
			//}

			glm::vec3 pathToCam = strafeCam->front - this->front;
			float lengthToTravel = glm::length(pathToCam);
			glm::vec3 normalizedPath = glm::normalize(pathToCam);

			this->front += normalizedPath * rotSpeed * deltaTime;
			float newLength = glm::length(strafeCam->front - this->front);
			if (newLength >= lengthToTravel)
			{
				this->front = strafeCam->front;
				angleReady = true;
			}
		}

		if (!positionReady)
		{
			glm::vec3 pathToCam = strafeCam->position - this->position;
			float lengthToTravel = glm::length(pathToCam);
			glm::vec3 normalizedPath = glm::normalize(pathToCam);

			this->position += normalizedPath * moveSpeed * deltaTime;
			float newLength = glm::length(strafeCam->position - this->position);
			if (newLength >= lengthToTravel)
			{
				this->position = strafeCam->position;
				positionReady = true;
			}
		}

		if (positionReady && angleReady)
		{
			transitionType = 0;
			positionReady = false;
			angleReady = false;
		}
	}

	else if (transitionType == -1)
	{	//Strafe to third person
		if (!angleReady)
		{
			glm::vec3 pathToCam = thirdPersonCam->front - this->front;
			float lengthToTravel = glm::length(pathToCam);
			glm::vec3 normalizedPath = glm::normalize(pathToCam);

			this->front += normalizedPath * rotSpeed * deltaTime;
			float newLength = glm::length(thirdPersonCam->front - this->front);
			if (newLength >= lengthToTravel)
			{
				this->front = thirdPersonCam->front;
				angleReady = true;
			}
		}

		if (!positionReady)
		{
			glm::vec3 pathToCam = thirdPersonCam->position - this->position;
			float lengthToTravel = glm::length(pathToCam);
			glm::vec3 normalizedPath = glm::normalize(pathToCam);

			this->position += normalizedPath * moveSpeed * deltaTime;
			float newLength = glm::length(thirdPersonCam->position - this->position);
			if (newLength >= lengthToTravel)
			{
				this->position = thirdPersonCam->position;
				positionReady = true;
			}
		}

		if (positionReady && angleReady)
		{
			transitionType = 0;
			positionReady = false;
			angleReady = false;
		}
	}

	updateCameraVectors();
}

void cCameraTransition::strafeToThird()
{
	if (transitionType == 0)
	{
		//Set camera pos to strafe position
		this->position = strafeCam->position;
		this->front = strafeCam->front;
	}

	glm::vec3 pathToPos = thirdPersonCam->position - this->position;
	moveSpeed = glm::length(pathToPos) * 12.0f;

	glm::vec3 pathToCam = thirdPersonCam->front - this->front;
	rotSpeed = glm::length(pathToCam) * 12.0f;

	transitionType = -1;
}

void cCameraTransition::thirdToStrafe()
{
	//Try this:
	if (transitionType == 0)
	{
		//Set camera pos to thirdPerson position
		this->position = thirdPersonCam->position;
		this->front = thirdPersonCam->front;
	}

	glm::vec3 pathToPos = strafeCam->position - this->position;
	moveSpeed = glm::length(pathToPos) * 4.0f;
	
	glm::vec3 pathToCam = strafeCam->front - this->front;
	rotSpeed = glm::length(pathToCam) * 4.0f;
	if (rotSpeed < 1.0f)
	{
		rotSpeed = 4.0f;
	}

	transitionType = 1;
}

bool cCameraTransition::isFinished()
{
	if (transitionType == 0)
		return true;
	else
		return false;
}

void cCameraTransition::updateCameraVectors()
{
	//glm::vec3 forwardCamera;
	//forwardCamera.x = sin(cameraRotAngle);
	//forwardCamera.y = 0.0f;
	//forwardCamera.z = cos(cameraRotAngle);
	//forwardCamera = glm::normalize(forwardCamera);

	//We're going to be dealing with these values in the "transition" function. Hopefully they don't need to be altered again...
	//this->position = myPlayer->Position + ((zoomFactor + 1.0f) * forwardCamera);
	//this->position.y = zoomFactor + 1.0f;

	//this->front = -(forwardCamera);
	//float yDifference = position.y - myPlayer->Position.y;
	//this->front.y = -(yDifference) * 0.04f;

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}
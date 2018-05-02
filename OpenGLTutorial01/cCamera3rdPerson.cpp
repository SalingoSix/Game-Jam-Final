#include "cCamera3rdPerson.h"

#define _USE_MATH_DEFINES
#include <math.h>

cCamera3rdPerson::cCamera3rdPerson(cSkinnedGameObject* player, glm::vec3 Up, float initialZoom)
{
	myPlayer = player;
	zoomFactor = initialZoom;
	worldUp = Up;
	cameraRotAngle = 0.0f;
	zoom = 45.0f;

	rotSpeed = 1.0f;
	zoomSpeed = 5.0f;

	updateCameraVectors();
}

glm::mat4 cCamera3rdPerson::getViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

void cCamera3rdPerson::processKeyboard(Camera_Movement_3rd direction, float deltaTime)
{
	if (direction == CCW)
	{
		cameraRotAngle -= deltaTime * rotSpeed;
	}
	else if (direction == CW)
	{
		cameraRotAngle += deltaTime * rotSpeed;	
	}
	else if (direction == INWARD)
	{
		zoomFactor -= deltaTime * zoomSpeed;
		if (zoomFactor < 4.0f)
			zoomFactor = 4.0f;
	}
	else if (direction == OUTWARD)
	{
		zoomFactor += deltaTime * zoomSpeed;
		if (zoomFactor > 20.0f)
			zoomFactor = 20.0f;
	}

	updateCameraVectors();
}

void cCamera3rdPerson::processController(float xAxis, float yAxis, float deltaTime)
{
	//Give a small "dead zone" range, since moving one axis typically moves the other axis very slightly
	if (xAxis > 0.1f || xAxis < -0.1f)
	{
		cameraRotAngle -= deltaTime * rotSpeed * xAxis;
	}

	if (yAxis > 0.1f || yAxis < -0.1f)
	{
		zoomFactor -= deltaTime * zoomSpeed * yAxis;
		if (zoomFactor < 4.0f)
			zoomFactor = 4.0f;
		else if (zoomFactor > 20.0f)
			zoomFactor = 20.0f;
	}
}

void cCamera3rdPerson::playerMovement()
{
	updateCameraVectors();
}

void cCamera3rdPerson::updateCameraVectors()
{
	if (this->cameraRotAngle < 0.0f)
	{
		while (this->cameraRotAngle < 0.0f)
		{
			this->cameraRotAngle += 2 * M_PI;
		}
	}

	else if (this->cameraRotAngle >(2 * M_PI))
	{
		while (this->cameraRotAngle >(2 * M_PI))
		{
			this->cameraRotAngle -= 2 * M_PI;
		}
	}

	glm::vec3 forwardCamera;
	forwardCamera.x = sin(cameraRotAngle);
	forwardCamera.y = 0.0f;
	forwardCamera.z = cos(cameraRotAngle);
	forwardCamera = glm::normalize(forwardCamera);

	this->position = myPlayer->Position + ((zoomFactor + 1.0f) * forwardCamera);
	this->position.y = zoomFactor + 1.0f;

	this->front = -(forwardCamera);
	float yDifference = position.y - myPlayer->Position.y;
	this->front.y = -(yDifference) * 0.04f;

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

void cCamera3rdPerson::returnFromStrafe()
{
	zoomFactor = 7.0f;
	cameraRotAngle = myPlayer->OrientationEuler.y - M_PI;
	updateCameraVectors();
}
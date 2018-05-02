#include "cCameraStrafe.h"

#define _USE_MATH_DEFINES
#include <math.h>

cCameraStrafe::cCameraStrafe(cSkinnedGameObject* player, glm::vec3 Up, float camDistance)
{
	myPlayer = player;
	zoomFactor = camDistance;
	worldUp = Up;
	cameraRotAngle = myPlayer->OrientationEuler.y;
	zoom = 45.0f;

	rotSpeed = 1.0f;
	zoomSpeed = 5.0f;

	updateCameraVectors();
}

glm::mat4 cCameraStrafe::getViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

void cCameraStrafe::processController(float xAxis, float yAxis, float deltaTime)
{
	//if (xAxis > 0.1f || xAxis < -0.1f)
	//{
	//	cameraRotAngle -= deltaTime * rotSpeed * xAxis;
	//}

	//Also do something about the x axis... A little harder though
	if (xAxis > 0.1f || xAxis < -0.1f)
	{
		myPlayer->OrientationEuler.y -= deltaTime * rotSpeed * xAxis;
	}
	if (yAxis > 0.1f || yAxis < -0.1f)
	{
		yLook += deltaTime * (rotSpeed * 1.5f) * yAxis;

		if (yLook > 1.0f)
			yLook = 1.0f;
		else if (yLook < -1.0f)
			yLook = -1.0f;
	}
	//else
	//{
	//	yLook = 0.0f;
	//}
	
}

void cCameraStrafe::playerMovement()
{
	updateCameraVectors();
}

void cCameraStrafe::updateCameraVectors()
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
	forwardCamera.x = sin(myPlayer->OrientationEuler.y);
	forwardCamera.y = 0.0f;
	forwardCamera.z = cos(myPlayer->OrientationEuler.y);
	forwardCamera = glm::normalize(forwardCamera);

	this->position = myPlayer->Position - ((zoomFactor + 1.0f) * forwardCamera);
	this->position.y = 6.0f;

	this->front = forwardCamera;
	float yDifference = position.y - myPlayer->Position.y;
	this->front.y = -(yDifference) * 0.04f;
	this->front.y += yLook * 0.40f;

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}
#ifndef _HG_cCameraStrafe_
#define _HG_cCameraStrafe_

#include "cSkinnedGameObject.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//enum Camera_Movement_Strafe
//{
//	STRAFE_LOOK_UP,
//	STRAFE_LOOK_DOWN,
//	STRAFE_
//};

class cCameraStrafe
{
public:
	cCameraStrafe(cSkinnedGameObject* player, glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f), float camDistance = 4.0f);
	glm::mat4 getViewMatrix();
	void processController(float xAxis, float yAxis, float deltaTime);
	void playerMovement();

	//Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float zoom;
	float cameraRotAngle;
	float yLook;

	float rotSpeed;
	float zoomSpeed;

private:

	void updateCameraVectors();

	cSkinnedGameObject* myPlayer;
	float zoomFactor;
};

#endif
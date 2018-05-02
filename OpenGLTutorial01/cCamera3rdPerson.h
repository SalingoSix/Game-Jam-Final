#ifndef _HG_cCamera3rdPerson_
#define _HG_cCamera3rdPerson_

#include "cSkinnedGameObject.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement_3rd
{
	INWARD,
	OUTWARD,
	CCW,
	CW
};

class cCamera3rdPerson
{
public:
	cCamera3rdPerson(cSkinnedGameObject* player, glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f), float initialZoom = 2.0f);
	glm::mat4 getViewMatrix();
	void processKeyboard(Camera_Movement_3rd direction, float deltaTime);
	void processController(float xAxis, float yAxis, float deltaTime);
	void playerMovement();
	void returnFromStrafe();

	//Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float zoom;
	float zoomFactor;
	float cameraRotAngle;

	float rotSpeed;
	float zoomSpeed;

private:

	void updateCameraVectors();

	cSkinnedGameObject* myPlayer;
};


#endif
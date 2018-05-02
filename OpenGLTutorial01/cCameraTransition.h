#ifndef _HG_cCameraTransition_
#define _HG_cCameraTransition_

#include "cCamera3rdPerson.h"
#include "cCameraStrafe.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class cCameraTransition
{
public:
	cCameraTransition(cCamera3rdPerson* third, cCameraStrafe* strafe, glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 getViewMatrix();
	void updateCamera(float deltaTime);
	void strafeToThird();
	void thirdToStrafe();
	bool isFinished();

	//Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float cameraRotAngle;

	float zoom;
	float rotSpeed;
	float moveSpeed;

private:

	//Have we gotten to the right angle?
	bool angleReady;
	//Have we moved to the right position?
	bool positionReady;
	//Which camera are we transitioning to?
	int transitionType;

	cCamera3rdPerson* thirdPersonCam;
	cCameraStrafe* strafeCam;

	void updateCameraVectors();
};


#endif
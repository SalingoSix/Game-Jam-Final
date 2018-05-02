#ifndef _HG_cSkinnedGameObject_
#define _HG_cSkinnedGameObject_

#include <glm\vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <string>

#include "cSkinnedMesh.h"
#include "sAnimationState.h"

class cCamera3rdPerson;

class cSkinnedGameObject
{
public:

	enum Direction
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	glm::vec3 Position, Scale;
	glm::vec3 Forward;
	glm::quat OrientationQuat;
	glm::vec3 OrientationEuler;

	cSkinnedGameObject(std::string modelName, std::string modelDir, glm::vec3 position, glm::vec3 scale, glm::vec3 orientationEuler, std::vector<std::string> charAnimations);

	void setCamera(cCamera3rdPerson* theCamera);
	void Draw(cShaderProgram Shader, float deltaTime);
	void Movement(Direction direction, float deltaTime, bool shiftKey, bool ctrlKey);
	void Movement(glm::vec3 movementDirection, float deltaTime, bool shiftKey, bool ctrlKey);
	void controllerMovement(float xAxis, float yAxis, float deltaTime, bool running, bool LTDown);

	std::vector<std::string> vecCharacterAnimations;
	sAnimationState* defaultAnimState, *curAnimState;
	std::string animToPlay;
	float Speed;

	cSkinnedMesh * Model;

	float lastXAxis, lastYAxis;

	bool oneFrameEvent;
	int oneFrameStandWalkRun;
private:
	std::vector<glm::mat4> vecBoneTransformation;
	std::vector<glm::mat4> vecFinalTransformation;
	std::vector<glm::mat4> vecPrevTransformation;

	cCamera3rdPerson* myCamera;
};
#endif
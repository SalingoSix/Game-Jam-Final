#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/SOIL2.h>
#include <FMOD/fmod.hpp>
#include <FMOD/fmod_errors.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "cShaderProgram.h"
#include "cCamera.h"
#include "cCamera3rdPerson.h"
#include "cCameraStrafe.h"
#include "cCameraTransition.h"
#include "cMesh.h"
#include "cModel.h"
#include "cSkybox.h"
#include "cSkinnedMesh.h"
#include "cSkinnedGameObject.h"
#include "sAnimationState.h"
#include "cScreenQuad.h"
#include "cPlaneObject.h"
#include "cFrameBuffer.h"
#include "cTexture.h"
#include "cParticleEmitter.h"
#include "cLightManager.h"
#include "cFBO.h"
#include "cHDR.h"
#include "cShadowMap.h"
#include "cOmniShadowMap.h"
#include "nUtilities.h"
#include "cFireball.h"

//Setting up a camera GLOBAL
cCamera Camera(glm::vec3(0.0f, 15.0f, 25.0f),		//Camera Position
			glm::vec3(0.0f, 1.0f, 0.0f),		//World Up vector
			-30.0f,								//Pitch
			-90.0f);							//Yaw

cCamera3rdPerson* thirdPersonCam;
cCameraStrafe* strafeCam;
cCameraTransition* transitionCam;


glm::mat4 projection;
glm::mat4 view;
glm::mat4 viewProjectionMatrix;
glm::mat4 prevViewProjectionMatrix;
glm::mat4 lSpaceMatrix;
glm::mat4 model;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 400, lastY = 300;
float shootCooldown = 0.0f;

unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;

//For second pass render filters
int drawType = 1;
cFBO * gBufferPass, * deferredPass, * finalPass;
cFBO * blurPass;
cFBO * horizontalPass, *verticalPass;
cHDR * hdrPass;
cShadowMap * shadowPass;
cOmniShadowMap * omniShadowPass;
bool checkDistance = true;
bool checkGamma = false;
bool checkHDR = true;
bool isOmniDirectionalShadow = false;
float exposure = 0.75f;
float angle = 0.0f;
float farPlane = 0.0f;
bool horizontal = true;
bool firstIter = true;
bool isNormal = true;

//For animation
cSkinnedGameObject* robaxMan;
cSkinnedGameObject* robaxBoy;
cSkinnedGameObject* robaxSelected;
bool boyOrMan = false;
bool shiftLock = false;
bool spaceLock = false;
bool shiftKey = false;
bool ctrlKey = false;
bool LTDown = false;
bool cameraTransition = false;
bool jumpHappening = false;
bool actionHappening = false;


cTexture *fireTexture;

//For controller
int numAxes, numButtons;
const float* axes;
const unsigned char* buttons;

//For lights
cLightManager* LightManager;

std::map<std::string, cModel*> mapModelsToNames;
std::map<std::string, cShaderProgram*> mapShaderToName;

std::vector<cParticleEmitter*> vecParticleEmitters;
std::vector<glm::vec3> vecTreePositions;
std::vector<glm::vec3> vecBushPositions;
std::vector<bool> vecTreeIgnite;
std::vector<cFireball*> vecFireBalls;


FMOD_RESULT mResult;
FMOD::System * gSystem = NULL;
FMOD::Channel * gChannels[4];
FMOD::Sound * gSounds[4];



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void initFMOD();
void checkFMODError(FMOD_RESULT result);

void ShadowMap();
void GBuffer();
void Deferred();
void Final();
void Lights();
void Blur();

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialized GLAD" << std::endl;
		return -1;
	}

	initFMOD();

	mResult = gSystem->createSound("assets/sounds/grassWalk.wav", FMOD_CREATESTREAM, 0, &gSounds[0]);
	checkFMODError(mResult);
	mResult = gSystem->playSound(gSounds[0], 0, true, &gChannels[0]);
	checkFMODError(mResult);
	mResult = gChannels[0]->setMode(FMOD_LOOP_NORMAL);
	checkFMODError(mResult);
	mResult = gChannels[0]->setVolume(0.5f);
	checkFMODError(mResult);

	mResult = gSystem->createSound("assets/sounds/ambient.wav", FMOD_CREATESTREAM, 0, &gSounds[1]);
	checkFMODError(mResult);
	mResult = gSystem->playSound(gSounds[1], 0, true, &gChannels[1]);
	checkFMODError(mResult);
	mResult = gChannels[1]->setMode(FMOD_LOOP_NORMAL);
	checkFMODError(mResult);


	mResult = gSystem->createSound("assets/sounds/hadouken.mp3", FMOD_CREATESTREAM, 0, &gSounds[2]);
	checkFMODError(mResult);
	mResult = gSystem->playSound(gSounds[2], 0, true, &gChannels[2]);
	checkFMODError(mResult);

	mResult = gSystem->createSound("assets/sounds/treeFire.wav", FMOD_CREATESTREAM, 0, &gSounds[3]);
	checkFMODError(mResult);




	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	//Setting up global openGL state
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//Set up all our programs
	cShaderProgram* myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "vertShader.glsl", "fragShader.glsl");
	mapShaderToName["mainProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "animVert.glsl", "animFrag.glsl");
	mapShaderToName["skinProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "skyBoxVert.glsl", "skyBoxFrag.glsl");
	mapShaderToName["skyboxProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "modelVert.glsl", "modelFrag.glsl");
	mapShaderToName["simpleProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "quadVert.glsl", "quadFrag.glsl");
	mapShaderToName["quadProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "lampVert.glsl", "lampFrag.glsl");
	mapShaderToName["lightProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "deferVert.glsl", "deferFrag.glsl");
	mapShaderToName["deferredPass"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "shadowVert.glsl", "shadowFrag.glsl");
	mapShaderToName["shadowPass"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "omniShadowVert.glsl", "omniShadowFrag.glsl", "omniShadowGeom.glsl");
	mapShaderToName["omniShadowPass"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "blurVert.glsl", "blurFrag.glsl");
	mapShaderToName["blurPass"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "particleVert.glsl", "particleFrag.glsl");
	mapShaderToName["particleProgram"] = myProgram;


	//Loading our textures
	cTexture grassTexture("assets/textures/grass.png");
	cTexture fireBallTexture("assets/textures/fireball.jpg");
	cTexture bushTexture("assets/models/bush/texture_palm_128x128.png");
	cTexture treeTexture("assets/models/tree/FirBranches_Df.png");
	cTexture grassFloor("assets/textures/largeGrass.jpg");
	fireTexture = new cTexture("assets/textures/fire.png");

	int NUMBER_OF_OBJECTS = 200;
	for (int index = 0; index < NUMBER_OF_OBJECTS; index++)
	{
		bool isOK = true;
		glm::vec3 possibleSpot(0.0f);
		do
		{
			isOK = true;
			int randomX = rand() % 2001 - 1000;
			int randomZ = rand() % 2001 - 1000;
			float floatRandX = (float)randomX / 10.0f;
			float floatRandZ = (float)randomZ / 10.0f;

			possibleSpot = glm::vec3(floatRandX, 0.0f, floatRandZ);
			for (int treeIndex = 0; treeIndex < vecTreePositions.size(); treeIndex++)
			{
				if (glm::distance(possibleSpot, vecTreePositions[treeIndex]) < 2.0f)
				{
					isOK = false;
					break;
				}
			}
			for (int bushIndex = 0; bushIndex < vecBushPositions.size(); bushIndex++)
			{
				if (glm::distance(possibleSpot, vecBushPositions[bushIndex]) < 2.0f)
				{
					isOK = false;
					break;
				}
			}
		} while (isOK == false);

		if (index < NUMBER_OF_OBJECTS / 2)
		{
			vecTreePositions.push_back(possibleSpot);
			vecTreeIgnite.push_back(false);
		}
		else
		{
			vecBushPositions.push_back(possibleSpot);
		}
	}


	//Assemble all our models

	std::string path = "assets/models/floor/floor.obj";
	mapModelsToNames["Floor"] = new cModel(path);

	path = "assets/models/sphere/sphere.obj";
	mapModelsToNames["Sphere"] = new cModel(path);


	std::vector<glm::mat4> treeModels;
	for (int index = 0; index < vecTreePositions.size(); index++)
	{
		model = glm::mat4(1.0);
		model = glm::translate(model, vecTreePositions[index]);
		model = glm::scale(model, glm::vec3(nUtility::getRandInRange(0.005f, 0.015f)));
		treeModels.push_back(model);
		//mapShaderToName["mainProgram"]->setMat4("model", model);
		//glBindTexture(GL_TEXTURE_2D, treeTexture.ID);
		//mapModelsToNames["Tree"]->Draw(*mapShaderToName["mainProgram"]);
	}
	path = "assets/models/tree/Fir.obj";
	mapModelsToNames["Tree"] = new cModel(path, treeModels, treeTexture.ID);


	std::vector<glm::mat4> bushModels;
	for (int index = 0; index < vecBushPositions.size(); index++)
	{
		model = glm::mat4(1.0);
		model = glm::translate(model, vecBushPositions[index]);
		model = glm::scale(model, glm::vec3(nUtility::getRandInRange(0.005f, 0.015f)));
		bushModels.push_back(model);
		//mapShaderToName["mainProgram"]->setMat4("model", model);
		//glBindTexture(GL_TEXTURE_2D, bushTexture.ID);
		//mapModelsToNames["Bush"]->Draw(*mapShaderToName["mainProgram"]);
	}
	path = "assets/models/bush/bush01.obj";
	mapModelsToNames["Bush"] = new cModel(path, bushModels, bushTexture.ID);




	std::vector<std::string> vecAnims;
	vecAnims.push_back("assets/modelsFBX/IdleRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/WalkingRobaxacet.fbx");
	//vecAnims.push_back("assets/modelsFBX/ThrowRobaxacet.fbx");

	//vecAnims.push_back("assets/modelsFBX/StrafeLeftRobaxacet.fbx");
	//vecAnims.push_back("assets/modelsFBX/StrafeRightRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/RunningBackwardRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/WalkingBackwardRobaxacet.fbx");
	//vecAnims.push_back("assets/modelsFBX/LeftTurnRobaxacet.fbx");


	vecAnims.push_back("assets/modelsFBX/RunningJumpRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/StandingJumpRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/RunningRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/Jog Strafe Left.fbx");
	vecAnims.push_back("assets/modelsFBX/Jog Strafe Right.fbx");
	vecAnims.push_back("assets/modelsFBX/Fireball.fbx");

	robaxMan = new cSkinnedGameObject("RobaxacetMan", "assets/modelsFBX/IdleRobaxacet.fbx",
		glm::vec3(0.0f, 0.0f, 5.0f),
		glm::vec3(0.025f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		vecAnims);

	robaxSelected = robaxMan;

	thirdPersonCam = new cCamera3rdPerson(robaxMan, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);
	strafeCam = new cCameraStrafe(robaxMan, glm::vec3(0.0f, 1.0f, 0.0f));
	transitionCam = new cCameraTransition(thirdPersonCam, strafeCam, glm::vec3(0.0f, 1.0f, 0.0f));

	robaxMan->setCamera(thirdPersonCam);

	//Creating a frame buffer
	cFrameBuffer mainFrameBuffer(SCR_HEIGHT, SCR_WIDTH);
	gBufferPass= new cFBO(SCR_WIDTH, SCR_HEIGHT, false);
	deferredPass = new cFBO(SCR_WIDTH, SCR_HEIGHT, false);
	blurPass = new cFBO(SCR_WIDTH, SCR_HEIGHT, false);
	finalPass = new cFBO(SCR_WIDTH, SCR_HEIGHT, false);
	hdrPass = new cHDR(SCR_WIDTH, SCR_HEIGHT, false);
	horizontalPass = new cFBO(SCR_WIDTH, SCR_HEIGHT, false);
	verticalPass = new cFBO(SCR_WIDTH, SCR_HEIGHT, false);
	shadowPass = new cShadowMap(2048, 2048);
	omniShadowPass = new cOmniShadowMap(4096, 4096);
	//Some simple shapes
	cScreenQuad screenQuad;
	cPlaneObject planeObject;

	//Prepare some particle VBO
	const int MaxParticles = 10000;
	glBindVertexArray(planeObject.VAO);

	// The VBO containing the positions and sizes of the particles
	GLuint particles_position_buffer;
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	//Load the skyboxes
	cSkybox skybox("assets/textures/skybox/");

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	mapShaderToName["skyboxProgram"]->useProgram();
	mapShaderToName["skyboxProgram"]->setInt("skybox", 0);

	mapShaderToName["mainProgram"]->useProgram();
	mapShaderToName["mainProgram"]->setInt("skybox", 0);
	mapShaderToName["mainProgram"]->setInt("reflectRefract", 0);

	mapShaderToName["quadProgram"]->useProgram();








	mapShaderToName["deferredPass"]->useProgram();
	mapShaderToName["deferredPass"]->setFloat("screenWidth", SCR_WIDTH);
	mapShaderToName["deferredPass"]->setFloat("screenHeight", SCR_HEIGHT);
	mapShaderToName["deferredPass"]->setInt("screenTexture", 0);
	mapShaderToName["deferredPass"]->setInt("normalTexture", 1);
	mapShaderToName["deferredPass"]->setInt("vertexTexture", 2);
	mapShaderToName["deferredPass"]->setInt("motionTexture", 3);
	mapShaderToName["deferredPass"]->setInt("depthTexture", 5);









	LightManager = new cLightManager();
	LightManager->CreateLights(5, 5);
	LightManager->LoadLampsIntoShader(*mapShaderToName["lightProgram"]);


	double nbFrames = 0;
	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		int controllerPresent = glfwJoystickPresent(GLFW_JOYSTICK_1);
		if (controllerPresent == 1)
		{
			axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &numAxes);
			buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &numButtons);
			const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);
		}
		processInput(window);

		mResult = gSystem->update();
		checkFMODError(mResult);

		bool isAmbientPaused;
		mResult = gChannels[1]->getPaused(&isAmbientPaused);
		checkFMODError(mResult);
		if (isAmbientPaused)
		{
			mResult = gChannels[1]->setPaused(false);
			checkFMODError(mResult);
		}


		if (shootCooldown > 0.0f)
		{
			shootCooldown -= deltaTime;
			if (shootCooldown <= 0.0f)
			{
				shootCooldown = 0.0f;
			}
		}






		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		nbFrames++;
		if (currentFrame - lastTime >= 1.0)
		{
			printf("%f ms/frame\n", 1000.0 / nbFrames);
			nbFrames = 0;
			lastTime += 1.0f;
		}

		LightManager->LoadLightsIntoShader(*mapShaderToName["deferredPass"]);

		if (cameraTransition)
		{
			transitionCam->updateCamera(deltaTime);
			if (transitionCam->isFinished())
			{
				cameraTransition = false;
			}
		}

		if (LTDown)
		{
			strafeCam->playerMovement();
		}
		else
		{
			thirdPersonCam->playerMovement();
		}



		if (!isOmniDirectionalShadow)
		{
			shadowPass->BindFBO();
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, 2048, 2048);
			glCullFace(GL_FRONT);
			mapShaderToName["shadowPass"]->useProgram();

			float nearPlane = 1.0f, farPlane = 20.0f;
			glm::mat4 lProjection = glm::ortho(-40.f, 40.f, -40.f, 40.f, nearPlane, farPlane);
			//glm::mat4 lProjection = glm::perspective(glm::radians(angle), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 50.0f);
			glm::mat4 lView = glm::lookAt(glm::vec3(-4.0f, 10.0f, -4.0f) + robaxMan->Position, robaxMan->Position, glm::vec3(0.0f, 1.0f, 0.0f));
			lSpaceMatrix = lProjection * lView;

			mapShaderToName["shadowPass"]->setMat4("lightSpaceMatrix", lSpaceMatrix);


			glm::mat4 model = glm::mat4(1.0);
			model = glm::scale(model, glm::vec3(100.0f, 2.0f, 100.0f));
			mapShaderToName["shadowPass"]->setMat4("model", model);
			mapModelsToNames["Floor"]->Draw(*mapShaderToName["shadowPass"]);

			//for (int index = 0; index < vecTreePositions.size(); index++)
			//{
			//	model = glm::mat4(1.0);
			//	model = glm::translate(model, vecTreePositions[index]);
			//	model = glm::scale(model, glm::vec3(0.01f));
			//	mapShaderToName["shadowPass"]->setMat4("model", model);
			//	glBindTexture(GL_TEXTURE_2D, treeTexture.ID);
			//	mapModelsToNames["Tree"]->Draw(*mapShaderToName["shadowPass"]);
			//}
			
			mapShaderToName["shadowPass"]->setBool("isInstanced", true);
			mapModelsToNames["Tree"]->Draw(*mapShaderToName["shadowPass"]);
			mapModelsToNames["Bush"]->Draw(*mapShaderToName["shadowPass"]);
			mapShaderToName["shadowPass"]->setBool("isInstanced", false);


			//for (int index = 0; index < vecBushPositions.size(); index++)
			//{
			//	model = glm::mat4(1.0);
			//	model = glm::translate(model, vecBushPositions[index]);
			//	model = glm::scale(model, glm::vec3(0.01f));
			//	mapShaderToName["shadowPass"]->setMat4("model", model);
			//	glBindTexture(GL_TEXTURE_2D, bushTexture.ID);
			//	mapModelsToNames["Bush"]->Draw(*mapShaderToName["shadowPass"]);
			//}

			/*	model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-10.0f, 2.0f, 0.0f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(100.0f, 1.0f, 100.0f));
			mapShaderToName["mainProgram"]->setMat4("model", model);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
			mapModelsToNames["Fence"]->Draw(*mapShaderToName["mainProgram"]);*/

			//Draw a quad with our grass texture on it
			////glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			//mapShaderToName["shadowPass"]->useProgram();
			//mapShaderToName["simpleProgram"]->setMat4("projection", projection);
			//mapShaderToName["simpleProgram"]->setMat4("view", view);



			for (int index = 0; index < vecParticleEmitters.size(); index++)
			{
				vecParticleEmitters[index]->Update(deltaTime);
				//Draw all our particles
				for (int i = 0; i < vecParticleEmitters[index]->particles.size(); i++)
				{
					if (vecParticleEmitters[index]->particles[i]->Life > 0.0f)
					{
						model = glm::mat4(1.0f);
						model = glm::translate(model, vecParticleEmitters[index]->particles[i]->Position);
						if (LTDown)
						{
							model = glm::rotate(model, robaxSelected->OrientationEuler.y, glm::vec3(0.0f, 1.0f, 0.0f));
						}
						else
						{
							model = glm::rotate(model, thirdPersonCam->cameraRotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
						}
						mapShaderToName["shadowPass"]->setMat4("model", model);
						glBindVertexArray(planeObject.VAO);
						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					}
				}
				if (vecParticleEmitters[index]->deleteMe == true)
				{
					vecParticleEmitters.erase(vecParticleEmitters.begin() + index);
					index--;
				}
			}
			if (robaxSelected == robaxMan)
			{
				mapShaderToName["shadowPass"]->setInt("activeBoy", 1);
			}
			mapShaderToName["shadowPass"]->setBool("isSkinnedMesh", true);
			robaxMan->Draw(*mapShaderToName["shadowPass"], deltaTime);

			mapShaderToName["shadowPass"]->setInt("activeBoy", 0);
			mapShaderToName["shadowPass"]->setBool("isSkinnedMesh", false);

			if (jumpHappening)
			{
				jumpHappening = robaxSelected->oneFrameEvent;
			}
			glCullFace(GL_BACK);
			shadowPass->UnbindFBO();
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		}

		mapShaderToName["mainProgram"]->useProgram();
		mapShaderToName["mainProgram"]->setVec3("spotLight.position", thirdPersonCam->position);
		mapShaderToName["mainProgram"]->setVec3("spotLight.direction", thirdPersonCam->front);

		glm::mat4 projection, view, skyboxView;

		if (cameraTransition)
		{
			mapShaderToName["mainProgram"]->setVec3("cameraPos", transitionCam->position);
			projection = glm::perspective(glm::radians(transitionCam->zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = transitionCam->getViewMatrix();
			skyboxView = glm::mat4(glm::mat3(transitionCam->getViewMatrix()));
		}
		else if (LTDown)
		{
			mapShaderToName["mainProgram"]->setVec3("cameraPos", strafeCam->position);
			projection = glm::perspective(glm::radians(strafeCam->zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = strafeCam->getViewMatrix();
			skyboxView = glm::mat4(glm::mat3(strafeCam->getViewMatrix()));
			mapShaderToName["deferredPass"]->useProgram();
			mapShaderToName["deferredPass"]->setVec3("cameraPos", strafeCam->position);
			mapShaderToName["mainProgram"]->useProgram();
		}
		else
		{
			mapShaderToName["mainProgram"]->setVec3("cameraPos", thirdPersonCam->position);
			projection = glm::perspective(glm::radians(thirdPersonCam->zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = thirdPersonCam->getViewMatrix();
			skyboxView = glm::mat4(glm::mat3(thirdPersonCam->getViewMatrix()));

			mapShaderToName["deferredPass"]->useProgram();
			mapShaderToName["deferredPass"]->setVec3("cameraPos", thirdPersonCam->position);
			mapShaderToName["mainProgram"]->useProgram();
		}





		//Begin writing to another frame buffer
		gBufferPass->BindFBO();
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	/*	mapShaderToName["lightProgram"]->useProgram();
		mapShaderToName["lightProgram"]->setMat4("projection", projection);
		mapShaderToName["lightProgram"]->setMat4("view", view);
		Lights();*/

		mapShaderToName["deferredPass"]->useProgram();
		mapShaderToName["deferredPass"]->setBool("checkDistance", checkDistance);
		mapShaderToName["mainProgram"]->useProgram();

		model = glm::mat4(1.0);
		model = glm::scale(model, glm::vec3(100.0f, 1.0f, 100.0f));
		mapShaderToName["mainProgram"]->setMat4("model", model);
		mapShaderToName["mainProgram"]->setMat4("projection", projection);
		mapShaderToName["mainProgram"]->setMat4("view", view);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, grassFloor.ID);
		mapModelsToNames["Floor"]->Draw(*mapShaderToName["mainProgram"]);
		
		
		mapModelsToNames["Tree"]->Draw(*mapShaderToName["mainProgram"]);
		mapModelsToNames["Bush"]->Draw(*mapShaderToName["mainProgram"]);

		//Draw a quad with our grass texture on it
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		mapShaderToName["simpleProgram"]->useProgram();
		mapShaderToName["simpleProgram"]->setMat4("projection", projection);
		mapShaderToName["simpleProgram"]->setMat4("view", view);


		//Draw all the fire balls
		for (int index = 0; index < vecFireBalls.size(); index++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, vecFireBalls[index]->Position);
			//model = glm::scale(model, glm::vec3(0.01f));
			mapShaderToName["mainProgram"]->setMat4("model", model);
			glBindTexture(GL_TEXTURE_2D, fireBallTexture.ID);
			mapModelsToNames["Sphere"]->Draw(*mapShaderToName["mainProgram"]);
			vecFireBalls[index]->Update(deltaTime);

			if (vecFireBalls[index]->Position.y <= 1.0f)
			{
				glm::vec3 starterPos = glm::vec3(vecFireBalls[index]->Position.x, 0.0f, vecFireBalls[index]->Position.z);
				cParticleEmitter* groundSmolder = new cParticleEmitter(50, starterPos, fireTexture->ID, glm::vec3(-0.4f, 0.0f, -0.4f), glm::vec3(0.4f, 1.0f, 0.4f), 1.0f, 1.0f, 25.0f, 5.0f);
				vecParticleEmitters.push_back(groundSmolder);
				vecFireBalls[index]->myEmitter->timed = true;
				vecFireBalls[index]->deleteMe = true;
			}

			else if (vecFireBalls[index]->Position.y <= 9.0f)
			{
				glm::vec3 groundedPos = glm::vec3(vecFireBalls[index]->Position.x, 0.0f, vecFireBalls[index]->Position.z);
				for (int treeIndex = 0; treeIndex < vecTreePositions.size(); treeIndex++)
				{
					float distToTree = glm::distance(groundedPos, vecTreePositions[treeIndex]);
					if (distToTree < 2.0f)
					{
						if (vecTreeIgnite[treeIndex] == false)
						{

							glm::vec3 burnPos = glm::vec3(vecTreePositions[treeIndex].x, 2.0f, vecTreePositions[treeIndex].z);
							cParticleEmitter* groundSmolder = new cParticleEmitter(200, burnPos, fireTexture->ID, glm::vec3(-0.4f, 1.0f, -0.4f), glm::vec3(0.4f, 1.0f, 0.4f), 3.0f, 1.0f, 65.0f, 0.0f);
							vecParticleEmitters.push_back(groundSmolder);
							vecTreeIgnite[treeIndex] = true;
							mResult = gSystem->playSound(gSounds[3], 0, false, &gChannels[3]);
							checkFMODError(mResult);
							mResult = gChannels[3]->setMode(FMOD_LOOP_NORMAL);
							checkFMODError(mResult);
						}
						vecFireBalls[index]->myEmitter->timed = true;
						vecFireBalls[index]->deleteMe = true;
						break;
					}
				}
			}

			if (vecFireBalls[index]->deleteMe == true)
			{
				vecFireBalls.erase(vecFireBalls.begin() + index);
				index--;
			}
		}

		mapShaderToName["simpleProgram"]->useProgram();
		mapShaderToName["simpleProgram"]->setMat4("projection", projection);
		mapShaderToName["simpleProgram"]->setMat4("view", view);

		int ParticlesToDraw = 0;
		glm::vec3 ParticlePos[MaxParticles];
		glm::mat4 ParticleMat[MaxParticles];

		for (int index = 0; index < vecParticleEmitters.size(); index++)
		{
			vecParticleEmitters[index]->Update(deltaTime);
			for (int i = 0; i < vecParticleEmitters[index]->particles.size(); i++)
			{
				if (vecParticleEmitters[index]->particles[i]->Life > 0.0f)
				{
					ParticlePos[ParticlesToDraw++] = vecParticleEmitters[index]->particles[i]->Position;
				}
			}
			if (vecParticleEmitters[index]->deleteMe == true)
			{
				vecParticleEmitters.erase(vecParticleEmitters.begin() + index);
				index--;
			}
		}

		glBindVertexArray(planeObject.VAO);

		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesToDraw * sizeof(GLfloat) * 3, ParticlePos);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glVertexAttribPointer(
			3, // attribute. No particular reason for 0, but must match the layout in the shader.
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0, // stride
			(void*)0 // array buffer offset
		);

		model = glm::mat4(1.0f);
		if (LTDown)
		{
			//model = glm::rotate(model, robaxSelected->OrientationEuler.y, glm::vec3(0.0f, 1.0f, 0.0f));
			mapShaderToName["particleProgram"]->setFloat("rotationValue", robaxSelected->OrientationEuler.y);
		}
		else
		{
			//model = glm::rotate(model, thirdPersonCam->cameraRotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			mapShaderToName["particleProgram"]->setFloat("rotationValue", thirdPersonCam->cameraRotAngle);
		}
		mapShaderToName["particleProgram"]->setMat4("model", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fireTexture->ID);
		mapShaderToName["particleProgram"]->setInt("texture_diffuse1", 0);
		glBindVertexArray(planeObject.VAO);

		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 0); // particles normals : always reuse
		glVertexAttribDivisor(2, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(3, 1); // positions : one per quad (its center) -> 1

		glDrawElementsInstanced(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0, ParticlesToDraw);



		//Drawing the main scene's skybox
		mapShaderToName["skyboxProgram"]->useProgram();

		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		mapShaderToName["skyboxProgram"]->setMat4("projection", projection);
		mapShaderToName["skyboxProgram"]->setMat4("view", skyboxView);

		glBindVertexArray(skybox.VAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS); // set depth function back to default





		//Here am rendering skinned meshes
		mapShaderToName["skinProgram"]->useProgram();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		mapShaderToName["skinProgram"]->setMat4("projection", projection);
		mapShaderToName["skinProgram"]->setMat4("view", view);
		if (robaxSelected == robaxMan)
		{
			mapShaderToName["skinProgram"]->setInt("activeBoy", 1);
		}
		robaxMan->Draw(*mapShaderToName["skinProgram"], deltaTime);

		mapShaderToName["skinProgram"]->setInt("activeBoy", 0);

		if (jumpHappening)
		{
			jumpHappening = robaxSelected->oneFrameEvent;
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		gBufferPass->UnbindFBO();


		if (checkHDR)
		{
			hdrPass->BindFBO();
			Deferred();
			hdrPass->UnbindFBO();
			//blurPass->BindFBO();
			//Blur();
			//blurPass->UnbindFBO();
		}
		else
		{
			deferredPass->BindFBO();
			Deferred();
			deferredPass->UnbindFBO();
		}


		Final();
		






		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}


void GBuffer()
{

}
void Deferred()
{
	glDisable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	mapShaderToName["deferredPass"]->useProgram();
	mapShaderToName["deferredPass"]->setInt("drawType", drawType);
	mapShaderToName["deferredPass"]->setInt("isNormal", isNormal);

	mapShaderToName["deferredPass"]->setFloat("nearPlane", 0.1f);
	mapShaderToName["deferredPass"]->setFloat("farPlane", 100.0f);

	mapShaderToName["deferredPass"]->setMat4("lightSpaceMatrix", lSpaceMatrix);
	mapShaderToName["deferredPass"]->setMat4("viewProjectionMatrix", projection * view);
	mapShaderToName["deferredPass"]->setMat4("prevViewProjectionMatrix", prevViewProjectionMatrix);
	prevViewProjectionMatrix = projection * view;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBufferPass->texColorBuffer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBufferPass->texNormalBuffer);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBufferPass->texVertexBuffer);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBufferPass->texMotionBuffer);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gBufferPass->texDepthBuffer);

	if (isOmniDirectionalShadow)
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, omniShadowPass->texDepthBuffer);
		mapShaderToName["deferredPass"]->setInt("omniShadowTexture", 4);
		mapShaderToName["deferredPass"]->setBool("isOmniDirectional", isOmniDirectionalShadow);
		mapShaderToName["deferredPass"]->setFloat("omniFarPlane", farPlane);
	}
	else
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowPass->texDepthBuffer);
		mapShaderToName["deferredPass"]->setInt("shadowTexture", 4);
		mapShaderToName["deferredPass"]->setBool("isOmniDirectional", isOmniDirectionalShadow);
	}

	deferredPass->Draw();
}
void Final()
{
	glDisable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Paste the entire scene onto a quad as a single texture
	mapShaderToName["quadProgram"]->useProgram();
	mapShaderToName["quadProgram"]->setFloat("screenWidth", SCR_WIDTH);
	mapShaderToName["quadProgram"]->setFloat("screenHeight", SCR_HEIGHT);
	mapShaderToName["quadProgram"]->setFloat("exposure", exposure);
	mapShaderToName["quadProgram"]->setBool("checkGamma", checkGamma);
	mapShaderToName["quadProgram"]->setBool("checkHDR", checkHDR);
	
	if (checkHDR)
	{
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, hdrPass->texColorBuffer);
		mapShaderToName["quadProgram"]->setInt("screenTexture", 10);
	}
	else
	{
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, deferredPass->texColorBuffer);
		mapShaderToName["quadProgram"]->setInt("screenTexture", 10);
	}


	finalPass->Draw();
}
void Lights()
{
	glEnable(GL_DEPTH_TEST);
	LightManager->DrawLightsIntoScene(*mapShaderToName["lightProgram"]);
}

//void Blur()
//{
//	unsigned int amount = 10;
//	mapShaderToName["blurPass"]->useProgram();
//
//	for (int i = 0; i != amount; i++)
//	{
//		if (horizontal)
//		{
//			horizontalPass->BindFBO();
//			mapShaderToName["blurPass"]->setFloat("horizontal", horizontal);
//
//			glActiveTexture(GL_TEXTURE0);
//			glBindTexture(GL_TEXTURE_2D, firstIter ? hdrPass->texBloomBuffer : horizontalPass->texColorBuffer);
//			mapShaderToName["blurPass"]->setInt("blurTexture", firstIter ? hdrPass->texBloomBuffer : horizontalPass->texColorBuffer);
//			horizontalPass->Draw();
//			horizontal = !horizontal;
//			if (firstIter)
//				firstIter = false;
//		}
//		else
//		{
//			verticalPass->BindFBO();
//			mapShaderToName["blurPass"]->setFloat("horizontal", horizontal);
//			glActiveTexture(GL_TEXTURE0);
//			glBindTexture(GL_TEXTURE_2D, firstIter ? hdrPass->texBloomBuffer : horizontalPass->texColorBuffer);
//			mapShaderToName["blurPass"]->setInt("blurTexture", firstIter ? hdrPass->texBloomBuffer : horizontalPass->texColorBuffer);
//			horizontalPass->Draw();
//			horizontal = !horizontal;
//		}
//	}
//
//	verticalPass->UnbindFBO();
//}















void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	return;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float magnitude = sqrt(pow(axes[0], 2) + pow(axes[1], 2));
	bool fastEnough = false;
	if (magnitude >= 0.75f)
		fastEnough = true;

	if (axes[4] > 0.0f)
	{
		if (!LTDown)
		{	//LT was just pushed down on this frame
			//Transition to the behind-the-back cam
			cameraTransition = true;
			strafeCam->yLook = 0.0f;
			strafeCam->processController(axes[2], axes[3], deltaTime);
			transitionCam->thirdToStrafe();
		}
		LTDown = true;
	}
	else
	{
		if (LTDown)
		{	//LT has just been released
			//Transition back to regular cam
			cameraTransition = true;
			thirdPersonCam->returnFromStrafe();
			transitionCam->strafeToThird();
		}
		LTDown = false;
	}

	if (!jumpHappening && (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || buttons[0] == GLFW_PRESS))
	{
		jumpHappening = true;
		robaxSelected->oneFrameEvent = true;
		robaxSelected->curAnimState->defaultAnimation.currentTime = 0.0f;
		robaxSelected->lastXAxis = axes[0];
		robaxSelected->lastYAxis = -axes[1];
		if (glfwGetKey(window, GLFW_KEY_I) || glfwGetKey(window, GLFW_KEY_K) || glfwGetKey(window, GLFW_KEY_J) || glfwGetKey(window, GLFW_KEY_L) || axes[0] != 0.0f || axes[1] != 0.0f)
		{
			jumpHappening = true;
			robaxSelected->oneFrameEvent = true;
			robaxSelected->curAnimState->defaultAnimation.currentTime = 0.0f;
			robaxSelected->animToPlay = "assets/modelsFBX/Fireball.fbx";
			robaxSelected->oneFrameStandWalkRun = 0;		
		}
		else
		{
			robaxSelected->animToPlay = "assets/modelsFBX/StandingJumpRobaxacet.fbx";
			robaxSelected->oneFrameStandWalkRun = 0;
		}
	}

	if (!jumpHappening && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		jumpHappening = true;
		robaxSelected->oneFrameEvent = true;
		robaxSelected->curAnimState->defaultAnimation.currentTime = 0.0f;
		robaxSelected->oneFrameStandWalkRun = 0;
		robaxSelected->animToPlay = "assets/modelsFBX/ThrowRobaxacet.fbx";
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || buttons[1] == GLFW_PRESS)
	{
		shiftKey = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE || buttons[1] == GLFW_RELEASE)
	{
		shiftKey = false;
	}

	if (!jumpHappening && glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
	{
		if (!shiftLock)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
			if (boyOrMan)
			{
				robaxSelected = robaxBoy;
			}
			else
			{
				robaxSelected = robaxMan;
			}
			boyOrMan = !boyOrMan;
		}
		shiftLock = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE)
	{
		shiftLock = false;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		ctrlKey = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
	{
		ctrlKey = false;
	}

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		drawType = 1;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		drawType = 2;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		drawType = 3;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		drawType = 4;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		drawType = 5;
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		drawType = 6;
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
		drawType = 7;
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
		drawType = 8;
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
		drawType = 9;

	if (glfwGetKey(window, GLFW_KEY_KP_DIVIDE) == GLFW_PRESS)
		checkDistance = true;
	if (glfwGetKey(window, GLFW_KEY_KP_MULTIPLY) == GLFW_PRESS)
		checkDistance = false;
	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
	{
		exposure += 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
	{
		//LightManager->DirectionalLight.ambient += glm::vec3(0.1f);
		//LightManager->DirectionalLight.diffuse += glm::vec3(0.1f);
		exposure -= 0.1f;
	}

	if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
		checkGamma = true;
	if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
		checkGamma = false;

	if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS)
		checkHDR = true;
	if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
		checkHDR = false;

	if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
		angle -= 2.5f;
	if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
		angle += 2.5f;

	if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
		isOmniDirectionalShadow = true;
	if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
		isOmniDirectionalShadow = false;

	


	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		LightManager->DirectionalLight.direction.x -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		LightManager->DirectionalLight.direction.x += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		LightManager->DirectionalLight.direction.z += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		LightManager->DirectionalLight.direction.z -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		LightManager->DirectionalLight.direction.y -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		LightManager->DirectionalLight.direction.y += 0.1f;

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		isNormal = true;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		isNormal = false;


	//Camera control by WASD keys
	//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	//{
	//	thirdPersonCam->processKeyboard(Camera_Movement_3rd::INWARD, deltaTime);
	//}
	//else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	//{
	//	thirdPersonCam->processKeyboard(Camera_Movement_3rd::OUTWARD, deltaTime);
	//}		
	//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	//{
	//	thirdPersonCam->processKeyboard(Camera_Movement_3rd::CCW, deltaTime);
	//}
	//else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	//{
	//	thirdPersonCam->processKeyboard(Camera_Movement_3rd::CW, deltaTime);
	//}

	//Pass off camera control to the controller
	if (LTDown || (axes[2] != 0.0f || axes[3] != 0.0f))
	{
		if (LTDown)
		{
			strafeCam->processController(axes[2], axes[3], deltaTime);
		}
		else
		{
			thirdPersonCam->processController(axes[2], axes[3], deltaTime);
		}
	}

	//Pass off camera control to the controller
	if (!jumpHappening && (axes[0] != 0.0f || axes[1] != 0.0f))
	{
		robaxSelected->controllerMovement(axes[0], -axes[1], deltaTime, fastEnough, LTDown);
		mResult = gChannels[0]->setPaused(false);
		checkFMODError(mResult);

		if (LTDown)
		{
			if(axes[0] < -0.25f)
				robaxSelected->animToPlay = "assets/modelsFBX/Jog Strafe Left.fbx";
			else if(axes[0] > 0.25f)
				robaxSelected->animToPlay = "assets/modelsFBX/Jog Strafe Right.fbx";
			else
				robaxSelected->animToPlay = "assets/modelsFBX/RunningRobaxacet.fbx";
		}
		else
		{

			if (fastEnough && !ctrlKey)
			{
				robaxSelected->animToPlay = "assets/modelsFBX/RunningRobaxacet.fbx";
			}
			else if (!ctrlKey)
			{
				robaxSelected->animToPlay = "assets/modelsFBX/WalkingRobaxacet.fbx";
			}
			else
			{
				robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
			}
		}
	}
	else if (!jumpHappening)
	{
		robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
		bool isPaused;
		mResult = gChannels[0]->getPaused(&isPaused);
		checkFMODError(mResult);
		if (!isPaused)
		{
			mResult = gChannels[0]->setPaused(true);
			checkFMODError(mResult);
		}
	}

	if (!jumpHappening && LTDown && !cameraTransition && buttons[5] == GLFW_PRESS)
	{
		if (shootCooldown == 0.0f)
		{
			glm::vec3 fireStartPosition = robaxSelected->Position + strafeCam->front;
			fireStartPosition.y = 3.0f;
			cParticleEmitter* newShot = new cParticleEmitter(50, fireStartPosition, fireTexture->ID, glm::vec3(-0.1f, -1.0f, -0.1f), glm::vec3(0.1f, -1.0f, 0.1f), 1.0f, 1.0f, 10.0f, 0.0f);
			cFireball* newBall = new cFireball(fireStartPosition, strafeCam->front, newShot);
			vecParticleEmitters.push_back(newShot);
			vecFireBalls.push_back(newBall);
			shootCooldown = 0.75f;



			jumpHappening = true;
			robaxSelected->oneFrameEvent = true;
			robaxSelected->curAnimState->defaultAnimation.currentTime = 0.0f;
			//robaxSelected->curAnimState->defaultAnimation.frameStepTime 
			robaxSelected->animToPlay = "assets/modelsFBX/Fireball.fbx";
			robaxSelected->oneFrameStandWalkRun = 0;
			
			/*bool isShooting;
			mResult = gChannels[2]->getPaused(&isShooting);
			checkFMODError(mResult);
			if (!isShooting)
			{
				gChannels[2]->setPaused(false);
			}*/
			mResult = gSystem->playSound(gSounds[2], 0, false, &gChannels[2]);
			checkFMODError(mResult);
		}
	}
	else if (buttons[5] == GLFW_RELEASE)
	{
		shootCooldown = 0.0f;
	}

	//glm::vec3 movementDirection(0.0f);
	//bool amIWalking = false;

	////Movement "forward"
	//if (!jumpHappening && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	//{
	//	movementDirection.z = -1.0f;
	//	amIWalking = true;
	//	if (shiftKey && !ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/RunningRobaxacet.fbx";
	//	}
	//	else if (!ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/WalkingRobaxacet.fbx";
	//	}
	//	else
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
	//	}
	//}
	////Movement "backward"
	//if (!jumpHappening && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	//{
	//	movementDirection.z = 1.0f;
	//	amIWalking = true;
	//	if (shiftKey && !ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/RunningRobaxacet.fbx";
	//	}
	//	else if (!ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/WalkingRobaxacet.fbx";
	//	}
	//	else
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
	//	}
	//}
	//if (!jumpHappening && glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	//{
	//	movementDirection.x = -1.0f;
	//	amIWalking = true;
	//	if (ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/StrafeLeftRobaxacet.fbx";
	//	}
	//	else if (shiftKey && !ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/RunningRobaxacet.fbx";
	//	}
	//	else if (!ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/WalkingRobaxacet.fbx";
	//	}
	//	else
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
	//	}
	//}
	//if (!jumpHappening && glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	//{
	//	movementDirection.x = 1.0f;
	//	amIWalking = true;
	//	if (ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/StrafeRightRobaxacet.fbx";
	//	}
	//	else if (shiftKey && !ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/RunningRobaxacet.fbx";
	//	}
	//	else if (!ctrlKey)
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/WalkingRobaxacet.fbx";
	//	}
	//	else
	//	{
	//		robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
	//	}
	//}

	//if (amIWalking)
	//{
	//	movementDirection = glm::normalize(movementDirection);
	//	robaxSelected->Movement(movementDirection, deltaTime, shiftKey, ctrlKey, thirdPersonCam);
	//}
	//else if (!jumpHappening)
	//{
	//	robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
	//}
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) // this bool variable is initially set to true
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	Camera.processMouseMovement(xOffset, yOffset, true);



}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera.processMouseScroll(yoffset);
}


void checkFMODError(FMOD_RESULT result) {
	if (result != FMOD_OK)
	{
		fprintf(stderr, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		Sleep(3000);
		exit(-1);
	}
}


void initFMOD()
{
	mResult = FMOD::System_Create(&gSystem);
	checkFMODError(mResult);

	mResult = gSystem->init(512, FMOD_INIT_NORMAL, 0);
	checkFMODError(mResult);
}
//
// AVT 2025: Texturing with Phong Shading and Text rendered with TrueType library
// The text rendering was based on https://dev.to/shreyaspranav/how-to-render-truetype-fonts-in-opengl-using-stbtruetypeh-1p5k
// You can also learn an alternative with FreeType text: https://learnopengl.com/In-Practice/Text-Rendering
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
//
// Author: João Madeiras Pereira
//

#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>

#include "renderer.h"
#include "shader.h"
#include "mathUtility.h"
#include "model.h"
#include "texture.h"
#include "sceneObject.h"
#include "light.h"
#include "drone.cpp"

using namespace std;

#ifndef RESOURCE_BASE
#define RESOURCE_BASE "resources/"
#endif

#define CAPTION "AVT 2025 Welcome Demo"
int WindowHandle = 0;
int WinX = 1024, WinY = 695;

unsigned int FrameCount = 0;

// Object of class gmu (Graphics Math Utility) to manage math and matrix operations
gmu mu;

// Object of class renderer to manage the rendering of meshes and ttf-based bitmap text
Renderer renderer;

// Scene objects
std::vector<SceneObject *> sceneObjects;
std::vector<Light *> sceneLights;

// Collision system
CollisionSystem collisionSystem;

// Cameras
Camera *cams[3];
int activeCam = 0;

// Mouse Tracking Variables
int startX, startY, tracking = 0;
float mouseSensitivity = 0.3f;

// Frame counting and FPS computation
float lastTime = glutGet(GLUT_ELAPSED_TIME);
char s[32];

bool fontLoaded = false;

/// ::::::::::::::::::::::::::::::::::::::::::::::::CALLBACK FUNCIONS:::::::::::::::::::::::::::::::::::::::::::::::::://///

void timer(int value)
{
	(void)value;

	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();

	// std::cout << s << std::endl;

	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;

	// Every second
	glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{
	(void)value;

	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
}

void gameloop(void)
{
	float currentTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (currentTime - lastTime) / 1000.0f; // Convert milliseconds to seconds
	lastTime = currentTime;

	// Update all scene objects with the elapsed time
	for (size_t i = 0; i < sceneObjects.size(); i++)
	{
		sceneObjects[i]->update(deltaTime);
	}

	glutPostRedisplay();
}
// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h)
{
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);

	WinX = w;
	WinY = h;

	mu.loadIdentity(gmu::PROJECTION);

	if (cams[activeCam]->getProjectionType() == ProjectionType::Orthographic)
	{
		float viewSize = 30.0f;
		mu.ortho(-viewSize, viewSize, -viewSize, viewSize, -100.0f, 100.0f);
	}
	else
	{
		float ratio = (1.0f * w) / h;
		mu.perspective(53.13f, ratio, 0.1f, 1000.0f);
	}
}

// ------------------------------------------------------------
//
// Render stufff
//

void renderSim(void)
{
	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the required GLSL program to draw the meshes with illumination
	renderer.activateRenderMeshesShaderProg();
	renderer.resetLights();

	// Associar os Texture Units aos Objects Texture
	// stone.tga loaded in TU0; checker.tga loaded in TU1;  lightwood.tga loaded in TU2
	renderer.setTexUnit(0, 0);
	renderer.setTexUnit(1, 1);
	renderer.setTexUnit(2, 2);

	// load identity matrices
	mu.loadIdentity(gmu::VIEW);
	mu.loadIdentity(gmu::MODEL);

	// set the camera using a function similar to gluLookAt
	mu.lookAt(cams[activeCam]->getX(), cams[activeCam]->getY(), cams[activeCam]->getZ(),
			  cams[activeCam]->getTargetX(), cams[activeCam]->getTargetY(), cams[activeCam]->getTargetZ(),
			  cams[activeCam]->getUpX(), cams[activeCam]->getUpY(), cams[activeCam]->getUpZ());

	mu.loadIdentity(gmu::PROJECTION);

	if (cams[activeCam]->getProjectionType() == ProjectionType::Orthographic)
	{
		mu.ortho(-30.0f, 30.0f, -30.0f, 30.0f, -100.0f, 100.0f);
	}
	else
	{
		float ratio = (1.0f * WinX) / WinY;
		mu.perspective(53.13f, ratio, 0.1f, 1000.0f);
	}

	// setup the lights
	for (size_t i = 0; i < sceneLights.size(); i++)
	{
		// std::cout << "Loading " << sceneLights[i]->typeString() << " light.\n";
		sceneLights[i]->render(renderer, mu);
	}

	// Update and then render scene objects
	for (size_t i = 0; i < sceneObjects.size(); i++)
	{
		sceneObjects[i]->render(renderer, mu);
	}

	// Check collisions
	collisionSystem.checkCollisions();

	// Render debug information
	collisionSystem.showDebug(renderer, mu);

	// Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	// Each glyph quad texture needs just one byte color channel: 0 in background and 1 for the actual character pixels. Use it for alpha blending
	// text to be rendered in last place to be in front of everything
	if (fontLoaded)
	{
		glDisable(GL_DEPTH_TEST);

		std::vector<TextCommand> texts;
		float size = 0.5f;
		texts.push_back({.str = "X",
						 .position = {0.f, 0.f},
						 .size = size,
						 .color = {1.f, 0.f, 0.f, 1.f}});

		texts.push_back({.str = "Y",
						 .position = {30.f, 0.f},
						 .size = size,
						 .color = {0.f, 1.f, 0.f, 1.f}});

		texts.push_back({.str = "Z",
						 .position = {50.f, 0.f},
						 .size = size,
						 .color = {0.f, 0.f, 1.f, 1.f}});

		size = 0.4f;
		texts.push_back({.str = "X",
						 .position = {0.f, 80.f},
						 .size = size,
						 .color = {1.f, 0.f, 0.f, 1.f}});

		texts.push_back({.str = "Y",
						 .position = {30.f, 80.f},
						 .size = size,
						 .color = {0.f, 1.f, 0.f, 1.f}});

		texts.push_back({.str = "Z",
						 .position = {50.f, 80.f},
						 .size = size,
						 .color = {0.f, 0.f, 1.f, 1.f}});
		// the glyph contains transparent background colors and non-transparent for the actual character pixels. So we use the blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// viewer at origin looking down at  negative z direction
		int m_viewport[4];
		glGetIntegerv(GL_VIEWPORT, m_viewport);

		mu.loadIdentity(gmu::MODEL);
		mu.loadIdentity(gmu::VIEW);
		mu.pushMatrix(gmu::PROJECTION);
		mu.loadIdentity(gmu::PROJECTION);
		mu.ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
		mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL);

		for (auto text : texts)
		{
			text.pvm = mu.get(gmu::PROJ_VIEW_MODEL);
			renderer.renderText(text);
		}
		mu.popMatrix(gmu::PROJECTION);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	(void)xx;
	(void)yy;

	for (size_t i = 0; i < sceneObjects.size(); i++)
	{
		sceneObjects[i]->handleKeyInput(key);
	}

	switch (key)
	{

	case 27:
		glutLeaveMainLoop();
		break;

	case 'n': // toggle directional light
		for (auto light : sceneLights)
		{
			if (light->isType(lightType::DIRECTIONAL))
			{
				light->toggle();
			}
		}
		break;

	case 'c': // toggle point lights
		for (auto light : sceneLights)
		{
			if (light->isType(lightType::POINTLIGHT))
			{
				light->toggle();
			}
		}
		break;

	case 'h': // toggle spotlights
		for (auto light : sceneLights)
		{
			if (light->isType(lightType::SPOTLIGHT))
			{
				light->toggle();
			}
		}
		break;

	case '1':
		activeCam = 0;
		printf("Camera 1 activated\n");
		break;
	case '2':
		activeCam = 1;
		printf("Camera 2 activated\n");
		break;
	case '3':
		activeCam = 2;
		printf("Camera 3 activated\n");
		break;
	}
}

void processKeysUp(unsigned char key, int xx, int yy)
{
	for (size_t i = 0; i < sceneObjects.size(); i++)
	{
		sceneObjects[i]->handleKeyRelease(key);
	}
}

void processSpecialKeys(int key, int xx, int yy)
{
	(void)xx;
	(void)yy;

	for (size_t i = 0; i < sceneObjects.size(); i++)
	{
		sceneObjects[i]->handleSpecialKeyInput(key);
	}
}

void processSpecialKeysUp(int key, int xx, int yy)
{
	for (size_t i = 0; i < sceneObjects.size(); i++)
	{
		sceneObjects[i]->handleSpecialKeyRelease(key);
	}
}

// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN)
	{
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
	}

	// stop tracking the mouse
	else if (state == GLUT_UP)
	{
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{
	if (tracking == 0 || activeCam != 2)
		return;

	SphericalCoords sc = cams[activeCam]->getSpherical();

	int deltaX = -xx + startX;
	int deltaY = yy - startY;

	float alphaAux = sc.alpha + deltaX * mouseSensitivity;
	float betaAux = sc.beta + deltaY * mouseSensitivity;

	if (betaAux > 85.0f)
		betaAux = 85.0f;
	else if (betaAux < -85.0f)
		betaAux = -85.0f;

	cams[activeCam]->setSpherical(alphaAux, betaAux, sc.r);

	startX = xx;
	startY = yy;

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

void mouseWheel(int wheel, int direction, int x, int y)
{
	(void)x;
	(void)y;
	(void)wheel;

	if (activeCam != 2)
		return;
	SphericalCoords sc = cams[activeCam]->getSpherical();
	float rAux = sc.r + (direction * 0.1f);
	if (rAux < 0.1f)
		rAux = 0.1f;

	cams[activeCam]->setSpherical(sc.alpha, sc.beta, rAux);

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

//
// Scene building with basic geometry
//

void buildScene()
{
	// Texture Object definition
	renderer.TexObjArray.texture2D_Loader((std::string(RESOURCE_BASE) + "assets/stone.tga").c_str());
	renderer.TexObjArray.texture2D_Loader((std::string(RESOURCE_BASE) + "assets/checker.png").c_str());
	renderer.TexObjArray.texture2D_Loader((std::string(RESOURCE_BASE) + "assets/lightwood.tga").c_str());

	// Scene geometry with triangle meshes
	MyMesh amesh;

	float amb1[] = {1.f, 1.f, 1.f, 1.f};
	float diff1[] = {1.f, 1.f, 1.f, 1.f};
	float spec[] = {0.8f, 0.8f, 0.8f, 1.0f};
	float spec1[] = {0.3f, 0.3f, 0.3f, 1.0f};
	float black[] = {0.f, 0.f, 0.f, 1.f};
	float nonemissive[] = {0.0f, 0.0f, 0.0f, 1.0f};
	float shininess = 100.0f;
	int texcount = 0;

	// create geometry and VAO of the quad
	amesh = createQuad(1.0f, 1.0f);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, nonemissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	int quadID = renderer.addMesh(amesh);

	// create geometry and VAO of the sphere
	amesh = createSphere(0.1f, 20);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, nonemissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	int sphereID = renderer.addMesh(amesh);

	// create geometry and VAO of the cube
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, nonemissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	int cubeID = renderer.addMesh(amesh);

	// Load drone model from file
	std::vector<MyMesh> droneMeshs = createFromFile((std::string(RESOURCE_BASE) + "assets/drone.obj").c_str());
	std::vector<int> droneMeshIDs;
	for (size_t i = 0; i < droneMeshs.size(); i++)
	{
		// set material properties
		memcpy(droneMeshs[i].mat.ambient, amb1, 4 * sizeof(float));
		memcpy(droneMeshs[i].mat.diffuse, diff1, 4 * sizeof(float));
		memcpy(droneMeshs[i].mat.specular, spec1, 4 * sizeof(float));
		memcpy(droneMeshs[i].mat.emissive, nonemissive, 4 * sizeof(float));
		droneMeshs[i].mat.shininess = shininess;
		droneMeshs[i].mat.texCount = texcount;
		int meshID = renderer.addMesh(droneMeshs[i]);
		droneMeshIDs.push_back(meshID);
	}

	// Scene Lights
	float ambient = 0.01f;
	float diffuse = 0.5f;

	float whiteLight[4] = {1.f, 1.f, 1.f, 1.f};
	float fortyfive[4] = {-1.f, -1.f, -1.f, 0.f};
	Light *sun = new Light(whiteLight, 0.1f, diffuse, fortyfive);
	sceneLights.push_back(sun);

	float blueLight[4] = {0.f, 0.f, 1.f, 1.f};
	float bLightPos[4] = {4.f, 2.f, 2.f, 1.f};
	Light *bluePoint = new Light(blueLight, ambient, diffuse, bLightPos, 1.f, 0.1f, 0.01f);
	sceneLights.push_back(bluePoint);

	float redLight[4] = {1.f, 0.f, 0.f, 1.f};
	float rLightPos[4] = {0.f, 2.f, 2.f, 1.f};
	Light *redPoint = new Light(redLight, ambient, diffuse, rLightPos, 1.f, 0.1f, 0.01f);
	sceneLights.push_back(redPoint);

	float greenLight[4] = {0.f, 1.f, 0.f, 1.f};
	float yellowLight[4] = {1.f, 1.f, 0.f, 1.f};
	float yLightPos[4] = {2.5f, 4.f, 1.5f, 1.f};
	float yLightDir[4] = {0.f, -1.f, 0.f, 0.f};
	Light *yellowSpot = new Light(yellowLight, ambient, diffuse, yLightPos, yLightDir, 0.93f, 1.f, 0.1f, 0.01f);
	sceneLights.push_back(yellowSpot);

	float cyanLight[4] = {0.f, 1.f, 1.f, 1.f};
	float cLightPos[4] = {2.f, 0.2f, -2.f, 1.f};
	float cLightDir[4] = {0.f, 0.f, -1.f, 0.f};
	Light *cyanSpot = new Light(cyanLight, ambient, diffuse, cLightPos, cLightDir, 0.93f, 1.f, 0.1f, 0.01f);
	sceneLights.push_back(cyanSpot);

	// Scene objects
	// Floor
	SceneObject *floor = new SceneObject(std::vector<int>{quadID}, 2);
	floor->setRotation(0.0f, -90.0f, 0.0f);
	floor->setScale(30.0f, 30.0f, 1.0f);
	sceneObjects.push_back(floor);

	// Drone
	Drone *drone = new Drone(cams[2], droneMeshIDs, 1);
	drone->setPosition(5.0f, 5.0f, 1.0f);
	drone->setScale(0.05f, 0.05f, 0.05f);
	sceneObjects.push_back(drone);

	// Light markers
	amesh = createSphere(0.1f, 20);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blueLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	int blueLightID = renderer.addMesh(amesh);
	SceneObject *bluePointLight = new SceneObject({blueLightID}, 0);
	bluePointLight->setPosition(4.f, 2.f, 2.f);
	sceneObjects.push_back(bluePointLight);

	amesh = createSphere(0.1f, 20);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, redLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	int redLightID = renderer.addMesh(amesh);
	SceneObject *redPointLight = new SceneObject({redLightID}, 0);
	redPointLight->setPosition(0.f, 2.f, 2.f);
	sceneObjects.push_back(redPointLight);

	amesh = createCone(0.2f, 0.1f, 10);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, yellowLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	int yellowLightID = renderer.addMesh(amesh);
	SceneObject *yellowSpotLight = new SceneObject({yellowLightID}, 0);
	yellowSpotLight->setPosition(2.5f, 4.f, 1.5f);
	sceneObjects.push_back(yellowSpotLight);

	amesh = createCone(0.2f, 0.1f, 10); // createSphere(0.1f, 20);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, cyanLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	int cyanLightID = renderer.addMesh(amesh);
	SceneObject *cyanSpotLight = new SceneObject({cyanLightID}, 0);
	cyanSpotLight->setPosition(2.f, 0.2f, -2.f);
	cyanSpotLight->setRotation(0.f, 90.f, 0.f);
	sceneObjects.push_back(cyanSpotLight);

	// === ORIGIN MARKER ===
	amesh = createSphere(0.1f, 20);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, whiteLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	int whiteLightID = renderer.addMesh(amesh);
	SceneObject *whiteSpot = new SceneObject({whiteLightID}, 0);
	whiteSpot->setPosition(0.f, 0.f, 0.f);
	sceneObjects.push_back(whiteSpot);

	amesh = createCone(1.f, 0.05f, 4);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, redLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	int redXunitID = renderer.addMesh(amesh);
	SceneObject *redXunit = new SceneObject({redXunitID}, 0);
	redXunit->setPosition(0.f, 0.f, 0.f);
	redXunit->setRotation(90.f, 90.f, 0.f);
	sceneObjects.push_back(redXunit);

	amesh = createCone(1.f, 0.05f, 4);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, greenLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	int greenYunitID = renderer.addMesh(amesh);
	SceneObject *greenYunit = new SceneObject({greenYunitID}, 0);
	greenYunit->setPosition(0.f, 0.f, 0.f);
	greenYunit->setRotation(0.f, 0.f, 0.f);
	sceneObjects.push_back(greenYunit);

	amesh = createCone(1.f, 0.05f, 4);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blueLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	int blueZunitID = renderer.addMesh(amesh);
	SceneObject *blueZunit = new SceneObject({blueZunitID}, 0);
	blueZunit->setPosition(0.f, 0.f, 0.f);
	blueZunit->setRotation(0.f, 90.f, 0.f);
	sceneObjects.push_back(blueZunit);

	// Cameras
	// Create 3 cameras
	for (int i = 0; i < 3; i++)
		cams[i] = new Camera();

	// Top Orthogonal Camera
	cams[0]->setPosition(0.0f, 30.0f, 0.0f);
	cams[0]->setTarget(0.0f, 0.0f, 0.0f);
	cams[0]->setUp(0.0f, 0.0f, -1.0f);
	cams[0]->setProjectionType(ProjectionType::Orthographic);

	// Top Perspective Camera
	cams[1]->setPosition(0.0f, 30.0f, 0.0f);
	cams[1]->setTarget(0.0f, 0.0f, 0.0f);
	cams[1]->setUp(0.0f, 0.0f, -1.0f);
	cams[1]->setProjectionType(ProjectionType::Perspective);

	// Drone camera
	cams[2]->setPosition(5.0f, 5.0f, 0.0f);
	cams[2]->setTarget(2.0f, 2.0f, 0.0f);
	cams[2]->setUp(0.0f, 1.0f, 0.0f);
	cams[2]->setProjectionType(ProjectionType::Perspective);

	// Collision System
	collisionSystem.setDebugCubeMesh(cubeID);

	floor->getCollider()->setBox(-30.0f, -0.1f, -30.0f,
								 30.0f, 0.0f, 30.0f);
	collisionSystem.addCollider(drone->getCollider());
	collisionSystem.addCollider(floor->getCollider());

	// The truetypeInit creates a texture object in TexObjArray for storing the fontAtlasTexture
	fontLoaded = renderer.truetypeInit((std::string(RESOURCE_BASE) + "fonts/arial.ttf").c_str());
	if (!fontLoaded)
		std::cerr << "Fonts not loaded\n";
	else
		std::cerr << "Fonts loaded\n";

	printf("\nNumber of Texture Objects is %d\n\n", renderer.TexObjArray.getNumTextureObjects());
}

// ------------------------------------------------------------
//
// Main function
//

int main(int argc, char **argv)
{

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(4, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);

	//  Callback Registration
	glutDisplayFunc(renderSim);
	glutReshapeFunc(changeSize);

	// Update window title with FPS every second
	glutTimerFunc(0, timer, 0);
	glutIdleFunc(gameloop); // Use it for maximum performance
	// glutTimerFunc(0, refresh, 0); // use it to to get 60 FPS whatever

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutKeyboardUpFunc(processKeysUp);
	glutSpecialFunc(processSpecialKeys);
	glutSpecialUpFunc(processSpecialKeysUp);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);

	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	buildScene();

	if (!renderer.setRenderMeshesShaderProg(
			(std::string(RESOURCE_BASE) + "shaders/mesh.vert").c_str(),
			(std::string(RESOURCE_BASE) + "shaders/mesh.frag").c_str()) ||
		!renderer.setRenderTextShaderProg(
			(std::string(RESOURCE_BASE) + "shaders/ttf.vert").c_str(),
			(std::string(RESOURCE_BASE) + "shaders/ttf.frag").c_str()))
		return (1);

	//  GLUT main loop
	glutMainLoop();

	return (0);
}

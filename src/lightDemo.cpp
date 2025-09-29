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
#include "autoMover.cpp"

struct {
	int WindowHandle = 0;
	int WinX = 1024, WinY = 695;
	unsigned int FrameCount = 0;
	const char* WinTitle = "AVT Project 2025 G13";

	// Mouse Tracking Variables
	int startX, startY, tracking = 0;
	float mouseSensitivity = 0.3f;

	float lastTime = glutGet(GLUT_ELAPSED_TIME);
	bool fontLoaded = false;
} GLOBAL;

#define RESOURCE_BASE "resources/"
#define FONT_FOLDER   RESOURCE_BASE "fonts/"
#define ASSET_FOLDER  RESOURCE_BASE "assets/"
#define SHADER_FOLDER RESOURCE_BASE "shaders/"

struct {
	const char* Drone_OBJ = ASSET_FOLDER "drone.obj";
	const char* Stone_Tex = ASSET_FOLDER "stone.tga";
	const char* Checker_Tex = ASSET_FOLDER "checker.png";
	const char* Lightwood_Tex = ASSET_FOLDER "lightwood.tga";

	const char* Font_File = FONT_FOLDER "arial.ttf";

	const char* Mesh_Vert = SHADER_FOLDER "mesh.vert";
	const char* Mesh_Frag = SHADER_FOLDER "mesh.frag";
	const char* Font_Vert = SHADER_FOLDER "ttf.vert";
	const char* Font_Frag = SHADER_FOLDER "ttf.frag";
} FILEPATH;

gmu mu;
Renderer renderer;

std::vector<Light *> sceneLights;
std::vector<SceneObject *> sceneObjects;
CollisionSystem collisionSystem;

Camera *cams[3];
int activeCam = 0;


/// ::::::::::::::::::::::: CALLBACK FUNCTIONS ::::::::::::::::::::::: ///

void timer(int value)
{
	(void)value;

	std::ostringstream oss;
	oss << GLOBAL.WinTitle << ": " << GLOBAL.FrameCount << " FPS @ (" << GLOBAL.WinX << "x" << GLOBAL.WinY << ")";
	std::string s = oss.str();

	glutSetWindow(GLOBAL.WindowHandle);
	glutSetWindowTitle(s.c_str());
	GLOBAL.FrameCount = 0;

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
	float deltaTime = (currentTime - GLOBAL.lastTime) / 1000.0f; // Convert milliseconds to seconds
	GLOBAL.lastTime = currentTime;

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

	GLOBAL.WinX = w;
	GLOBAL.WinY = h;

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
	GLOBAL.FrameCount++;
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
		float ratio = (1.0f * GLOBAL.WinX) / GLOBAL.WinY;
		mu.perspective(53.13f, ratio, 0.1f, 1000.0f);
	}

	// setup the lights
	for (size_t i = 0; i < sceneLights.size(); i++)
	{
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
	if (GLOBAL.fontLoaded)
	{
		glDisable(GL_DEPTH_TEST);

		std::vector<TextCommand> texts;
		float size = 0.3f;
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
			if (light->isType(lightType::DIRECTIONAL))
				light->toggle();
		break;

	case 'c': // toggle point lights
		for (auto light : sceneLights)
			if (light->isType(lightType::POINTLIGHT))
				light->toggle();
		break;

	case 'h': // toggle spotlights
		for (auto light : sceneLights)
			if (light->isType(lightType::SPOTLIGHT))
				light->toggle();
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
	(void)xx;
	(void)yy;

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
	(void)xx;
	(void)yy;

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
	(void)xx;
	(void)yy;

	// start tracking the mouse
	if (state == GLUT_DOWN)
	{
		GLOBAL.startX = xx;
		GLOBAL.startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			GLOBAL.tracking = 1;
	}

	// stop tracking the mouse
	else if (state == GLUT_UP)
	{
		GLOBAL.tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{
	if (GLOBAL.tracking == 0 || activeCam != 2)
		return;

	SphericalCoords sc = cams[activeCam]->getSpherical();

	int deltaX = -xx + GLOBAL.startX;
	int deltaY = yy - GLOBAL.startY;

	float alphaAux = sc.alpha + deltaX * GLOBAL.mouseSensitivity;
	float betaAux = sc.beta + deltaY * GLOBAL.mouseSensitivity;

	if (betaAux > 85.0f)
		betaAux = 85.0f;
	else if (betaAux < -85.0f)
		betaAux = -85.0f;

	cams[activeCam]->setSpherical(alphaAux, betaAux, sc.r);

	GLOBAL.startX = xx;
	GLOBAL.startY = yy;

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

// Build a simple city with buildings primitives
void buildCity(MyMesh amesh, float amb1[], float diff1[], float spec1[], float nonemissive[],
	 		   float shininess, int texcount, int cubeID) {

	SceneObject *tower_1 = new SceneObject(std::vector<int>{cubeID}, 2);
	tower_1->setScale(2.0f, 10.0f, 2.0f);
	tower_1->setPosition(10.0f, 0.f, 5.f);
	sceneObjects.push_back(tower_1);

	SceneObject *tower_2 = new SceneObject(std::vector<int>{cubeID}, 2);
	tower_2->setScale(2.0f, 10.0f, 2.0f);
	tower_2->setPosition(6.0f, 0.f, 5.f);
	sceneObjects.push_back(tower_2);
	
	SceneObject *tower_rot_1 = new SceneObject(std::vector<int>{cubeID}, 2);
	tower_rot_1->setRotation(30.0f, 0.0f, 0.0f);
	tower_rot_1->setScale(2.0f, 6.0f, 2.0f);	
	tower_rot_1->setPosition(12.0f, 0.f, 13.f);
	sceneObjects.push_back(tower_rot_1);

	SceneObject *tower_rot_2 = new SceneObject(std::vector<int>{cubeID}, 2);
	tower_rot_2->setRotation(30.0f, 0.0f, 0.0f);
	tower_rot_2->setScale(2.0f, 10.0f, 2.0f);	
	tower_rot_2->setPosition(10.0f, 0.0f, 13.0f);
	sceneObjects.push_back(tower_rot_2);

	SceneObject *tower_rot_3 = new SceneObject(std::vector<int>{cubeID}, 2);
	tower_rot_3->setRotation(30.0f, 0.0f, 0.0f);
	tower_rot_3->setScale(2.0f, 10.0f, 2.0f);	
	tower_rot_3->setPosition(8.0f, 0.0f, 13.0f);
	sceneObjects.push_back(tower_rot_3);

	SceneObject *tower_rot_4 = new SceneObject(std::vector<int>{cubeID}, 2);
	tower_rot_4->setRotation(30.0f, 0.0f, 0.0f);
	tower_rot_4->setScale(2.0f, 6.0f, 2.0f);	
	tower_rot_4->setPosition(6.0f, 0.0f, 13.0f);
	sceneObjects.push_back(tower_rot_4);

	// create geometry and VAO of the cylinder
	amesh = createCylinder(1.0f, 1.0f, 20);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, nonemissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	renderer.myMeshes.push_back(amesh);
	int cylinderID = renderer.addMesh(amesh);


	SceneObject *cyl_tower_1 = new SceneObject(std::vector<int>{cylinderID}, 2);
	cyl_tower_1->setScale(1.4f, 8.0f, 1.4f);
	cyl_tower_1->setPosition(-5.0f, 4.f, 12.f);
	sceneObjects.push_back(cyl_tower_1);

	SceneObject *cyl_tower_2 = new SceneObject(std::vector<int>{cylinderID}, 2);
	cyl_tower_2->setScale(2.0f, 3.0f, 2.0f);	
	cyl_tower_2->setPosition(.0f, 1.5f, 12.f);
	sceneObjects.push_back(cyl_tower_2);

	SceneObject *cyl_tower_3 = new SceneObject(std::vector<int>{cylinderID}, 2);
	cyl_tower_3->setScale(1.0f, 12.0f, 1.0f);
	cyl_tower_3->setPosition(-3.0f, 6.0f, 3.f);
	sceneObjects.push_back(cyl_tower_3);

	SceneObject *cyl_tower_4 = new SceneObject(std::vector<int>{cylinderID}, 2);
	cyl_tower_4->setScale(2.0f, 10.0f, 1.5f);
	cyl_tower_4->setPosition(-12.0f, 5.0f, 10.f);
	sceneObjects.push_back(cyl_tower_4);

	SceneObject *cyl_tower_5 = new SceneObject(std::vector<int>{cylinderID}, 2);
	cyl_tower_5->setScale(2.0f, 10.0f, 1.5f);
	cyl_tower_5->setPosition(-12.0f, 5.0f, 6.5f);
	sceneObjects.push_back(cyl_tower_5);

	SceneObject *cyl_tower_6 = new SceneObject(std::vector<int>{cylinderID}, 2);
	cyl_tower_6->setScale(2.0f, 10.0f, 1.5f);
	cyl_tower_6->setPosition(-12.0f, 5.0f, 3.4f);
	sceneObjects.push_back(cyl_tower_6);

	amesh = createTorus(0.5f, 2.0f, 20, 20);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, nonemissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	renderer.myMeshes.push_back(amesh);
	int torusID = renderer.addMesh(amesh);


	SceneObject *torus = new SceneObject(std::vector<int>{torusID}, 2);
	torus->setScale(3.0f, 2.0f, 3.0f);
	torus->setPosition(-7.5f, 1.50f, -7.5f);
	sceneObjects.push_back(torus);

	amesh = createCone(1.0f, 1.0f, 5);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, nonemissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	renderer.myMeshes.push_back(amesh);
	int coneID = renderer.addMesh(amesh);

	SceneObject *piramid_1 = new SceneObject(std::vector<int>{coneID}, 2);
	piramid_1->setScale(2.5f, 5.0f, 2.5f);
	piramid_1->setPosition(7.5f, 0.0f, -11.25f);
	sceneObjects.push_back(piramid_1);

	SceneObject *piramid_2 = new SceneObject(std::vector<int>{coneID}, 2);
	piramid_2->setScale(2.5f, 5.0f, 2.5f);
	piramid_2->setPosition(11.25f, 0.0f, -7.5f);
	sceneObjects.push_back(piramid_2);

	SceneObject *piramid_3 = new SceneObject(std::vector<int>{coneID}, 2);
	piramid_3->setScale(2.5f, 5.0f, 2.5f);
	piramid_3->setPosition(7.5f, 0.0f, -3.75f);
	sceneObjects.push_back(piramid_3);

	SceneObject *piramid_4 = new SceneObject(std::vector<int>{coneID}, 2);
	piramid_4->setScale(2.5f, 5.0f, 2.5f);
	piramid_4->setPosition(3.75f, 0.0f, -7.5f);
	sceneObjects.push_back(piramid_4);

	// --------------------------------------------------------------------
	// Collider registration for city buildings (AABB approximations)
	// Ignore rotation of buildings for simplicity

	// Lambda function to reduce code repetition
	auto addBox = [&](SceneObject *obj,
					float minX, float minY, float minZ,
					float maxX, float maxY, float maxZ) {
		obj->getCollider()->setBox(minX, minY, minZ, maxX, maxY, maxZ);
		collisionSystem.addCollider(obj->getCollider());
	};

	// Cube based buildings (scale.x/z span full width, centered at position)
	addBox(tower_1, 10.0f , 0.0f, 5.0f , 10.0f + 2.0f, 10.0f, 5.0f + 2.0f);
	addBox(tower_2, 6.0f , 0.0f, 5.0f , 6.0f + 2.0f, 10.0f, 5.0f + 2.0f);
	addBox(tower_rot_1, 12.0f , 0.0f, 13.0f , 12.0f + 2.0f, 6.0f, 13.0f + 2.0f);
	addBox(tower_rot_2, 10.0f , 0.0f, 13.0f , 10.0f + 2.0f, 10.0f, 13.0f + 2.0f);
	addBox(tower_rot_3, 8.0f , 0.0f, 13.0f , 8.0f + 2.0f, 10.0f, 13.0f + 2.0f);
	addBox(tower_rot_4, 6.0f , 0.0f, 13.0f , 6.0f + 2.0f, 6.0f, 13.0f + 2.0f);

	// Cylinders (approximated as boxes)
	addBox(cyl_tower_1, -5.0f - 0.7f, 0.0f, 12.0f - 0.7f, -5.0f + 0.7f, 8.0f, 12.0f + 0.7f);
	addBox(cyl_tower_2, 0.0f - 1.0f, 0.0f, 12.0f - 1.0f, 0.0f + 1.0f, 3.0f, 12.0f + 1.0f);
	addBox(cyl_tower_3, -3.0f - 0.5f, 0.0f, 3.0f - 0.5f, -3.0f + 0.5f, 12.0f, 3.0f + 0.5f);
	addBox(cyl_tower_4, -12.0f - 1.0f, 0.0f, 10.0f - 0.75f, -12.0f + 1.0f, 10.0f, 10.0f + 0.75f);
	addBox(cyl_tower_5, -12.0f - 1.0f, 0.0f, 6.5f - 0.75f, -12.0f + 1.0f, 10.0f, 6.5f + 0.75f);
	addBox(cyl_tower_6, -12.0f - 1.0f, 0.0f, 3.4f - 0.75f, -12.0f + 1.0f, 10.0f, 3.4f + 0.75f);

	// Torus (broad bounding box; torus center elevated at y=1.5 with scale.y = 2)
	addBox(torus, -7.5f - 1.5f, 0.5f, -7.5f - 1.5f,-7.5f + 1.5f, 2.5f, -7.5f + 1.5f);

	// Cones (centered, base on ground)
	addBox(piramid_1, 7.5f - 1.25f, 0.0f, -11.25f - 1.25f, 7.5f + 1.25f, 5.0f, -11.25f + 1.25f);
	addBox(piramid_2, 11.25f - 1.25f, 0.0f, -7.5f - 1.25f, 11.25f + 1.25f, 5.0f, -7.5f + 1.25f);
	addBox(piramid_3, 7.5f - 1.25f, 0.0f, -3.75f - 1.25f, 7.5f + 1.25f, 5.0f, -3.75f + 1.25f);
	addBox(piramid_4, 3.75f - 1.25f, 0.0f, -7.5f - 1.25f, 3.75f + 1.25f, 5.0f, -7.5f + 1.25f);

}

// Create the scene objects, cameras and lights
void buildScene()
{
	// Texture Object definition
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Stone_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Checker_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Lightwood_Tex);

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
	cams[2]->setPosition(0.0f, 10.0f, 10.0f);
	cams[2]->setUp(0.0f, 1.0f, 0.0f);
	cams[2]->setProjectionType(ProjectionType::Perspective);

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
	std::vector<MyMesh> droneMeshs = createFromFile(FILEPATH.Drone_OBJ);
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
	drone->setScale(2.f, 2.f, 2.f);
	// drone->setScale(0.05f, 0.05f, 0.05f);
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

	amesh = createCone(0.2f, 0.1f, 10);
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

	// Collision System
	collisionSystem.setDebugCubeMesh(cubeID);

	floor->getCollider()->setBox(-30.0f, -0.1f, -30.0f,
								 30.0f, 0.0f, 30.0f);
	collisionSystem.addCollider(drone->getCollider());
	collisionSystem.addCollider(floor->getCollider());
	
	buildCity(amesh, amb1, diff1, spec1, nonemissive, shininess, texcount, cubeID);

	// Moving object
	// Texture not working	
	MyMesh moverMesh = createSphere(2.0f, 16);
	memcpy(moverMesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(moverMesh.mat.diffuse, amb1, 4 * sizeof(float));
	memcpy(moverMesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(moverMesh.mat.emissive, nonemissive, 4 * sizeof(float));
	moverMesh.mat.shininess = 50.0f;
	moverMesh.mat.texCount = 0;
	int moverID = renderer.addMesh(moverMesh);



	AutoMover *mover = new AutoMover({cubeID}, 2, 20.0f, 4.0f);
	mover->setPosition(0.f, 5.0f, 0.f);
	mover->setScale(1.0f, 1.0f, 1.0f);
	sceneObjects.push_back(mover);

	AutoMover *mover2 = new AutoMover({cubeID}, 2, 20.0f, 6.0f);
	mover2->setPosition(0.f, 5.0f, 0.f);
	mover2->setScale(1.0f, 1.0f, 1.0f);
	sceneObjects.push_back(mover2);
	//collisionSystem.addCollider(mover->getCollider());


	// The truetypeInit creates a texture object in TexObjArray for storing the fontAtlasTexture
	GLOBAL.fontLoaded = renderer.truetypeInit(FILEPATH.Font_File);
	if (!GLOBAL.fontLoaded)
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
	glutInitWindowSize(GLOBAL.WinX, GLOBAL.WinY);
	GLOBAL.WindowHandle = glutCreateWindow(GLOBAL.WinTitle);

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

	if (!renderer.setRenderMeshesShaderProg(FILEPATH.Mesh_Vert, FILEPATH.Mesh_Frag) ||
		!renderer.setRenderTextShaderProg(FILEPATH.Font_Vert, FILEPATH.Font_Frag))
		return (1);

	//  GLUT main loop
	glutMainLoop();

	return (0);
}

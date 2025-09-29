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

#define RESOURCE_BASE "resources/"
#define FONT_FOLDER   RESOURCE_BASE "fonts/"
#define ASSET_FOLDER  RESOURCE_BASE "assets/"
#define SHADER_FOLDER RESOURCE_BASE "shaders/"

struct {
	const char* Drone_OBJ = ASSET_FOLDER "drone.obj";
	const char* Grass_OBJ = ASSET_FOLDER "grass_billboard.obj";

	const char* Stone_Tex = ASSET_FOLDER "stone.tga";
	const char* Floor_Tex = ASSET_FOLDER "floor_grass.png";
	const char* Window_Tex = ASSET_FOLDER "window.png";
	const char* BBGrass_Tex = ASSET_FOLDER "billboard_grass.png";
	const char* Lightwood_Tex = ASSET_FOLDER "lightwood.tga";

	const char* Skybox_Cubemap_Day[6] = {
		ASSET_FOLDER "skybox/day_right.png", ASSET_FOLDER "skybox/day_left.png",
		ASSET_FOLDER "skybox/day_top.png", ASSET_FOLDER "skybox/day_bottom.png",
		ASSET_FOLDER "skybox/day_front.png", ASSET_FOLDER "skybox/day_back.png"
	};

	const char* Skybox_Cubemap_Night[6] = {
		ASSET_FOLDER "skybox/night_right.png", ASSET_FOLDER "skybox/night_left.png",
		ASSET_FOLDER "skybox/night_top.png", ASSET_FOLDER "skybox/night_bottom.png",
		ASSET_FOLDER "skybox/night_front.png", ASSET_FOLDER "skybox/night_back.png"
	};

	const char* Font_File = FONT_FOLDER "arial.ttf";

	const char* Mesh_Vert = SHADER_FOLDER "mesh.vert";
	const char* Mesh_Frag = SHADER_FOLDER "mesh.frag";
	const char* Font_Vert = SHADER_FOLDER "ttf.vert";
	const char* Font_Frag = SHADER_FOLDER "ttf.frag";
	const char* Post_Vert = SHADER_FOLDER "skybox.vert";
	const char* Post_Frag = SHADER_FOLDER "skybox.frag";
} FILEPATH;

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
	bool daytime = true;
	bool showFog = true;
	bool showDebug = true;

	unsigned int cubemap_dayID = 0;
	unsigned int cubemap_nightID = 0;
} GLOBAL;

gmu mu;
Renderer renderer;

std::vector<Light> sceneLights;
std::vector<SceneObject*> sceneObjects;
std::vector<SceneObject*> transparentObjects;
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
	glutTimerFunc(1000 / 100, refresh, 0);
}

void gameloop(void)
{
	float currentTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (currentTime - GLOBAL.lastTime) / 1000.0f; // Convert milliseconds to seconds
	GLOBAL.lastTime = currentTime;

	// Update all scene objects with the elapsed time
	for (auto obj: sceneObjects) obj->update(deltaTime);

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
	renderer.setTexUnit(0, 0); // Stone
	renderer.setTexUnit(1, 1); // Floor grass
	renderer.setTexUnit(2, 2); // Window
	renderer.setTexUnit(3, 3); // Billboard grass
	renderer.setTexUnit(4, 4); // Lightwood

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

	float fogColor[] = { 0.f, 0.f, 0.f, 0.f };
	if (GLOBAL.showFog) {
		float lightgray[] = { .75f, 0.85f, 0.75f, 1.f };
		float darkgray[]  = { 0.15f, 0.15f, 0.15f, 1.f };
		if (GLOBAL.daytime) {
			for (int i = 0; i < 4; i++) fogColor[i] = lightgray[i];
		} else {
			for (int i = 0; i < 4; i++) fogColor[i] = darkgray[i];
		}
	}
	renderer.setFogColor(fogColor);

	/*  RENDER QUEUE	
	  1) setup the lights
	  2) render opaque objects
	  3) render skybox
	  4) render transparent objects
	*/

	for (auto& light: sceneLights) light.render(renderer, mu);
	for (auto obj: sceneObjects) obj->render(renderer, mu);

	// Render skybox
	mu.pushMatrix(gmu::MODEL);
	mu.translate(gmu::MODEL, cams[activeCam]->getX(), cams[activeCam]->getY(), cams[activeCam]->getZ());
	mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL);
	float* m_VP = mu.get(gmu::PROJ_VIEW_MODEL);
	if (GLOBAL.daytime) {
		unsigned int id = renderer.TexObjArray.getTextureId(GLOBAL.cubemap_dayID);
		renderer.activateSkyboxShaderProg(m_VP, id, fogColor);
	} else {
		unsigned int id = renderer.TexObjArray.getTextureId(GLOBAL.cubemap_nightID);
		renderer.activateSkyboxShaderProg(m_VP, id, fogColor);
	}
	mu.popMatrix(gmu::MODEL);
	renderer.activateRenderMeshesShaderProg();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (auto obj: transparentObjects) obj->render(renderer, mu);
	glDisable(GL_BLEND);

	// Check collisions
	collisionSystem.checkCollisions();

	// Render debug information
	if (GLOBAL.showDebug)
		collisionSystem.showDebug(renderer, mu);

	// Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	// Each glyph quad texture needs just one byte color channel: 0 in background and 1 for the actual character pixels. Use it for alpha blending
	// text to be rendered in last place to be in front of everything
	if (GLOBAL.fontLoaded)
	{
		glDisable(GL_DEPTH_TEST);

		std::vector<TextCommand> texts;
		float size = 0.3f;
		texts.push_back(TextCommand{"X", { 0.f, 0.f}, size, {1.f, 0.f, 0.f, 1.f}});
		texts.push_back(TextCommand{"Y", {20.f, 0.f}, size, {0.f, 1.f, 0.f, 1.f}});
		texts.push_back(TextCommand{"Z", {35.f, 0.f}, size, {0.f, 0.f, 1.f, 1.f}});

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
		GLOBAL.daytime = !GLOBAL.daytime;
		for (auto& light : sceneLights)
			if (light.isType(LightType::DIRECTIONAL))
				light.toggleLight();
		break;

	case 'c': // toggle point lights
		for (auto& light : sceneLights)
			if (light.isType(LightType::POINTLIGHT))
				light.toggleLight();
		break;

	case 'h': // toggle spotlights
		for (auto& light : sceneLights)
			if (light.isType(LightType::SPOTLIGHT))
				light.toggleLight();
		break;

	case 'k': // toggle debug
		GLOBAL.showDebug = !GLOBAL.showDebug;
		for (auto& light : sceneLights)
			light.toggleObj();
			//if (light.isDebug())
		break;

	case 'f': // toggle fog
		GLOBAL.showFog = !GLOBAL.showFog;
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

void buildScene()
{
	// Texture Object definition
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Stone_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Floor_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Window_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.BBGrass_Tex, false);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Lightwood_Tex);
	GLOBAL.cubemap_dayID = renderer.TexObjArray.getNumTextureObjects();
	renderer.TexObjArray.textureCubeMap_Loader(FILEPATH.Skybox_Cubemap_Day);
	GLOBAL.cubemap_nightID = renderer.TexObjArray.getNumTextureObjects();
	renderer.TexObjArray.textureCubeMap_Loader(FILEPATH.Skybox_Cubemap_Night);

	// Top Orthogonal Camera
	cams[0] = new Camera();
	cams[0]->setPosition(0.0f, 30.0f, 0.0f);
	cams[0]->setTarget(0.0f, 0.0f, 0.0f);
	cams[0]->setUp(0.0f, 0.0f, -1.0f);
	cams[0]->setProjectionType(ProjectionType::Orthographic);

	// Top Perspective Camera
	cams[1] = new Camera();
	cams[1]->setPosition(0.0f, 30.0f, 0.0f);
	cams[1]->setTarget(0.0f, 0.0f, 0.0f);
	cams[1]->setUp(0.0f, 0.0f, -1.0f);
	cams[1]->setProjectionType(ProjectionType::Perspective);

	// Drone camera
	cams[2] = new Camera();
	cams[2]->setPosition(0.0f, 10.0f, 10.0f);
	cams[2]->setUp(0.0f, 1.0f, 0.0f);
	cams[2]->setProjectionType(ProjectionType::Perspective);

	// Scene geometry with triangle meshes
	MyMesh amesh;

	float amb1[] = {1.f, 1.f, 1.f, 1.f};
	float diff1[] = {1.f, 1.f, 1.f, 1.f};
	float spec[] = {0.8f, 0.8f, 0.8f, 1.0f};
	float spec1[] = {0.3f, 0.3f, 0.3f, 1.0f};
	float nonemissive[] = {0.0f, 0.0f, 0.0f, 1.0f};
	float shininess = 100.0f;
	int texcount = 0;

	// create geometry and VAO of the floor quad
	amesh = createQuad(1.0f, 1.0f);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, nonemissive, 4 * sizeof(float));
	amesh.mat.shininess = 1.f;
	amesh.mat.texCount = texcount;
	int quadID = renderer.addMesh(amesh);

	// create geometry and VAO of the cube
	amesh = createCube();
	int cubeID = renderer.addMesh(amesh);

	SceneObject* window = new SceneObject(std::vector<int>{cubeID}, TexMode::TEXTURE_WINDOW);
	window->setPosition(0.f, 5.f, 10.f);
	window->setScale(10.f, 5.f, 2.f);
	window->getCollider()->setBox(0.f, 5.f, 10.f, 10.0f, 10.0f, 12.0f);
	transparentObjects.push_back(window);
	collisionSystem.addCollider(window->getCollider());

	// Load drone model from file
	std::vector<MyMesh> grassMesh = createFromFile(FILEPATH.Grass_OBJ);
	std::vector<int> grassMeshIDs;
	for (size_t i = 0; i < grassMesh.size(); i++)
	{
		float amb[] = {10.f, 10.f, 10.f, 1.f};
		float blk[] = {0.f, 0.f, 0.f, 1.f};
		// set material properties
		memcpy(grassMesh[i].mat.ambient, amb, 4 * sizeof(float));
		memcpy(grassMesh[i].mat.diffuse, blk, 4 * sizeof(float));
		memcpy(grassMesh[i].mat.specular, blk, 4 * sizeof(float));
		memcpy(grassMesh[i].mat.emissive, blk, 4 * sizeof(float));
		int meshID = renderer.addMesh(grassMesh[i]);
		grassMeshIDs.push_back(meshID);
	}

	const int grassCount = 100;
	for (int i = 1; i < grassCount; i++) {
		SceneObject* grass = new SceneObject(grassMeshIDs, TexMode::TEXTURE_BBGRASS);
		const float goldRatio = PI_F * (3 - std::sqrt(5));
		const float radius = std::sqrt(i / (float)grassCount) * 10.f;
		const float angle = i * goldRatio;

		grass->setPosition(std::cos(angle) * radius, 0.f, 30 + std::sin(angle) * radius);
		grass->setScale(2.f, 1.5f, 2.f);
		grass->setRotation(10.f * i, 0.0f, 0.0f);
		sceneObjects.push_back(grass);
	}

	amesh = createTorus(1.f, 10.f, 20, 20);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, nonemissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess * 2;
	int torusID = renderer.addMesh(amesh);
	for (int i = 1; i < 10; i++) {
		SceneObject* torus = new SceneObject(std::vector<int>{torusID}, TexMode::TEXTURE_STONE);
		torus->setPosition(50.0f * i, 0.0f, -1.f * 2.0f * i * i);
		torus->setRotation(10.f * i, 0.0f, 0.0f);
		sceneObjects.push_back(torus);
	}

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

	// Scene objects
	// Floor
	SceneObject *floor = new SceneObject(std::vector<int>{quadID}, TexMode::TEXTURE_FLOOR);
	floor->setRotation(0.0f, -90.0f, 0.0f);
	floor->setScale(1000.0f, 1000.0f, 1.0f);
	sceneObjects.push_back(floor);

	// Drone
	Drone *drone = new Drone(cams[2], droneMeshIDs, TexMode::TEXTURE_LIGHTWOOD);
	drone->setPosition(5.0f, 5.0f, 1.0f);
	drone->setScale(2.f, 2.f, 2.f);
	// drone->setScale(0.05f, 0.05f, 0.05f);
	sceneObjects.push_back(drone);

	
	// === SCENE LIGHTS === //
	float whiteLight[4] = {1.f, 1.f, 1.f, 1.f};
	float sunDirection[4] = {-1.f, -1.f, 0.001f, 0.f};
	sceneLights.emplace_back(LightType::DIRECTIONAL, whiteLight)
		.setDirection(sunDirection);

	float blueLight[4] = {0.f, 0.f, 1.f, 1.f};
	float bLightPos[4] = {4.f, 2.f, 2.f, 1.f};
	sceneLights.emplace_back(LightType::POINTLIGHT, blueLight)
		.setPosition(bLightPos).createObject(renderer, sceneObjects);

	float redLight[4] = {1.f, 0.f, 0.f, 1.f};
	float rLightPos[4] = {0.f, 2.f, 2.f, 1.f};
	sceneLights.emplace_back(LightType::POINTLIGHT, redLight)
		.setPosition(rLightPos).createObject(renderer, sceneObjects);

	float yellowLight[4] = {1.f, 1.f, 0.f, 1.f};
	float yLightPos[4] = {2.5f, 4.f, 1.5f, 1.f};
	float yLightDir[4] = {0.f, -1.f, 0.f, 0.f};
	sceneLights.emplace_back(LightType::SPOTLIGHT, yellowLight)
		.setPosition(yLightPos).setDirection(yLightDir)
		.createObject(renderer, sceneObjects);

	float cyanLight[4] = {0.f, 1.f, 1.f, 1.f};
	float cLightPos[4] = {1.f, 25.f, 0.f, 1.f};
	float cLightDir[4] = {1.f, 0.f, 0.f, 0.f};
	sceneLights.emplace_back(LightType::SPOTLIGHT, cyanLight)
		.setPosition(cLightPos).setDirection(cLightDir)
		.setDiffuse(2.f).setAttenuation(1.f, 0.f, 0.0005f)
		.createObject(renderer, sceneObjects);

	// === ORIGIN MARKER === //
	float origin[] = { 0.f, 0.f, 0.f, 1.f };
	sceneLights.emplace_back(LightType::POINTLIGHT, whiteLight).setDebug()
		.setPosition(origin).setAmbient(0.f).setDiffuse(0.f)
		.createObject(renderer, sceneObjects);

	float axisXdir[] = { -1.f, 0.f, 0.f, 0.f };
	sceneLights.emplace_back(LightType::SPOTLIGHT, redLight).setDebug()
		.setPosition(origin).setDirection(axisXdir).setAmbient(0.f).setDiffuse(0.f)
		.createObject(renderer, sceneObjects);

	float greenLight[] = {0.f, 1.f, 0.f, 1.f};
	float axisYdir[] = { 0.f, -1.f, 0.f, 0.f };
	sceneLights.emplace_back(LightType::SPOTLIGHT, greenLight).setDebug()
		.setPosition(origin).setDirection(axisYdir).setAmbient(0.f).setDiffuse(0.f)
		.createObject(renderer, sceneObjects);

	float axisZdir[] = { 0.f, 0.f, -1.f, 0.f };
	sceneLights.emplace_back(LightType::SPOTLIGHT, blueLight).setDebug()
		.setPosition(origin).setDirection(axisZdir).setAmbient(0.f).setDiffuse(0.f)
		.createObject(renderer, sceneObjects);

	// Collision System
	collisionSystem.setDebugCubeMesh(cubeID);

	floor->getCollider()->setBox(-30.0f, -0.1f, -30.0f,
								 30.0f, 0.0f, 30.0f);
	collisionSystem.addCollider(drone->getCollider());
	collisionSystem.addCollider(floor->getCollider());

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
		!renderer.setRenderTextShaderProg(FILEPATH.Font_Vert, FILEPATH.Font_Frag) ||
		!renderer.setSkyboxShaderProg(FILEPATH.Post_Vert, FILEPATH.Post_Frag))
		return (1);

	//  GLUT main loop
	glutMainLoop();

	return (0);
}

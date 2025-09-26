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
#include "camera.h"
#include "sceneObject.h"
#include "drone.h"
#include "light.h"

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

// Cameras
Camera *cams[3];
int activeCam = 0;

// Mouse Tracking Variables
int startX, startY, tracking = 0;
float mouseSensitivity = 0.3f;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
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
	glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{
	(void)value;

	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
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
		sceneObjects[i]->update();
		sceneObjects[i]->render(renderer, mu);
	}

	// Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	// Each glyph quad texture needs just one byte color channel: 0 in background and 1 for the actual character pixels. Use it for alpha blending
	// text to be rendered in last place to be in front of everything
	if (fontLoaded)
	{
		glDisable(GL_DEPTH_TEST);

		std::vector<TextCommand> texts;
		texts.push_back({
			.str = "X",
			.position = { 0.f, 0.f },
			.size = 0.5f,
			.color = { 1.f, 0.f, 0.f, 1.f }
		});
		
		texts.push_back({
			.str = "Y",
			.position = { 30.f, 0.f },
			.size = 0.5f,
			.color = { 0.f, 1.f, 0.f, 1.f }
		});
		
		texts.push_back({
			.str = "Z",
			.position = { 50.f, 0.f },
			.size = 0.5f,
			.color = { 0.f, 0.f, 1.f, 1.f }
		});
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

		for (auto text : texts) {
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
		sceneObjects[i]->handleInput(key);
	}

	switch (key)
	{

	case 27:
		glutLeaveMainLoop();
		break;

	case 'c':
		printf("Camera info:\n%s\n", cams[activeCam]->toString().c_str());
		break;

	case 'l': // toggle spotlight mode
		break;

	/*
	case 'r': // reset
		alpha = 57.0f;
		betaAngle = 18.0f; // Camera Spherical Coordinates
		r = 45.0f;
		camX = r * sin(alpha * 3.14f / 180.0f) * cos(betaAngle * 3.14f / 180.0f);
		camZ = r * cos(alpha * 3.14f / 180.0f) * cos(betaAngle * 3.14f / 180.0f);
		camY = r * sin(betaAngle * 3.14f / 180.0f);
		break;
	*/

	case 'm':
		glEnable(GL_MULTISAMPLE);
		break;
	case 'n':
		glDisable(GL_MULTISAMPLE);
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

void processSpecialKeys(int key, int xx, int yy)
{
	(void)xx;
	(void)yy;

	for (size_t i = 0; i < sceneObjects.size(); i++)
	{
		sceneObjects[i]->handleSpecialInput(key);
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
	renderer.TexObjArray.texture2D_Loader("resources/assets/stone.tga");
	renderer.TexObjArray.texture2D_Loader("resources/assets/checker.png");
	renderer.TexObjArray.texture2D_Loader("resources/assets/lightwood.tga");

	// Scene geometry with triangle meshes

	MyMesh amesh;

	float amb1[] = {1.f, 1.f, 1.f, 1.f};
	float diff1[] = {1.f, 1.f, 1.f, 1.f};
	float spec[] = {0.8f, 0.8f, 0.8f, 1.0f};
	float spec1[] = {0.3f, 0.3f, 0.3f, 1.0f};
	float black[] = { 0.f, 0.f, 0.f, 1.f };
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
	renderer.myMeshes.push_back(amesh);

	// create geometry and VAO of the cube
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, nonemissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	renderer.myMeshes.push_back(amesh);

	// The truetypeInit creates a texture object in TexObjArray for storing the fontAtlasTexture
	fontLoaded = renderer.truetypeInit("resources/fonts/arial.ttf");
	if (!fontLoaded)
		std::cerr << "Fonts not loaded\n";
	else
		std::cerr << "Fonts loaded\n";

	printf("\nNumber of Texture Objects is %d\n\n", renderer.TexObjArray.getNumTextureObjects());

	float ambient = 0.01f;
	float diffuse = 0.5f;

	// scene lights
	float whiteLight[4] = { 1.f, 1.f, 1.f, 1.f };
	float fortyfive[4] = { -1.f, -1.f, -1.f, 0.f };
	Light *sun = new Light(whiteLight, 0.1f, diffuse, fortyfive);
	sceneLights.push_back(sun);

	float blueLight[4] = { 0.f, 0.f, 1.f, 1.f };
	float bLightPos[4] = { 4.f, 2.f, 2.f, 1.f };
	Light *bluePoint = new Light(blueLight, ambient, diffuse, bLightPos, 1.f, 0.1f, 0.01f);
	sceneLights.push_back(bluePoint);

	float redLight[4] = { 1.f, 0.f, 0.f, 1.f };
	float rLightPos[4] = { 0.f, 2.f, 2.f, 1.f };
	Light *redPoint = new Light(redLight, ambient, diffuse, rLightPos, 1.f, 0.1f, 0.01f);
	sceneLights.push_back(redPoint);

	float greenLight[4] = { 0.f, 1.f, 0.f, 1.f };
	float yellowLight[4] = { 1.f, 1.f, 0.f, 1.f };
	float yLightPos[4] = { 2.5f, 4.f, 1.5f, 1.f };
	float yLightDir[4] = { 0.f, -1.f, 0.f, 0.f };
	Light *yellowSpot = new Light(yellowLight, ambient, diffuse, yLightPos, yLightDir, 0.93f, 1.f, 0.1f, 0.01f);
	sceneLights.push_back(yellowSpot);

	// Scene objects
	// Floor, meshID=0 (quad), texMode=1 (lighwood.tga)
	SceneObject *floor = new SceneObject(0, 1);
	floor->setRotation(0.0f, -90.0f, 0.0f);
	floor->setScale(30.0f, 30.0f, 1.0f);
	sceneObjects.push_back(floor);

	// Drone, meshID=1 (cube), texMode=2 (stone.tga)
	Drone *drone = new Drone(cams[2], 1, 2);
	drone->setPosition(2.0f, 2.0f, 1.0f);
	sceneObjects.push_back(drone);

	amesh = createSphere(0.1f, 20);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blueLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	renderer.myMeshes.push_back(amesh);
	SceneObject *bluePointLight = new SceneObject(renderer.myMeshes.size()-1, 0);
	bluePointLight->setPosition(4.f, 2.f, 2.f);
	sceneObjects.push_back(bluePointLight);

	amesh = createSphere(0.1f, 20);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, redLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	renderer.myMeshes.push_back(amesh);
	SceneObject *redPointLight = new SceneObject(renderer.myMeshes.size()-1, 0);
	redPointLight->setPosition(0.f, 2.f, 2.f);
	sceneObjects.push_back(redPointLight);

	amesh = createCone(0.2f, 0.1f, 10);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, yellowLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	renderer.myMeshes.push_back(amesh);
	SceneObject *yellowSpotLight = new SceneObject(renderer.myMeshes.size()-1, 0);
	yellowSpotLight->setPosition(2.5f, 4.f, 1.5f);
	sceneObjects.push_back(yellowSpotLight);
	

	float cyanLight[4] = { 0.f, 1.f, 1.f, 1.f };
	float cLightPos[4] = { 2.f, 0.2f, -2.f, 1.f };
	float cLightDir[4] = { 0.f, 0.f, -1.f, 0.f };
	Light *cyanSpot = new Light(cyanLight, ambient, diffuse, cLightPos, cLightDir, 0.93f, 1.f, 0.05f, 0.01f);
	sceneLights.push_back(cyanSpot);
	amesh = createCone(0.2f, 0.1f, 10);//createSphere(0.1f, 20);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, cyanLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	renderer.myMeshes.push_back(amesh);
	SceneObject *cyanSpotLight = new SceneObject(renderer.myMeshes.size()-1, 0);
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
	renderer.myMeshes.push_back(amesh);
	SceneObject *whiteSpot = new SceneObject(renderer.myMeshes.size()-1, 0);
	whiteSpot->setPosition(0.f, 0.f, 0.f);
	sceneObjects.push_back(whiteSpot);

	amesh = createCone(1.f, 0.05f, 4);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, redLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	renderer.myMeshes.push_back(amesh);
	SceneObject *redXunit = new SceneObject(renderer.myMeshes.size()-1, 0);
	redXunit->setPosition(0.f, 0.f, 0.f);
	redXunit->setRotation(90.f, 90.f, 0.f);
	sceneObjects.push_back(redXunit);
	
	amesh = createCone(1.f, 0.05f, 4);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, greenLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	renderer.myMeshes.push_back(amesh);
	SceneObject *greenYunit = new SceneObject(renderer.myMeshes.size()-1, 0);
	greenYunit->setPosition(0.f, 0.f, 0.f);
	greenYunit->setRotation(0.f, 0.f, 0.f);
	sceneObjects.push_back(greenYunit);

	amesh = createCone(1.f, 0.05f, 4);
	memcpy(amesh.mat.ambient, black, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, black, 4 * sizeof(float));
	memcpy(amesh.mat.specular, black, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blueLight, 4 * sizeof(float));
	amesh.mat.shininess = 0.f;
	renderer.myMeshes.push_back(amesh);
	SceneObject *blueZunit = new SceneObject(renderer.myMeshes.size()-1, 0);
	blueZunit->setPosition(0.f, 0.f, 0.f);
	blueZunit->setRotation(0.f, 90.f, 0.f);
	sceneObjects.push_back(blueZunit);


	// Setup cameras
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

	glutTimerFunc(0, timer, 0);
	// glutIdleFunc(renderSim); // Use it for maximum performance
	glutTimerFunc(0, refresh, 0); // use it to to get 60 FPS whatever

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutSpecialFunc(processSpecialKeys);
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

	if (!renderer.setRenderMeshesShaderProg("resources/shaders/mesh.vert", "resources/shaders/mesh.frag") ||
		!renderer.setRenderTextShaderProg("resources/shaders/ttf.vert", "resources/shaders/ttf.frag"))
		return (1);

	//  GLUT main loop
	glutMainLoop();

	return (0);
}

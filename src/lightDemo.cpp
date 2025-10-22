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
#include <ctime>
#include <array>
#include <random>
#include <iomanip>

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
#include "drone.h"
#include "autoMover.h"
#include "package.h"
#include "camera.h"
#include "collision.h"
#include "flare.h"
#include "particle.cpp"

#ifndef RESOURCE_BASE
#define RESOURCE_BASE "resources/"
#endif
#define FONT_FOLDER RESOURCE_BASE "fonts/"
#define ASSET_FOLDER RESOURCE_BASE "assets/"
#define SHADER_FOLDER RESOURCE_BASE "shaders/"

#define MAX_PARTICLES 1500
#define frand() ((float)rand() / RAND_MAX)

inline int clampi(const int x, const int min, const int max)
{
	return (x < min ? min : (x > max ? max : x));
}

struct
{
	const char *Drone_OBJ = ASSET_FOLDER "drone.obj";
	const char *Grass_OBJ = ASSET_FOLDER "grass_billboard.obj";
	const char *Tree_OBJ = ASSET_FOLDER "Tree.obj";

	const char *Stone_Tex = ASSET_FOLDER "stone.tga";
	const char *Floor_Tex = ASSET_FOLDER "floor_grass.png";
	const char *Window_Tex = ASSET_FOLDER "window.png";
	const char *BBGrass_Tex = ASSET_FOLDER "billboard_grass.png";
	const char *BBTree_Tex = ASSET_FOLDER "Tree_DM.png";
	const char *Lightwood_Tex = ASSET_FOLDER "lightwood.tga";
	const char *Particle_Tex = ASSET_FOLDER "particle.tga";
	const char *Normalmap_Tex = ASSET_FOLDER "grassNormal.png";
	const char *CRLC_Tex = ASSET_FOLDER "crcl.tga";
	const char *FLAR_Tex = ASSET_FOLDER "flar.tga";
	const char *HXGN_Tex = ASSET_FOLDER "hxgn.tga";
	const char *RING_Tex = ASSET_FOLDER "ring.tga";
	const char *SUN_Tex = ASSET_FOLDER "sun.tga";

	const char *Flare_Tex = ASSET_FOLDER "flare.txt";

	const char *Skybox_Cubemap_Day[6] = {
		ASSET_FOLDER "skybox/day_right.png", ASSET_FOLDER "skybox/day_left.png",
		ASSET_FOLDER "skybox/day_top.png", ASSET_FOLDER "skybox/day_bottom.png",
		ASSET_FOLDER "skybox/day_front.png", ASSET_FOLDER "skybox/day_back.png"};

	const char *Skybox_Cubemap_Night[6] = {
		ASSET_FOLDER "skybox/night_right.png", ASSET_FOLDER "skybox/night_left.png",
		ASSET_FOLDER "skybox/night_top.png", ASSET_FOLDER "skybox/night_bottom.png",
		ASSET_FOLDER "skybox/night_front.png", ASSET_FOLDER "skybox/night_back.png"};

	const char *Font_File = FONT_FOLDER "arial.ttf";

	const char *Mesh_Vert = SHADER_FOLDER "mesh.vert";
	const char *Mesh_Frag = SHADER_FOLDER "mesh.frag";
	const char *Font_Vert = SHADER_FOLDER "ttf.vert";
	const char *Font_Frag = SHADER_FOLDER "ttf.frag";
	const char *Post_Vert = SHADER_FOLDER "skybox.vert";
	const char *Post_Frag = SHADER_FOLDER "skybox.frag";
} FILEPATH;

struct
{
	int WindowHandle = 0;
	int WinX = 1024, WinY = 695;
	unsigned int FrameCount = 0;
	const char *WinTitle = "AVT Project 2025 G13";
	int numMovers = 15;
	// Mouse Tracking Variables
	int startX = 0, startY = 0, tracking = 0;
	float mouseSensitivity = 0.3f;

	float lastTime = glutGet(GLUT_ELAPSED_TIME);
	bool fontLoaded = false;

	bool daytime = true;
	bool showPointlights = true;
	bool showSpotlights = true;
	bool showFog = true;
	bool showDebug = false;
	bool showKeybinds = false;
	bool fireworksOn = false;
	unsigned int cubemap_dayID = 0;
	unsigned int cubemap_nightID = 0;
	bool paused = false;
} GLOBAL;

gmu mu;
Renderer renderer;

Drone *drone;
SceneObject *floorObject;
std::vector<Light> sceneLights;
std::vector<SceneObject *> sceneObjects;
std::vector<SceneObject *> transparentObjects;
CollisionSystem collisionSystem;
Package *package = nullptr;
SceneObject *destination = nullptr;
std::vector<SceneObject *> billboardObjects;
std::vector<Particle *> particle_vector;
// Store building quadrants for package delivery
std::vector<std::vector<SceneObject *>> cityQuadrants;
int destinationMeshID = -1;
// Random engine
std::mt19937 gen{std::random_device{}()}; // random engine

// Flare
FLARE_DEF lensFlare;
GLuint FlareTextureArray[5];
float lightPos[4] = {1000.0f, 1000.0f, 0.0f, 1.0f}; // position of point light in World coordinates
int flareQuadID;
int offsetId = 0;

Camera *cams[3];
int activeCam = 0;
SceneObject *stencilQuad = nullptr;
int stencilQuadID = -1;

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
	glutTimerFunc(1000 / 600, refresh, 0);
}

void gameloop(void)
{
	float currentTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (currentTime - GLOBAL.lastTime) / 1000.0f; // Convert milliseconds to seconds
	GLOBAL.lastTime = currentTime;

	// Update all scene objects with the elapsed time
	for (auto obj : sceneObjects)
		if (GLOBAL.paused == false)
			obj->update(deltaTime);

	if (GLOBAL.fireworksOn)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		for (auto &particle : particle_vector)
			particle->update(deltaTime);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
	}

	glutPostRedisplay();
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h)
{
	float ratio;
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);

	GLOBAL.WinX = w;
	GLOBAL.WinY = h;

	// Set the perspective/orthographic matrix for main camera
	ratio = (1.0f * w) / h;
	mu.loadIdentity(gmu::PROJECTION);

	if (cams[activeCam]->getProjectionType() == ProjectionType::Orthographic)
	{
		float viewSize = 30.0f;
		mu.ortho(-viewSize, viewSize, -viewSize, viewSize, -100.0f, 100.0f);
	}
	else
	{
		mu.perspective(53.13f, ratio, 0.1f, 1000.0f);
	}

	// Size of rear-view mirror
	float quadWidth = 256.0f;
	float quadHeight = 174.0f;

	// Recompute center based on new window size
	float posX = GLOBAL.WinX - quadWidth / 2.0f;
	float posY = GLOBAL.WinY - quadHeight / 2.0f;

	stencilQuad->setPosition(posX, posY, 0.0f);
	stencilQuad->setScale(quadWidth, quadHeight, 1.f);
}

//
// Particles
//
void buildParticles(int particleQuadID)
{
	GLfloat v, theta, phi;

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		v = frand() + 1;
		phi = frand() * PI_F;
		theta = 2.0 * frand() * PI_F;

		Particle *particle = new Particle({particleQuadID}, TexMode::TEXTURE_PARTICLE, 1.0f, 0.3f,
										  0.0f, 10.0f, 0.0f,
										  v * cos(theta) * sin(phi),
										  v * cos(phi),
										  v * sin(theta) * sin(phi),
										  0.1f, -0.15f, 0.0f);
		particle_vector.push_back(particle);
	}
}

void reset_particles(void)
{
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		particle_vector[i]->reset();
		particle_vector[i]->setPosition(drone->pos[0], drone->pos[1] + 5.0f, drone->pos[2]);
	}
}

//
// Render stufff
//

void drawObjects(void)
{
	for (auto &obj : sceneObjects)
		obj->render(renderer, mu);

	for (auto &obj : billboardObjects)
	{
		float dirX = cams[activeCam]->getX() - obj->pos[0];
		float dirZ = cams[activeCam]->getZ() - obj->pos[2];
		float yaw = atan2(dirX, dirZ) * (180.0f / PI_F) + 180.f;
		obj->setRotation(yaw, 0.f, 0.f);
		obj->render(renderer, mu);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (auto obj : transparentObjects)
		obj->render(renderer, mu);
	glDisable(GL_BLEND);
}

void renderFlare(FLARE_DEF *flare, int lx, int ly, int *m_viewport, int flareQuadID)
{
	int dx, dy, px, py, cx, cy;
	float maxflaredist, flaredist, flaremaxsize, flarescale, scaleDistance;
	int width, height;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	int screenMaxCoordX = m_viewport[0] + m_viewport[2] - 1;
	int screenMaxCoordY = m_viewport[1] + m_viewport[3] - 1;

	// Viewport center
	cx = m_viewport[0] + (int)(0.5f * (float)m_viewport[2]) - 1;
	cy = m_viewport[1] + (int)(0.5f * (float)m_viewport[3]) - 1;

	// Compute distance from center
	maxflaredist = sqrt(cx * cx + cy * cy);
	flaredist = sqrt((lx - cx) * (lx - cx) + (ly - cy) * (ly - cy));
	scaleDistance = (maxflaredist - flaredist) / maxflaredist;
	flaremaxsize = (int)(m_viewport[2] * flare->fMaxSize);
	flarescale = (int)(m_viewport[2] * flare->fScale);

	// Destination is opposite side of center from source
	dx = clampi(cx + (cx - lx), m_viewport[0], screenMaxCoordX);
	dy = clampi(cy + (cy - ly), m_viewport[1], screenMaxCoordY);

	renderer.activateRenderMeshesShaderProg();

	for (int i = 0; i < flare->nPieces; ++i)
	{
		// Interpolate position along line
		px = (int)((1.0f - flare->element[i].fDistance) * lx + flare->element[i].fDistance * dx);
		py = (int)((1.0f - flare->element[i].fDistance) * ly + flare->element[i].fDistance * dy);
		px = clampi(px, m_viewport[0], screenMaxCoordX);
		py = clampi(py, m_viewport[1], screenMaxCoordY);

		width = (int)(scaleDistance * flarescale * flare->element[i].fSize);
		if (width > flaremaxsize)
			width = flaremaxsize;
		height = (int)((float)m_viewport[3] / (float)m_viewport[2] * (float)width);

		float diffuse[4];
		memcpy(diffuse, flare->element[i].matDiffuse, 4 * sizeof(float));
		diffuse[3] *= scaleDistance;

		if (width > 1)
		{
			// Setup transformation for this flare element
			mu.pushMatrix(gmu::MODEL);
			mu.translate(gmu::MODEL, (float)(px - width * 0.5f), (float)(py - height * 0.5f), 0.0f);
			mu.scale(gmu::MODEL, (float)width, (float)height, 1.0f);

			// Create and render flare quad
			SceneObject flareObj({flareQuadID}, flare->element[i].textureId + offsetId);
			flareObj.setPosition(0, 0, 0); // Position is handled by MODEL matrix
			flareObj.setScale(1, 1, 1);	   // Scale is handled by MODEL matrix
			flareObj.render(renderer, mu);

			mu.popMatrix(gmu::MODEL);
		}
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void renderSim(void)
{
	GLOBAL.FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// ===== STEP 1: CREATE STENCIL MASK =====
	if (stencilQuad && activeCam == 2)
	{
		mu.pushMatrix(gmu::PROJECTION);
		mu.loadIdentity(gmu::PROJECTION);

		// Pixel-perfect ortho
		mu.ortho(0.0f, (float)GLOBAL.WinX, 0.0f, (float)GLOBAL.WinY, -10.0f, 10.0f);

		mu.loadIdentity(gmu::VIEW);
		mu.loadIdentity(gmu::MODEL);

		glStencilFunc(GL_NEVER, 0x2, 0x3);
		glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		renderer.activateRenderMeshesShaderProg();
		stencilQuad->render(renderer, mu);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);

		mu.popMatrix(gmu::PROJECTION);
	}

	// ===== STEP 2: RENDER REAR VIEW (where stencil == 1) =====
	if (stencilQuad && activeCam == 2)
	{
		glStencilFunc(GL_EQUAL, 0x2, 0x3);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// Convert quad position/scale from pixels to viewport
		int vpX = (int)(stencilQuad->pos[0] - stencilQuad->scale[0] / 2.0f);
		int vpY = (int)(stencilQuad->pos[1] - stencilQuad->scale[1] / 2.0f);
		int vpWidth = (int)stencilQuad->scale[0];
		int vpHeight = (int)stencilQuad->scale[1];

		glViewport(vpX, vpY, vpWidth, vpHeight);

		// Setup rear-view camera (better third-person behind-drone camera)
		mu.loadIdentity(gmu::VIEW);
		mu.loadIdentity(gmu::MODEL);

		// convert yaw and build forward vector
		float yawRad = drone->yaw * PI_F / 180.0f;
		float fx = sinf(yawRad);
		float fz = cosf(yawRad);

		// choose sensible distances for a mirror view
		float distanceBehind = 8.0f; // camera sits 8 units behind drone
		float lookBehind = 12.0f;	 // how far the camera looks behind the drone
		float heightOffset = 0.0f;	 // lift camera above drone

		// camera position: behind and up
		float camX = drone->pos[0] - fx * distanceBehind;
		float camY = drone->pos[1] + heightOffset;
		float camZ = drone->pos[2] - fz * distanceBehind;

		// camera target: a point behind the drone (so mirror looks backward)
		float targetX = drone->pos[0] - fx * (distanceBehind + lookBehind);
		float targetY = drone->pos[1];
		float targetZ = drone->pos[2] - fz * (distanceBehind + lookBehind);

		mu.lookAt(camX, camY, camZ, targetX, targetY, targetZ, 0.0f, 1.0f, 0.0f);

		// perspective — use small near plane and moderate far plane
		mu.loadIdentity(gmu::PROJECTION);
		float ratio = (float)vpWidth / (float)vpHeight;
		mu.perspective(53.13f, ratio, 0.1f, 1000.0f);

		// Set fog for rear-view
		float fogColor[] = {0.f, 0.f, 0.f, 0.f};
		if (GLOBAL.showFog)
		{
			float lightgray[] = {.75f, 0.85f, 0.75f, 1.f};
			float darkgray[] = {0.15f, 0.15f, 0.15f, 1.f};
			if (GLOBAL.daytime)
			{
				for (int i = 0; i < 4; i++)
					fogColor[i] = lightgray[i];
			}
			else
			{
				for (int i = 0; i < 4; i++)
					fogColor[i] = darkgray[i];
			}
		}
		renderer.setFogColor(fogColor);

		// Render scene in rear-view mirror
		renderer.activateRenderMeshesShaderProg();
		renderer.resetLights();

		// Setup lights
		for (auto &light : sceneLights)
			light.setup(renderer, mu);

		// Render opaque objects
		for (auto &obj : sceneObjects)
			obj->render(renderer, mu);

		// Render billboard objects (grass, trees) oriented to rear camera
		for (auto &obj : billboardObjects)
		{
			float dirX = camX - obj->pos[0];
			float dirZ = camZ - obj->pos[2];
			float yaw = atan2(dirX, dirZ) * (180.0f / PI_F) + 180.f;
			obj->setRotation(yaw, 0.f, 0.f);
			obj->render(renderer, mu);
		}

		floorObject->render(renderer, mu);

		// Render skybox centered on rear camera
		mu.pushMatrix(gmu::MODEL);
		mu.translate(gmu::MODEL, camX, camY, camZ);
		mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL);
		float *m_VP = mu.get(gmu::PROJ_VIEW_MODEL);
		if (GLOBAL.daytime)
		{
			unsigned int id = renderer.TexObjArray.getTextureId(GLOBAL.cubemap_dayID);
			renderer.activateSkyboxShaderProg(m_VP, id, fogColor);
		}
		else
		{
			unsigned int id = renderer.TexObjArray.getTextureId(GLOBAL.cubemap_nightID);
			renderer.activateSkyboxShaderProg(m_VP, id, fogColor);
		}
		mu.popMatrix(gmu::MODEL);

		// Re-activate mesh shader for transparent objects
		renderer.activateRenderMeshesShaderProg();

		// Render transparent objects (windows) - sorted from rear camera position
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		auto rearCmp = [&](SceneObject *a, SceneObject *b)
		{
			float lenA_X = (a->pos[0] - camX);
			float lenA_Y = (a->pos[1] - camY);
			float lenA_Z = (a->pos[2] - camZ);
			float lenA = (lenA_X * lenA_X) + (lenA_Y * lenA_Y) + (lenA_Z * lenA_Z);

			float lenB_X = (b->pos[0] - camX);
			float lenB_Y = (b->pos[1] - camY);
			float lenB_Z = (b->pos[2] - camZ);
			float lenB = (lenB_X * lenB_X) + (lenB_Y * lenB_Y) + (lenB_Z * lenB_Z);

			return (lenA > lenB);
		};

		std::vector<SceneObject *> rearTransparentObjects = transparentObjects; // Copy to avoid modifying main list
		std::sort(rearTransparentObjects.begin(), rearTransparentObjects.end(), rearCmp);
		for (auto obj : rearTransparentObjects)
			obj->render(renderer, mu);

		glDisable(GL_BLEND);

		// Restore full viewport for main rendering
		glViewport(0, 0, GLOBAL.WinX, GLOBAL.WinY);
	}

	// ===== STEP 3: RENDER MAIN VIEW (where stencil != 1) =====
	glStencilFunc(GL_NOTEQUAL, 0x2, 0x3);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// use the required GLSL program to draw the meshes with illumination
	renderer.activateRenderMeshesShaderProg();
	renderer.resetLights();

	// Associar os Texture Units aos Objects Texture
	renderer.setTexUnit(0, 0);	 // Stone
	renderer.setTexUnit(1, 1);	 // Floor grass
	renderer.setTexUnit(2, 2);	 // Window
	renderer.setTexUnit(3, 3);	 // Billboard grass
	renderer.setTexUnit(4, 4);	 // Billboard tree
	renderer.setTexUnit(5, 5);	 // Lightwood
	renderer.setTexUnit(6, 6);	 // Particle
	renderer.setTexUnit(7, 7);	 // Normalmap
	renderer.setTexUnit(8, 8);	 // Crcl
	renderer.setTexUnit(9, 9);	 // Flar
	renderer.setTexUnit(10, 10); // hxgn
	renderer.setTexUnit(11, 11); // ring
	renderer.setTexUnit(12, 12); // sun

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
		mu.perspective(53.13f, ratio, 0.1f, 800.0f);
	}

	float fogColor[] = {0.f, 0.f, 0.f, 0.f};
	if (GLOBAL.showFog)
	{
		float lightgray[] = {.75f, 0.85f, 0.75f, 1.f};
		float darkgray[] = {0.15f, 0.15f, 0.15f, 1.f};
		if (GLOBAL.daytime)
		{
			for (int i = 0; i < 4; i++)
				fogColor[i] = lightgray[i];
		}
		else
		{
			for (int i = 0; i < 4; i++)
				fogColor[i] = darkgray[i];
		}
	}
	renderer.setFogColor(fogColor);

	auto cmp = [&](SceneObject *a, SceneObject *b)
	{
		float camX = cams[activeCam]->getX();
		float camY = cams[activeCam]->getY();
		float camZ = cams[activeCam]->getZ();

		float lenA_X = (a->pos[0] - camX);
		float lenA_Y = (a->pos[1] - camY);
		float lenA_Z = (a->pos[2] - camZ);
		float lenA = (lenA_X * lenA_X) + (lenA_Y * lenA_Y) + (lenA_Z * lenA_Z);

		float lenB_X = (b->pos[0] - camX);
		float lenB_Y = (b->pos[1] - camY);
		float lenB_Z = (b->pos[2] - camZ);
		float lenB = (lenB_X * lenB_X) + (lenB_Y * lenB_Y) + (lenB_Z * lenB_Z);

		return (lenA > lenB);
	};
	std::sort(particle_vector.begin(), particle_vector.end(), cmp);
	std::sort(transparentObjects.begin(), transparentObjects.end(), cmp);

	/*  RENDER QUEUE
	  0) render skybox
	  1) setup the lights
	  2) render opaque objects
	  2) render billboard objects
	  3) render floor
	  4) render particles (if any)
	  5) render transparent objects
	*/

	// Render skybox
	mu.pushMatrix(gmu::MODEL);
	mu.translate(gmu::MODEL, cams[activeCam]->getX(), cams[activeCam]->getY(), cams[activeCam]->getZ());
	mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL);
	float *m_VP = mu.get(gmu::PROJ_VIEW_MODEL);
	if (GLOBAL.daytime)
	{
		unsigned int id = renderer.TexObjArray.getTextureId(GLOBAL.cubemap_dayID);
		renderer.activateSkyboxShaderProg(m_VP, id, fogColor);
	}
	else
	{
		unsigned int id = renderer.TexObjArray.getTextureId(GLOBAL.cubemap_nightID);
		renderer.activateSkyboxShaderProg(m_VP, id, fogColor);
	}
	mu.popMatrix(gmu::MODEL);
	renderer.activateRenderMeshesShaderProg();

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 0x2, 0x2);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

	if (!(stencilQuad))
	{
		floorObject->render(renderer, mu);
	}

	glStencilFunc(GL_EQUAL, 0x1, 0x3);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_DEPTH_TEST);

	renderer.invert = true;
	for (auto &light : sceneLights)
	{
		light.invertY();
		light.setup(renderer, mu);
	}

	glCullFace(GL_FRONT);
	// render reflections
	drawObjects();
	glCullFace(GL_BACK);

	renderer.invert = false;
	renderer.resetLights();
	for (auto &light : sceneLights)
	{
		light.invertY();
		light.setup(renderer, mu);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (stencilQuad)
	{
		glStencilFunc(GL_NOTEQUAL, 0x2, 0x3);
	}

	floorObject->render(renderer, mu);

	// Dark the color stored in color buffer
	glDisable(GL_DEPTH_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

	// render shadows
	renderer.shadow = true;
	drawObjects();
	renderer.shadow = false;
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glStencilFunc(GL_GREATER, 0x2, 0x3);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// render real objects
	drawObjects();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (GLOBAL.fireworksOn)
	{
		glDisable(GL_CULL_FACE); // see both sides of the quad
		for (auto &particle : particle_vector)
		{
			particle->setCameraPos(cams[activeCam]->getX(), cams[activeCam]->getY(), cams[activeCam]->getZ());
			particle->render(renderer, mu);
		}
		glEnable(GL_CULL_FACE);

		int dead_num_particles = 0;
		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			if (particle_vector[i]->curr_life <= 0.0f)
				dead_num_particles++;
		}
		if (dead_num_particles == MAX_PARTICLES)
		{
			GLOBAL.fireworksOn = false;
			printf("All particles dead\n");
		}
	}

	// Flare
	if (activeCam == 2)
	{
		float lightScreenPos[3];
		int flarePos[2];
		int m_viewport[4];
		glGetIntegerv(GL_VIEWPORT, m_viewport);

		mu.pushMatrix(gmu::MODEL);
		mu.loadIdentity(gmu::MODEL);
		mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL); // pvm to be applied to lightPost. pvm is used in project function

		if (!mu.project(lightPos, lightScreenPos, m_viewport))
			printf("Error in getting projected light in screen\n"); // Calculate the window Coordinates of the light position: the projected position of light on viewport
		flarePos[0] = clampi((int)lightScreenPos[0], m_viewport[0], m_viewport[0] + m_viewport[2] - 1);
		flarePos[1] = clampi((int)lightScreenPos[1], m_viewport[1], m_viewport[1] + m_viewport[3] - 1);

		mu.popMatrix(gmu::MODEL);
		// viewer looking down at  negative z direction
		mu.pushMatrix(gmu::PROJECTION);
		mu.loadIdentity(gmu::PROJECTION);
		mu.pushMatrix(gmu::VIEW);
		mu.loadIdentity(gmu::VIEW);
		mu.ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);

		renderFlare(&lensFlare, flarePos[0], flarePos[1], m_viewport, flareQuadID);

		mu.popMatrix(gmu::PROJECTION);
		mu.popMatrix(gmu::VIEW);
	}

	for (auto obj : transparentObjects)
		obj->render(renderer, mu);
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
		float Ypos = 0.f, Yoff = 80.f;

		// --- HUD: battery and score ---
		texts.push_back(TextCommand{
			("Battery: " + std::to_string((int)drone->getBatteryLevel()) + "%").c_str(),
			{720.f, 0.0f}, // screen coords (pixels)
			size,
			{0.9f, 0.9f, 0.0f, 1.0f} // yellow
		});

		texts.push_back(TextCommand{
			("Score: " + std::to_string((int)drone->getScore())).c_str(),
			{720.f, Yoff}, // below battery
			size,
			{0.0f, 0.9f, 0.9f, 1.0f} // cyan
		});

		if (GLOBAL.paused)
		{
			texts.push_back(TextCommand{
				"PAUSED",
				{GLOBAL.WinX / 2.0f - 140.f, GLOBAL.WinY / 2.0f},
				1.0f,
				{0.9f, 0.1f, 0.1f, 1.0f} // red
			});
		}

		if (GLOBAL.paused == false)
		{
			if (drone->isDisabled())
			{
				texts.push_back(TextCommand{
					"Game Over! No battery!",
					{GLOBAL.WinX / 2.0f - 360.f, GLOBAL.WinY / 2.0f},
					1.0f,
					{0.9f, 0.1f, 0.1f, 1.0f} // red
				});
				texts.push_back(TextCommand{
					"Click 'R' to reset.",
					{(GLOBAL.WinX / 2.0f) - 100.f, (GLOBAL.WinY / 2.0f)},
					0.5f,
					{0.9f, 0.1f, 0.1f, 1.0f} // red
				});
			}
		}

		if (GLOBAL.showKeybinds)
		{
			// push_back the keybind gui
			texts.push_back(TextCommand{
				"Press 'i' to hide keybinds",
				{0.f, Ypos},
				size,
				{.9f, 0.9f, 0.9f, 1.f}});

			Ypos += Yoff;
			if (GLOBAL.showFog)
			{
				texts.push_back(TextCommand{
					"Press 'f' to hide fog",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}
			else
			{
				texts.push_back(TextCommand{
					"Press 'f' to show fog",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}

			Ypos += Yoff;
			if (GLOBAL.showDebug)
			{
				texts.push_back(TextCommand{
					"Press 'k' to hide debug",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}
			else
			{
				texts.push_back(TextCommand{
					"Press 'k' to show debug",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}

			Ypos += Yoff;
			if (GLOBAL.daytime)
			{
				texts.push_back(TextCommand{
					"Press 'n' to hide directional lights",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}
			else
			{
				texts.push_back(TextCommand{
					"Press 'n' to show directional lights",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}

			Ypos += Yoff;
			if (GLOBAL.showPointlights)
			{
				texts.push_back(TextCommand{
					"Press 'c' to hide pointlights",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}
			else
			{
				texts.push_back(TextCommand{
					"Press 'c' to show pointlights",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}

			Ypos += Yoff;
			if (GLOBAL.showSpotlights)
			{
				texts.push_back(TextCommand{
					"Press 'h' to hide spotlights",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}
			else
			{
				texts.push_back(TextCommand{
					"Press 'h' to show spotlights",
					{0.f, Ypos},
					size,
					{.9f, 0.9f, 0.9f, 1.f}});
			}
		}
		else
		{
			texts.push_back(TextCommand{
				"Press 'i' to show keybinds",
				{0.f, 0.f},
				size,
				{.9f, 0.9f, 0.9f, 1.f}});
		}

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

// Place package randomly on a building from the given quadrant
void placePackageOnBuilding(int quadrantIndex)
{
	if (quadrantIndex >= (int)cityQuadrants.size() || cityQuadrants[quadrantIndex].empty() || !package)
		return;

	const auto &buildings = cityQuadrants[quadrantIndex];
	SceneObject *building = buildings[rand() % buildings.size()];
	auto pos = building->getPosition();
	auto scale = building->getScale();

	// Place package on top of building (centered)
	float packageSize = 1.0f;
	float packageX, packageZ, packageY;

	// Handle different building types based on quadrant
	int buildingType = quadrantIndex; // 0=cube, 1=cone, 2=cylinder, 3=window

	if (buildingType == 2) // Cylinder quadrant
	{
		packageX = pos[0] - packageSize * 0.5f;		// Center on building (cylinders are center-positioned)
		packageZ = pos[2] - packageSize * 0.5f;		// Center on building (cylinders are center-positioned)
		packageY = pos[1] + scale[1] * 0.5f + 0.1f; // Top of cylinder (pos[1] is center, so add half height)
	}
	else if (buildingType == 1) // Cone quadrant
	{
		packageX = pos[0] - packageSize * 0.5f; // Center on building (cones are center-positioned)
		packageZ = pos[2] - packageSize * 0.5f; // Center on building (cones are center-positioned)
		packageY = pos[1] + scale[1] + 0.1f;	// Top of cone
	}
	else // Cube (0) and Window (3) quadrants
	{
		packageX = pos[0] + scale[0] * 0.5f - packageSize * 0.5f; // Center on building
		packageZ = pos[2] + scale[2] * 0.5f - packageSize * 0.5f; // Center on building
		packageY = pos[1] + scale[1] + 0.1f;					  // Top of building
	}

	package->reset(building, packageX, packageY, packageZ);
	package->updateCollider();

	// std::cout << "Package placed on building at (" << packageX << ", " << packageY << ", " << packageZ << ")\n";
}

// Mark a random building as the delivery destination (with distinct color)
void setDeliveryBuilding(int quadrantIndex)
{
	if (quadrantIndex >= (int)cityQuadrants.size() || cityQuadrants[quadrantIndex].empty())
		return;

	const auto &buildings = cityQuadrants[quadrantIndex];
	SceneObject *deliveryBuilding = buildings[rand() % buildings.size()];

	// Store original position and scale for collision box calculation
	auto pos = deliveryBuilding->getPosition();
	auto scale = deliveryBuilding->getScale();

	// Determine building type based on quadrant
	int buildingType = quadrantIndex; // 0=cube, 1=cone, 2=cylinder, 3=window

	package->setDestination(deliveryBuilding, destinationMeshID);

	if (buildingType == 2) // Cylinder quadrant
	{
		float cubeHeight = scale[1]; // Keep the same height as the cylinder

		float newX = pos[0] - scale[0] * 0.5f; // Convert from center to corner
		float newY = 0.0f;					   // Ground level
		float newZ = pos[2] - scale[2] * 0.5f; // Convert from center to corner
		deliveryBuilding->setPosition(newX, newY, newZ);

		deliveryBuilding->getCollider()->setBox(
			newX,			 // minX = corner position
			0.0f,			 // minY = ground level
			newZ,			 // minZ = corner position
			newX + scale[0], // maxX = corner + width
			cubeHeight,		 // maxY = full height from ground
			newZ + scale[2]	 // maxZ = corner + depth
		);
	}
	else if (buildingType == 1) // Cone quadrant
	{
		float newX = pos[0] - scale[0] * 0.5f; // Convert from center to corner
		float newY = pos[1];				   // Keep ground level
		float newZ = pos[2] - scale[2] * 0.5f; // Convert from center to corner
		deliveryBuilding->setPosition(newX, newY, newZ);

		deliveryBuilding->getCollider()->setBox(
			newX,			   // minX = corner position
			pos[1],			   // minY = ground level
			newZ,			   // minZ = corner position
			newX + scale[0],   // maxX = corner + width
			pos[1] + scale[1], // maxY = ground + height
			newZ + scale[2]	   // maxZ = corner + depth
		);
	}
	else // Cube (0) and Window (3) quadrants
	{
		deliveryBuilding->getCollider()->setBox(
			pos[0],			   // minX = corner position
			pos[1],			   // minY = ground level
			pos[2],			   // minZ = corner position
			pos[0] + scale[0], // maxX = corner + width
			pos[1] + scale[1], // maxY = ground + height
			pos[2] + scale[2]  // maxZ = corner + depth
		);
	}

	// std::cout << "Delivery building marked with yellow glow!\n";
}

// Reset the delivery mission (called after successful delivery)
void resetDelivery()
{
	if (!package || cityQuadrants.empty())
		return;

	// Reset previous destination building to normal color if needed
	if (destination)
	{
		// For simplicity, just leave it yellow - you could track original mesh per building
		// std::cout << "Previous destination remains marked\n";
	}

	// Pick random quadrants for package and destination
	int pickupQuadrant = rand() % cityQuadrants.size();
	int deliveryQuadrant;
	do
	{
		deliveryQuadrant = rand() % cityQuadrants.size();
	} while (deliveryQuadrant == pickupQuadrant && cityQuadrants.size() > 1);

	// Place package and set destination
	placePackageOnBuilding(pickupQuadrant);
	setDeliveryBuilding(deliveryQuadrant);

	// std::cout << "New delivery mission started!\n";
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	(void)xx;
	(void)yy;

	for (auto obj : sceneObjects)
		obj->handleKeyInput(key);

	switch (key)
	{

	case 27:
		glutLeaveMainLoop();
		break;

	case 'n': // toggle directional light
		GLOBAL.daytime = !GLOBAL.daytime;
		for (auto &light : sceneLights)
			if (light.isType(LightType::DIRECTIONAL))
				light.toggleLight();
		break;

	case 'c': // toggle point lights
		GLOBAL.showPointlights = !GLOBAL.showPointlights;
		for (auto &light : sceneLights)
			if (light.isType(LightType::POINTLIGHT))
				light.toggleLight();
		break;

	case 'h': // toggle spotlights
		GLOBAL.showSpotlights = !GLOBAL.showSpotlights;
		for (auto &light : sceneLights)
			if (light.isType(LightType::SPOTLIGHT))
				light.toggleLight();
		break;

	case 'k': // toggle debug
		GLOBAL.showDebug = !GLOBAL.showDebug;
		for (auto &light : sceneLights)
			light.toggleObj();
		// if (light.isDebug())
		break;

	case 'f': // toggle fog
		GLOBAL.showFog = !GLOBAL.showFog;
		break;

	case 'i':
		GLOBAL.showKeybinds = !GLOBAL.showKeybinds;
		break;

	case 'p': // Pause the game
		GLOBAL.paused = !GLOBAL.paused;
		break;

	case 'r': // Restart the game (Reset drone position and new package and delivery building)
		if (GLOBAL.paused == false && drone->isDisabled())
		{
			drone->setPosition(0.0f, 5.0f, 0.0f);
			drone->setScale(1.6f, 2.f, 1.4f);
			drone->getCollider()->setBox(-2.24f, 5.0f, -2.52f, 2.24f, 6.2f, 2.52f);
			drone->setBatteryLevel(Drone::MAX_BATTERY);
			drone->setScore(0);
			drone->enable();
			resetDelivery();
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
	(void)xx;
	(void)yy;

	for (auto obj : sceneObjects)
		obj->handleKeyRelease(key);
}

void processSpecialKeys(int key, int xx, int yy)
{
	(void)xx;
	(void)yy;

	for (auto obj : sceneObjects)
		obj->handleSpecialKeyInput(key);
}

void processSpecialKeysUp(int key, int xx, int yy)
{
	(void)xx;
	(void)yy;

	for (auto obj : sceneObjects)
		obj->handleSpecialKeyRelease(key);
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

	drone->updateCameraOffset();

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
void buildCityWithPackages(
	int quadID, int cubeID, int coneID, int cylinderID, int torusID,
	const std::vector<int> &grassMeshIDs, int packageID, int destinationID,
	const std::vector<int> &treeMeshID1, const std::vector<int> &treeMeshID2, const std::vector<int> &treeMeshID3)
{
	const float floorSize = 300.0f;
	const float cityRadius = 75.0f;
	const int torusCount = 20;
	const float PI2 = 2.0f * PI_F;

	auto addBox = [&](SceneObject *obj, float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	{
		obj->getCollider()->setBox(minX, minY, minZ, maxX, maxY, maxZ);
		collisionSystem.addCollider(obj->getCollider());
	};

	// --------------------------------------------------------------------
	// Floor
	floorObject = new SceneObject(std::vector<int>{quadID}, TexMode::TEXTURE_FLOOR);
	floorObject->setPosition(0.0f, 0.1f, 0.0f);
	floorObject->setRotation(0.0f, -90.0f, 0.0f);
	floorObject->setScale(floorSize, floorSize, 10.0f);
	addBox(floorObject, -floorSize, -0.1f, -floorSize, floorSize, 0.0f, floorSize);

	// --------------------------------------------------------------------
	// Circular torus ring
	for (int i = 0; i < torusCount; ++i)
	{
		float angle = i * (PI2 / torusCount);
		float x = std::cos(angle) * cityRadius;
		float z = std::sin(angle) * cityRadius;

		SceneObject* ring = new SceneObject(std::vector<int>{torusID}, 13);//TexMode::TEXTURE_STONE);
		ring->setPosition(x, 2.0f, z);
		ring->setRotation(90.0f, 0.0f, 0.0f);
		ring->setScale(8.0f, 4.0f, 8.0f);
		sceneObjects.push_back(ring);
	}

	// --------------------------------------------------------------------
	// Grid placement helper - simplified
	auto placeGridBuildings = [&](float startAngleDeg, float endAngleDeg, int rows, int cols, float spacing, auto builderFn) -> std::vector<SceneObject *>
	{
		float angleMid = (startAngleDeg + endAngleDeg) / 2.0f * PI_F / 180.0f;
		float quadrantCenterX = std::cos(angleMid) * cityRadius * 0.5f;
		float quadrantCenterZ = std::sin(angleMid) * cityRadius * 0.5f;

		float startX = quadrantCenterX - (cols - 1) * spacing * 0.5f;
		float startZ = quadrantCenterZ - (rows - 1) * spacing * 0.5f;

		std::vector<SceneObject *> quadrantBuildings;

		// Place all buildings
		for (int r = 0; r < rows; ++r)
		{
			for (int c = 0; c < cols; ++c)
			{
				float x = startX + c * spacing;
				float z = startZ + r * spacing;
				SceneObject *obj = builderFn(x, z, r * cols + c);
				quadrantBuildings.push_back(obj);
			}
		}

		return quadrantBuildings;
	};

	// --------------------------------------------------------------------
	// Quadrant builders
	auto cubeBuilder = [&](float x, float z, int i)
	{
		SceneObject *tower = new SceneObject(std::vector<int>{cubeID}, TexMode::TEXTURE_STONE);
		tower->setScale(2.0f, 6.0f + (i % 5), 2.0f);
		tower->setPosition(x, 0.0f, z);
		sceneObjects.push_back(tower);
		addBox(tower, x, 0.0f, z, x + 2.0f, 6.0f + (i % 5), z + 2.0f);
		return tower;
	};

	auto coneBuilder = [&](float x, float z, int i)
	{
		SceneObject *pyramid = new SceneObject(std::vector<int>{coneID}, TexMode::TEXTURE_STONE);
		pyramid->setScale(2.5f, 5.0f + (i % 3), 2.5f);
		pyramid->setPosition(x, 0.0f, z);
		sceneObjects.push_back(pyramid);
		addBox(pyramid, x - 1.25f, 0.0f, z - 1.25f, x + 1.25f, 5.0f + (i % 3), z + 1.25f);
		return pyramid;
	};

	auto cylinderBuilder = [&](float x, float z, int i)
	{
		SceneObject *cyl = new SceneObject(std::vector<int>{cylinderID}, TexMode::TEXTURE_STONE);
		cyl->setScale(1.5f, 8.0f + (i % 5), 1.5f);
		auto scale = cyl->getScale();
		cyl->setPosition(x, scale[1] * 0.5f, z);
		sceneObjects.push_back(cyl);
		addBox(cyl, x - 1.5f, 0.0f, z - 1.5f, x + 1.5f, scale[1], z + 1.5f);
		return cyl;
	};

	auto windowBuilder = [&](float x, float z, int i)
	{
		SceneObject *window = new SceneObject(std::vector<int>{cubeID}, TexMode::TEXTURE_WINDOW);
		window->setScale(3.0f, 10.0f + (i % 3), 3.0f);
		window->setPosition(x, 0.0f, z);
		transparentObjects.push_back(window);
		addBox(window, x, 0.0f, z, x + 3.0f, 10.0f + (i % 3), z + 3.0f);
		return window;
	};

	// --------------------------------------------------------------------
	// Place quadrants and store building lists for delivery
	cityQuadrants.clear();
	cityQuadrants.push_back(placeGridBuildings(0.0f, 90.0f, 4, 5, 5.0f, cubeBuilder));
	cityQuadrants.push_back(placeGridBuildings(90.0f, 180.0f, 4, 5, 5.0f, coneBuilder));
	cityQuadrants.push_back(placeGridBuildings(180.0f, 270.0f, 4, 5, 5.0f, cylinderBuilder));
	cityQuadrants.push_back(placeGridBuildings(270.0f, 360.0f, 4, 5, 5.0f, windowBuilder));

	// Store mesh IDs for later use
	destinationMeshID = destinationID;

	// Create the package object
	package = new Package({packageID}, TexMode::TEXTURE_LIGHTWOOD);
	package->setScale(1.0f, 1.0f, 1.0f);
	sceneObjects.push_back(package);

	// Set up delivery callback - triggers when package is delivered
	package->onDelivered = []()
	{
		// std::cout << "Package delivered! Resetting mission...\n";
		resetDelivery();
		reset_particles();
		GLOBAL.fireworksOn = true;
	};

	// Initialize first delivery mission
	int pickupQuadrant = rand() % cityQuadrants.size();
	int deliveryQuadrant = (pickupQuadrant + 1 + rand() % 3) % cityQuadrants.size(); // Different quadrant

	placePackageOnBuilding(pickupQuadrant);
	setDeliveryBuilding(deliveryQuadrant);

	collisionSystem.addCollider(package->getCollider());

	// --------------------------------------------------------------------
	// Grass outside the torus ring
	const int grassCount = 1000; // 500;
	std::uniform_real_distribution<float> pos{-5.f, 5.0f};
	std::uniform_real_distribution<float> col{-2.f, 2.0f};
	for (int i = 0; i < grassCount; ++i)
	{
		SceneObject *grass = new SceneObject(treeMeshID1, TexMode::TEXTURE_BBTREE);
		if (i % 3 == 1)
		{
			grass->meshID = treeMeshID2;
		}
		if (i % 3 == 2)
		{
			grass->meshID = treeMeshID3;
		}
		const float goldRatio = PI_F * (3 - std::sqrt(5.0f));
		const float radius = cityRadius + 20.0f + std::sqrt(i / (float)grassCount) * 50.0f;
		const float angle = i * goldRatio;

		float randX = pos(gen);
		float randZ = pos(gen);

		grass->setPosition(std::cos(angle) * radius + randX, 0.0f, std::sin(angle) * radius + randZ);
		grass->setScale(4.f, 10.f, 4.f);
		billboardObjects.push_back(grass);
	}

	// tree billboards
	const int treeCount = 0; // 2000;
	const float maxRadius = 300.f;
	const float minRadius = 200.f;
	const float goldRatio = PI_F * (3 - std::sqrt(5));
	for (int i = 1; i < treeCount; i++)
	{
		SceneObject *tree = new SceneObject(treeMeshID1, TexMode::TEXTURE_BBTREE);
		if (i % 3 == 1)
		{
			tree->meshID = treeMeshID2;
		}
		if (i % 3 == 2)
		{
			tree->meshID = treeMeshID3;
		}
		const float t = std::sqrt(i / (float)treeCount);
		const float radius = std::sqrt(t * (maxRadius * maxRadius - minRadius * minRadius) + minRadius * minRadius);
		const float angle = i * goldRatio;

		float randX = pos(gen);
		float randZ = pos(gen);

		tree->setPosition(std::cos(angle) * radius + randX, 0.f, 30 + std::sin(angle) * radius + randZ);
		tree->setScale(4.f, 10.f, 4.f);
		billboardObjects.push_back(tree);
	}
}

void buildScene()
{
	// Top Orthogonal Camera
	cams[0] = new Camera();
	cams[0]->setPosition(0.0f, 30.0f, 0.0f);
	cams[0]->setTarget(0.0f, 0.0f, 0.0f);
	cams[0]->setUp(0.0f, 0.0f, -1.0f);
	cams[0]->setProjectionType(ProjectionType::Orthographic);

	// Top Perspective Camera
	cams[1] = new Camera();
	cams[1]->setPosition(0.0f, 50.0f, 0.0f);
	cams[1]->setTarget(0.0f, 0.0f, 0.0f);
	cams[1]->setUp(0.0f, 0.0f, -1.0f);
	cams[1]->setProjectionType(ProjectionType::Perspective);

	// Drone camera
	cams[2] = new Camera();
	cams[2]->setPosition(0.0f, 4.0f, 10.0f);
	cams[2]->setUp(0.0f, 1.0f, 0.0f);
	cams[2]->setProjectionType(ProjectionType::Perspective);

	// Texture Object definition
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Stone_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Floor_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Window_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.BBGrass_Tex, false);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.BBTree_Tex, false);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Lightwood_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Particle_Tex);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.Normalmap_Tex);

	// Flare
	offsetId = renderer.TexObjArray.getNumTextureObjects();
	renderer.TexObjArray.texture2D_Loader(FILEPATH.CRLC_Tex, false);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.FLAR_Tex, false);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.HXGN_Tex, false);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.RING_Tex, false);
	renderer.TexObjArray.texture2D_Loader(FILEPATH.SUN_Tex, false);
	loadFlareFile(&lensFlare, FILEPATH.Flare_Tex);

	GLOBAL.cubemap_dayID = renderer.TexObjArray.getNumTextureObjects();
	renderer.TexObjArray.textureCubeMap_Loader(FILEPATH.Skybox_Cubemap_Day);
	GLOBAL.cubemap_nightID = renderer.TexObjArray.getNumTextureObjects();
	renderer.TexObjArray.textureCubeMap_Loader(FILEPATH.Skybox_Cubemap_Night);

	// Scene geometry with triangle meshes
	MyMesh amesh;

	float amb1[] = {1.f, 1.f, 1.f, 1.f};
	float diff1[] = {1.f, 1.f, 1.f, 1.f};
	float spec[] = {0.8f, 0.8f, 0.8f, 1.f};
	float spec1[] = {0.3f, 0.3f, 0.3f, 1.f};
	float spec2[] = {0.8f, 0.8f, 0.8f, 0.5f};
	float blk[] = {0.f, 0.f, 0.f, 1.f};
	float shininess = 100.0f;
	int texcount = 0;

	// Scene objects

	// create geometry and VAO of the flare quad
	amesh = createQuad(1, 1);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blk, 4 * sizeof(float));
	flareQuadID = renderer.addMesh(amesh);

	// create geometry and VAO of the cube
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blk, 4 * sizeof(float));
	amesh.mat.shininess = 10.0f;
	amesh.mat.texCount = texcount;
	int cubeID = renderer.addMesh(amesh);

	// create geometry and VAO of the floor quad
	amesh = createQuad(1.0f, 1.0f);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec2, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blk, 4 * sizeof(float));
	amesh.mat.shininess = 1.f;
	amesh.mat.texCount = texcount;
	int quadID = renderer.addMesh(amesh);

	// create geometry and VAO of the cone
	amesh = createCone(1.0f, 1.0f, 5);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blk, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	int coneID = renderer.addMesh(amesh);

	// create geometry and VAO of the cylinder
	amesh = createCylinder(1.0f, 1.0f, 20);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blk, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	int cylinderID = renderer.addMesh(amesh);

	amesh = createTorus(1.f, 2.0f, 40, 20);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blk, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	int torusID = renderer.addMesh(amesh);

	amesh = createCube();
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	float yellow[] = {1.f, 1.f, 0.f, 1.f};
	memcpy(amesh.mat.emissive, yellow, 4 * sizeof(float));
	amesh.mat.shininess = 10.0f;
	amesh.mat.texCount = texcount;
	int destinationID = renderer.addMesh(amesh);

	amesh = createCube();
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	float red[] = {1.f, 0.f, 0.f, 1.f};
	memcpy(amesh.mat.emissive, red, 4 * sizeof(float));
	amesh.mat.shininess = 10.0f;
	amesh.mat.texCount = texcount;
	int PackageId = renderer.addMesh(amesh);

	// Load drone model from file
	std::vector<MyMesh> droneMeshs = createFromFile(FILEPATH.Drone_OBJ);
	std::vector<int> droneMeshIDs;
	for (size_t i = 0; i < droneMeshs.size(); i++)
	{
		// set material properties
		memcpy(droneMeshs[i].mat.ambient, amb1, 4 * sizeof(float));
		memcpy(droneMeshs[i].mat.diffuse, diff1, 4 * sizeof(float));
		memcpy(droneMeshs[i].mat.specular, spec1, 4 * sizeof(float));
		memcpy(droneMeshs[i].mat.emissive, blk, 4 * sizeof(float));
		droneMeshs[i].mat.shininess = shininess;
		droneMeshs[i].mat.texCount = texcount;
		int meshID = renderer.addMesh(droneMeshs[i]);
		droneMeshIDs.push_back(meshID);
	}

	// Load grass model from file
	std::vector<MyMesh> grassMesh = createFromFile(FILEPATH.Grass_OBJ);
	std::vector<int> grassMeshIDs;
	for (size_t i = 0; i < grassMesh.size(); i++)
	{
		float amb[] = {10.f, 10.f, 10.f, 10.f};
		// set material properties
		memcpy(grassMesh[i].mat.ambient, amb, 4 * sizeof(float));
		memcpy(grassMesh[i].mat.diffuse, diff1, 4 * sizeof(float));
		memcpy(grassMesh[i].mat.specular, blk, 4 * sizeof(float));
		memcpy(grassMesh[i].mat.emissive, blk, 4 * sizeof(float));
		int meshID = renderer.addMesh(grassMesh[i]);
		grassMeshIDs.push_back(meshID);
	}

	// Load tree model from file
	std::vector<MyMesh> treeMesh = createFromFile(FILEPATH.Tree_OBJ);
	std::vector<int> treeMeshID1;
	for (size_t i = 0; i < treeMesh.size(); i++)
	{
		float amb[] = {10.f, 10.f, 10.f, 10.f};
		// set material properties
		memcpy(treeMesh[i].mat.ambient, amb, 4 * sizeof(float));
		memcpy(treeMesh[i].mat.diffuse, diff1, 4 * sizeof(float));
		memcpy(treeMesh[i].mat.specular, blk, 4 * sizeof(float));
		memcpy(treeMesh[i].mat.emissive, blk, 4 * sizeof(float));
		int meshID = renderer.addMesh(treeMesh[i]);
		treeMeshID1.push_back(meshID);
	}
	std::vector<int> treeMeshID2;
	for (size_t i = 0; i < treeMesh.size(); i++)
	{
		float amb[] = {12.f, 10.f, 10.f, 10.f};
		memcpy(treeMesh[i].mat.ambient, amb, 4 * sizeof(float));
		treeMeshID2.push_back(renderer.addMesh(treeMesh[i]));
	}
	std::vector<int> treeMeshID3;
	for (size_t i = 0; i < treeMesh.size(); i++)
	{
		float amb[] = {10.f, 8.f, 10.f, 10.f};
		memcpy(treeMesh[i].mat.ambient, amb, 4 * sizeof(float));
		treeMeshID3.push_back(renderer.addMesh(treeMesh[i]));
	}

	// Build the city
	buildCityWithPackages(
		quadID, cubeID, coneID, cylinderID, torusID,
		grassMeshIDs, PackageId, destinationID, treeMeshID1, treeMeshID2, treeMeshID3);

	// Drone
	drone = new Drone(cams[2], droneMeshIDs, TexMode::TEXTURE_LIGHTWOOD);
	drone->setPosition(0.0f, 5.0f, 0.0f);
	drone->setScale(1.6f, 2.f, 1.4f);
	drone->getCollider()->setBox(-2.24f, 5.0f, -2.52f, 2.24f, 6.2f, 2.52f);
	sceneObjects.push_back(drone);
	collisionSystem.addCollider(drone->getCollider());

	// create geometry and VAO of the quad for particles
	amesh = createQuad(1, 1);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blk, 4 * sizeof(float));
	amesh.mat.texCount = texcount;
	int particleQuadID = renderer.addMesh(amesh);

	// Fireworks particles
	buildParticles(particleQuadID);

	// create geometry and VAO of the stencil quad for rear-view mirror
	amesh = createQuad(1.0f, 1.0f);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, blk, 4 * sizeof(float));
	amesh.mat.shininess = 1.f;
	amesh.mat.texCount = texcount;
	stencilQuadID = renderer.addMesh(amesh);

	if (stencilQuadID)
	{
		stencilQuad = new SceneObject(std::vector<int>{stencilQuadID}, TexMode::TEXTURE_NONE);
		// Size of the rear-view mirror in pixels
		float quadWidth = 256.0f;  // half the previous width
		float quadHeight = 174.0f; // half the previous height

		// Position the center of the quad in the **top-right**
		float posX = GLOBAL.WinX - quadWidth / 2.0f;  // center X
		float posY = GLOBAL.WinY - quadHeight / 2.0f; // center Y

		stencilQuad->setPosition(posX, posY, 0.0f);
		stencilQuad->setScale(quadWidth, quadHeight, 1.f);
	}

	// Moving obstacles
	std::uniform_real_distribution<float> velocity{4.0f, 8.0f};
	std::uniform_real_distribution<float> position{-100.f, 100.0f};
	std::uniform_real_distribution<float> size{0.5f, 2.0f};
	float spawningRadius = 50.f;
	for (int i = 0; i < GLOBAL.numMovers; i++)
	{
		AutoMover *mover = new AutoMover({torusID}, TexMode::TEXTURE_LIGHTWOOD, spawningRadius, velocity(gen));
		mover->setPosition(position(gen), 5.0f, position(gen));
		mover->setScale(0.75f, 0.75f, 0.75f);
		sceneObjects.push_back(mover);
		collisionSystem.addCollider(mover->getCollider());
	}

	// === SCENE LIGHTS === //
	sceneLights.reserve(50);

	float whiteLight[4] = {1.f, 1.f, 1.f, 1.f};
	float sunDirection[4] = {-1.f, -1.f, 0.001f, 0.f};
	sceneLights.emplace_back(LightType::DIRECTIONAL, whiteLight);
	sceneLights.back().setDirection(sunDirection);

	float blueLight[4] = {0.f, 0.f, 1.f, 1.f};
	float bLightPos[4] = {4.f, 2.f, 2.f, 1.f};
	sceneLights.emplace_back(LightType::POINTLIGHT, blueLight);
	sceneLights.back().setPosition(bLightPos).createObject(renderer, sceneObjects);

	float redLight[4] = {1.f, 0.f, 0.f, 1.f};
	float rLightPos[4] = {0.f, 2.f, 2.f, 1.f};
	sceneLights.emplace_back(LightType::POINTLIGHT, redLight);
	sceneLights.back().setPosition(rLightPos).createObject(renderer, sceneObjects);

	std::uniform_real_distribution<float> lightPos{-20.f, 20.0f};
	std::uniform_real_distribution<float> lightCol{0.f, 1.0f};
	for (int i = 0; i < 0; i++)
	{
		float lightColor[4] = {lightCol(gen), lightCol(gen), lightCol(gen), 1.f};
		float lightPosition[4] = {lightPos(gen), 5.f, lightPos(gen), 1.f};
		sceneLights.emplace_back(LightType::POINTLIGHT, lightColor);
		sceneLights.back().setPosition(lightPosition).createObject(renderer, sceneObjects);
	}

	float magLight[4] = {1.f, 0.f, 1.f, 1.f};
	float yellowLight[4] = {1.f, 1.f, 0.f, 1.f};
	float hlightDir[4] = {0.f, 0.f, -1.f, 0.f};
	sceneLights.emplace_back(LightType::SPOTLIGHT, yellowLight);
	sceneLights.back().setDirection(hlightDir).createObject(renderer, sceneObjects);
	Light &headlight_l = sceneLights.back();

	sceneLights.emplace_back(LightType::SPOTLIGHT, magLight);
	sceneLights.back().setDirection(hlightDir).createObject(renderer, sceneObjects);
	drone->addHeadlight(headlight_l, sceneLights.back());

	float cyanLight[4] = {0.f, 1.f, 1.f, 1.f};
	float cLightPos[4] = {1.f, 25.f, 0.f, 1.f};
	float cLightDir[4] = {1.f, 0.f, 0.f, 0.f};
	sceneLights.emplace_back(LightType::SPOTLIGHT, cyanLight);
	sceneLights.back().setPosition(cLightPos).setDirection(cLightDir).setDiffuse(2.f).setAttenuation(1.f, 0.f, 0.0005f).createObject(renderer, sceneObjects);

	// === ORIGIN MARKER === //
	float origin[] = {0.f, 0.f, 0.f, 1.f};
	sceneLights.emplace_back(LightType::POINTLIGHT, whiteLight);
	sceneLights.back().setPosition(origin).setDebug().setAmbient(0.f).setDiffuse(0.f).createObject(renderer, sceneObjects);

	float axisXdir[] = {-1.f, 0.f, 0.f, 0.f};
	sceneLights.emplace_back(LightType::SPOTLIGHT, redLight);
	sceneLights.back().setDebug().setPosition(origin).setDirection(axisXdir).setAmbient(0.f).setDiffuse(0.f).createObject(renderer, sceneObjects);

	float greenLight[] = {0.f, 1.f, 0.f, 1.f};
	float axisYdir[] = {0.f, -1.f, 0.f, 0.f};
	sceneLights.emplace_back(LightType::SPOTLIGHT, greenLight);
	sceneLights.back().setDebug().setPosition(origin).setDirection(axisYdir).setAmbient(0.f).setDiffuse(0.f).createObject(renderer, sceneObjects);

	float axisZdir[] = {0.f, 0.f, -1.f, 0.f};
	sceneLights.emplace_back(LightType::SPOTLIGHT, blueLight);
	sceneLights.back().setDebug().setPosition(origin).setDirection(axisZdir).setAmbient(0.f).setDiffuse(0.f).createObject(renderer, sceneObjects);

	// The truetypeInit creates a texture object in TexObjArray for storing the fontAtlasTexture
	GLOBAL.fontLoaded = renderer.truetypeInit(FILEPATH.Font_File);
	if (!GLOBAL.fontLoaded)
		std::cerr << "Fonts not loaded\n";
	else
		std::cerr << "Fonts loaded\n";

	printf("\nNumber of Texture Objects is %d\n\n", renderer.TexObjArray.getNumTextureObjects());

	// Collision System
	collisionSystem.setDebugCubeMesh(cubeID);
}

// ------------------------------------------------------------
//
// Main function
//

int main(int argc, char **argv)
{
	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_MULTISAMPLE);

	glutInitContextVersion(4, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(GLOBAL.WinX, GLOBAL.WinY);
	GLOBAL.WindowHandle = glutCreateWindow(GLOBAL.WinTitle);
	GLOBAL.lastTime = glutGet(GLUT_ELAPSED_TIME);

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

	// Enable stencil testing for rear-view mirror
	glClearStencil(0x0);
	glEnable(GL_STENCIL_TEST);

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

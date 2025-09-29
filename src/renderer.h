//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
//
// Author: João Madeiras Pereira
//

#pragma once
#include <vector>
#include <unordered_map>
#include "texture.h"
#include "model.h"
#include "stb_truetype.h"

struct dataMesh
{
	int meshID = 0;			  // mesh ID in the myMeshes array
	float *pvm, *vm, *normal; // matrices pointers
	int texMode = 0;		  // type of shading-> 0:no texturing; 1:modulate diffuse color with texel color; 2:diffuse color is replaced by texel color; 3: multitexturing
};

enum class Align
{
	Left,
	Center,
	Right,
	Top = Right,
	Bottom = Left,
};

struct TextCommand
{
	std::string str{};
	float position[2]; // screen coordinates
	float size = 1.f;
	float color[4] = {1.f, 1.f, 1.f, 1.f};
	float *pvm = NULL;
	Align align_x = Align::Center, align_y = Align::Center;
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool truetypeInit(const std::string &ttf_filepath); // Initialization of TRUETYPE  for text rendering

	// Setup render meshes GLSL program
	bool setRenderMeshesShaderProg(const std::string &vertShaderPath, const std::string &fragShaderPath);

	// setup text font rasterizer GLSL program
	bool setRenderTextShaderProg(const std::string &vertShaderPath, const std::string &fragShaderPath);

	bool setSkyboxShaderProg(const std::string &vertShaderPath, const std::string &fragShaderPath);

	void activateRenderMeshesShaderProg();

	void activateSkyboxShaderProg(float*, unsigned int, float*);

	void renderMesh(const dataMesh &data);

	void renderText(const TextCommand &text);

	void resetLights();

	void setFogColor(float *color);

	void setDirectionalLight(float *color, float ambient, float diffuse, float *direction);

	void setPointLight(float *color, float ambient, float diffuse, float *position, float constant, float linear, float exponential);

	void setSpotLight(float *color, float ambient, float diffuse, float *direction, float cutoff, float *position, float constant, float linear, float exponential);

	void setTexUnit(int tuId, int texObjId);

	// Vector with meshes
	std::unordered_map<int, MyMesh> meshRegistry;
	int nextMeshID = 0;

	int addMesh(const MyMesh &mesh)
	{
		int id = nextMeshID++;
		meshRegistry[id] = mesh;
		return id;
	}

	MyMesh &getMesh(int id)
	{
		return meshRegistry.at(id);
	}

	// Vector with meshes
	std::vector<struct MyMesh> myMeshes;

	/// Object of class Texture that manage an array of Texture Objects
	Texture TexObjArray;

private:
	// Render meshes GLSL program
	GLuint program;

	// Text font rasterizer GLSL program
	GLuint textProgram;

	GLint pvm_loc, vm_loc, normal_loc, texMode_loc, fogColor_loc;
	GLint tex_loc[MAX_TEXTURES];

#define MAX_POINT_LIGHTS 6
#define MAX_SPOT_LIGHTS 2

	struct
	{
		GLuint color;
		GLuint ambient;
		GLuint diffuse;
		GLuint direction;
	} directionalLight_loc;
	GLuint directionalLightToggle_loc;

	struct
	{
		GLuint color;
		GLuint ambient;
		GLuint diffuse;
		GLuint position;
		GLuint attConstant;
		GLuint attLinear;
		GLuint attExp;
	} pointLight_loc[MAX_POINT_LIGHTS];
	GLuint pointLightNum_loc;
	int pointLightCount = 0;

	struct
	{
		GLuint color;
		GLuint ambient;
		GLuint diffuse;
		GLuint position;
		GLuint direction;
		GLuint cutoff;
		GLuint attConstant;
		GLuint attLinear;
		GLuint attExp;
	} spotLight_loc[MAX_SPOT_LIGHTS];
	GLuint spotLightNum_loc;
	int spotLightCount = 0;

	// renderer variables for skybox
	GLuint skyboxProgram, skyboxVAO, skyboxVBO;
	GLuint skyboxprojview_loc, cubemap_loc, fogColor_skyloc;

	// render font GLSL program variable locations and VAO
	GLint fontPvm_loc, textColor_loc;
	GLuint textVAO, textVBO[2];

	struct Font
	{
		float size;
		GLuint textureId; // font atlas texture object ID stored in TexObjArray
		stbtt_fontinfo info;
		stbtt_packedchar packedChars[96];
		stbtt_aligned_quad alignedQuads[96];
	} font{};
};

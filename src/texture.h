#ifndef _texture_
#define _texture_

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <vector>
#include <IL/il.h>
#include <IL/ilu.h>

class Texture
{

public:
	void texture2D_Loader(const char *strFileName, bool repeat = true);

	// Loader de uma textura apenas com um canal de cor
	void texture2D_Loader(int width, int height, const uint8_t *data);

	void textureCubeMap_Loader(const char **strFileName);
	GLuint getTextureId(unsigned int id);
	unsigned int getNumTextureObjects();
	~Texture();

private:
	std::vector<GLuint> textureArray;
};

#endif
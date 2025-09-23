/** ----------------------------------------------------------
 * * \class Shader
 * Based on Shader - Very Simple Shader Library from Lighthouse3D
 *
 * This requires:
 *
 * GLEW (http://glew.sourceforge.net/)
 *
 ---------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "shader.h"

// pre conditions are established with asserts
// if having errors using the lib switch to Debug mode
#include <assert.h>

GLenum
Shader::spGLShaderTypes[COUNT_SHADER_TYPE] = {
								GL_VERTEX_SHADER,
								GL_GEOMETRY_SHADER,
								GL_TESS_CONTROL_SHADER,
								GL_TESS_EVALUATION_SHADER,
								GL_FRAGMENT_SHADER };

std::string
Shader::spStringShaderTypes[COUNT_SHADER_TYPE] = {
								"Vertex Shader",
								"Geometry Shader",
								"Tesselation Control Shader",
								"Tesselation Evaluation Shader",
								"Fragment Shader" };

Shader::Shader() : pProgram(0), pInited(false)
{
	for (int i = 0; i < COUNT_SHADER_TYPE; ++i) {
		pShader[i] = 0;
	}
}

Shader::~Shader()
{
	for (int i = 0; i < COUNT_SHADER_TYPE; ++i) {
		if (pShader[i])
			glDeleteShader(pShader[i]);
	}
}

void 
Shader::init()
{
	pInited = true;
	pProgram = glCreateProgram();
}


void 
Shader::compileShader(Shader::ShaderType st, std::string fileName)
{
	// init should always be called first
	assert(pInited == true);

	char *s = NULL;

	s = textFileRead(fileName);

	if (s != NULL) {
		const char * ss = s;

		pShader[st] = glCreateShader(spGLShaderTypes[st]);
		glShaderSource(pShader[st], 1, &ss,NULL);
		glAttachShader(pProgram, pShader[st]);
		glCompileShader(pShader[st]);

		free(s);
	}

	GLint success= 0;
	glGetShaderiv(pShader[st], GL_COMPILE_STATUS, &success);
	if (!success) {
		char info_log[1024];
		glGetShaderInfoLog(pShader[st], 1024, nullptr, info_log);
		printf("Failed to compile shader! Info Log: %s\n", info_log);
		exit(0);
	}
}


GLuint
Shader::getProgramIndex() { return pProgram; }


GLuint
Shader::getShaderIndex(Shader::ShaderType aType)
{
	return pShader[aType];
}

std::string
Shader::getShaderInfoLog(Shader::ShaderType st)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	if (pShader[st]) {
		glGetShaderiv(pShader[st], GL_INFO_LOG_LENGTH,&infologLength);

		if (infologLength > 0)
		{
			infoLog = (char *)malloc(infologLength);
			glGetShaderInfoLog(pShader[st], infologLength, &charsWritten, infoLog);
			if (charsWritten)  pResult = infoLog;
			
			free(infoLog);
		}
		else
			pResult = "Shader OK";
	}
	else
		pResult = "Shader not loaded";
	return pResult;
}


std::string
Shader::getProgramInfoLog()
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	pResult = "Program created";

	if (pProgram) {

		glGetProgramiv(pProgram, GL_INFO_LOG_LENGTH, &infologLength);

		if (infologLength > 0)
		{
			infoLog = (char *)malloc(infologLength);
			glGetProgramInfoLog(pProgram, infologLength, &charsWritten, infoLog);
			pResult = infoLog;
			if (charsWritten)
				pResult = infoLog;
			free(infoLog);
		}
	}
	return pResult;
}


bool
Shader::isProgramValid()
{
	GLint b = GL_FALSE;
	if (pProgram) {
		glValidateProgram(pProgram);
		glGetProgramiv(pProgram, GL_VALIDATE_STATUS,&b);
	}
	return (b != GL_FALSE);
}

bool
Shader::isProgramLinked()
{
	GLint b = GL_FALSE;
	if (pProgram) 
		glGetProgramiv(pProgram, GL_LINK_STATUS, &b);
	
	return (b != GL_FALSE);
}


std::string 
Shader::getAllInfoLogs()
{
	std::string s;

	for (int i = 0; i < COUNT_SHADER_TYPE; ++i) {
		if (pShader[i]) {
			getShaderInfoLog((Shader::ShaderType)i);
			s += Shader::spStringShaderTypes[i] + ": " + pResult + "\n";
		}
	}

	if (pProgram) {
		getProgramInfoLog();
		s += "Program: " + pResult;

		if (isProgramLinked())
			s += " and sucessfully linked.\n";
		else
			s += " and not linked\n"; 
	}
	else
		pResult = "Program not created.";
	pResult = s;
	return pResult;
}


// PRIVATE METHODS
char *
Shader::textFileRead(std::string fileName)
{
	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fileName != "") {
		fp = fopen(fileName.c_str(),"rt");

		if (fp != NULL) {
      
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
		else {
			printf("error opening file %s\n", fileName.c_str());
		}
	}
	return content;
}

/** ----------------------------------------------------------
 * \class Shader
 * Based on Shader - Very Simple Shader Library from Lighthouse3D
 *
 * This requires:
 *
 * GLEW (http://glew.sourceforge.net/)
 *
 ---------------------------------------------------------------*/

#ifndef _shader_
#define _shader_

#include <string>
#include <vector>
#include <map>
#include <GL/glew.h>

class Shader
{
	public:
	
	/// Types of Vertex Attributes
	enum AttribType {
		VERTEX_COORD_ATTRIB,
		NORMAL_ATTRIB,
		TEXTURE_COORD_ATTRIB,
		TANGENT_ATTRIB,
		BITANGENT_ATTRIB,
		VERTEX_ATTRIB1,
		VERTEX_ATTRIB2,
		VERTEX_ATTRIB3,
		VERTEX_ATTRIB4
	};

	/// Types of Shaders
	enum ShaderType {
		VERTEX_SHADER,
		GEOMETRY_SHADER,
		TESS_CONTROL_SHADER,
		TESS_EVAL_SHADER,
		FRAGMENT_SHADER,
		COUNT_SHADER_TYPE
	};


	Shader();
	~Shader();

	/** Init should be called for every shader instance
	  * prior to any other function
	*/
	void init();
	
	/** Loads the text in the file to the source of the specified shader 
	  *
	  * \param st one of the enum values of ShaderType
	  *	\param filename the file where the source is to be found
	*/
	void compileShader(Shader::ShaderType st, std::string fileName);

/// returns the program index
	GLuint getProgramIndex();
	/// returns a shader index
	GLuint getShaderIndex(Shader::ShaderType);
	/// returns a string with a shader's infolog
	std::string getShaderInfoLog(Shader::ShaderType);
	/// returns a string with the program's infolog
	std::string getProgramInfoLog();
	/// returns a string will all info logs
	std::string getAllInfoLogs();
	/// returns GL_VALIDATE_STATUS for the program 
	bool isProgramValid();
	/// returns true if linked, false otherwise
	bool isProgramLinked();


protected:
	
	/// stores if init has been called
	bool pInited;

	/// stores the OpenGL shader types
	static GLenum spGLShaderTypes[COUNT_SHADER_TYPE];

	/// stores the text string related to each type
	static std::string spStringShaderTypes[COUNT_SHADER_TYPE];

	/// aux string used to return the shaders infologs
	std::string pResult;

	/// stores the shaders and program indices
	GLuint pShader[COUNT_SHADER_TYPE];
	GLuint  pProgram;

	/// aux function to read the shader's source code from file
	char *textFileRead(std::string fileName);
};

#endif

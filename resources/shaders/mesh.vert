#version 330 core

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;


in vec4 position;
in vec4 normal;
in vec4 texCoord;

out Data {
	vec3 normal;
	vec3 position;
	vec2 texCoord;
} DataOut;

void main ()
{
	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.position = vec3(m_viewModel * position);
	DataOut.texCoord = texCoord.st;

	gl_Position = m_pvm * position;	
}

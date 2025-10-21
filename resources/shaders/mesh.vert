#version 330 core

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

in vec4 position;
in vec4 normal;
in vec4 texCoord;
in vec4 tangent;

out Data {
	vec3 normal;
	vec3 position;
	vec2 texCoord;
	mat3 m_tbn;
} DataOut;

void main ()
{
	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.position = vec3(m_viewModel * position);
	DataOut.texCoord = texCoord.st;
	
	vec3 n = normalize(m_normal * normal.xyz);
	vec3 t = normalize(m_normal * tangent.xyz);
	// Gram-Schmidt process
    t = normalize(t - dot(t, n) * n);
    vec3 b = cross(n, t);
    DataOut.m_tbn = mat3(t, b, n);

	gl_Position = m_pvm * position;	
}

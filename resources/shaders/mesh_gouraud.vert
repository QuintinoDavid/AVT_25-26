#version 330 core

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 l_pos;

uniform int texMode;
uniform bool spotlight_mode;
uniform vec4 coneDir;
uniform float spotCosCutOff;

uniform Materials mat;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria
in vec4 texCoord;

out Data {
	vec4 color;
} DataOut;

void main () {
	vec4 pos = m_viewModel * position;

    vec3 n = normalize(m_normal * normal.xyz);
    vec3 l = normalize(vec3(l_pos - pos));
    vec3 e = normalize(-vec3(pos));
    vec3 sd = normalize(coneDir.xyz);

    float intensity = 0.0;
    vec4 spec = vec4(0.0);
    float spotExp = 60.0;
    float att = 0.0;

    if (spotlight_mode) {
        float spotCos = dot(-l, sd);
        if (spotCos > spotCosCutOff) {
            att = pow(spotCos, spotExp);
            intensity = max(dot(n, l), 0.0) * att;
            if (intensity > 0.0) {
                vec3 h = normalize(l + e);
                float intSpec = max(dot(h, n), 0.0);
                spec = mat.specular * pow(intSpec, mat.shininess) * att;
            }
        }
    } else {
        intensity = max(dot(n, l), 0.0);
        if (intensity > 0.0) {
            vec3 h = normalize(l + e);
            float intSpec = max(dot(h, n), 0.0);
            spec = mat.specular * pow(intSpec, mat.shininess);
        }
    }

    DataOut.color = max(intensity * mat.diffuse + spec, mat.ambient); // computed vertex color

    gl_Position = m_pvm * position;
}
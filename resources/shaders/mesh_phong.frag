#version 430

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
} DataIn;

uniform Materials mat;

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;

uniform int texMode;
uniform bool spotlight_mode;
uniform vec4 coneDir;
uniform float spotCosCutOff;

out vec4 colorOut;

void main() {
	vec4 texel, texel1;

	vec4 spec = vec4(0.0);
	float intensity = 0.0f;
	float intSpec = 0.0f;

	float att = 0.0;
	float spotExp = 60.0;

	vec3 n = normalize(DataIn.normal);
	vec3 l = normalize(DataIn.lightDir);
	vec3 e = normalize(DataIn.eye);
	vec3 sd = normalize(coneDir.xyz);

	if(spotlight_mode == true)  {  //Scene iluminated by a spotlight
		float spotCos = dot(-l, sd);
		if(spotCos > spotCosCutOff)  {	//inside cone?
			att = pow(spotCos, spotExp);
			intensity = max(dot(n,l), 0.0) * att;
			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				intSpec = max(dot(h,n), 0.0);
				spec = mat.specular * pow(intSpec, mat.shininess) * att;
			}
		}
	}
	else {				//Scene iluminated by a pointlight
		intensity = max(dot(n,l), 0.0);
		if (intensity > 0.0) {
			vec3 h = normalize(l + e);
			intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}
	}

	if(texMode == 0) //no texturing
		colorOut = vec4(max(intensity * mat.diffuse + spec, mat.ambient).rgb, 1.0);

	else if(texMode == 1) // modulate diffuse color with texel color
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from lighwood.tga
		colorOut = vec4(max(intensity * mat.diffuse * texel + spec,0.07 * texel).rgb, 1.0);
	}
	else if (texMode == 2) // diffuse color is replaced by texel color
	{
		texel = texture(texmap, DataIn.tex_coord);  // texel from stone.tga
		colorOut = vec4(max(intensity*texel + spec, 0.07*texel).rgb, 1.0);
	}
	else // multitexturing
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from lighwood.tga
		texel1 = texture(texmap1, DataIn.tex_coord);  // texel from checker.tga
		colorOut = vec4(max(intensity*texel*texel1 + spec, 0.07*texel*texel1).rgb, 1.0);
	}
}
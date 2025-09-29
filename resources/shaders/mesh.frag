#version 330 core

in Data {
	vec3 normal;
	vec3 position;
	vec2 texCoord;
} DataIn;

out vec4 colorOut;

struct Light
{
    vec4 color;
    float ambientIntensity;
    float diffuseIntensity;
};

struct DirectionalLight
{
    Light base;
    vec4 direction;
};

struct PointLight
{
    Light base;
    vec4 position;

    float constant;
    float linear;
    float exponential;
};

struct SpotLight
{
    PointLight base;
    vec4 direction;
    float cutoff;
};

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
};

uniform Materials mat;
uniform int texMode;

uniform sampler2D texmap_stone;
uniform sampler2D texmap_grass;
uniform sampler2D texmap_window;
uniform sampler2D texmap_bbgrass;
uniform sampler2D texmap_lightwood;

const int MAX_POINT_LIGHTS = 6;
const int MAX_SPOT_LIGHTS = 2;

uniform DirectionalLight directionalLight;
uniform int directionalLightToggle;
uniform PointLight pointLightArray[MAX_POINT_LIGHTS];
uniform int pointLightNum;
uniform SpotLight spotLightArray[MAX_SPOT_LIGHTS];
uniform int spotLightNum;

uniform vec4 fogColor = vec4(0.f);

vec4 CalcLight(Light light, vec3 lightDirection, vec3 normal)
{
    vec4 ambient  = light.color * light.ambientIntensity * mat.ambient;
    vec4 diffuse  = vec4(0);
    vec4 specular = vec4(0);

    float intensityDiffuse = dot(normal, -lightDirection);
    if (intensityDiffuse > 0.f) {
        diffuse = light.color * light.diffuseIntensity * mat.diffuse * intensityDiffuse;

        vec3 viewDir = normalize(-DataIn.position);
        vec3 halfway = normalize(-lightDirection + viewDir);
        float intensitySpecular = dot(halfway, normal);
        if (intensitySpecular > 0.f) {
            specular = light.color * light.diffuseIntensity * mat.specular * pow(intensitySpecular, mat.shininess);
        }
    }

    return ambient + diffuse + specular;
}

vec4 CalcDirectionalLight(vec3 normal)
{
    vec3 direction = vec3(normalize(directionalLight.direction));
    return CalcLight(directionalLight.base, direction, normal);
}

vec4 CalcPointLight(PointLight light, vec3 normal)
{
    vec3 lightDirection = DataIn.position - vec3(light.position);
    float distance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    vec4 color = CalcLight(light.base, lightDirection, normal);
    float attenuation = light.constant + light.linear * distance + light.exponential * distance * distance;

    return color / attenuation;
}

vec4 CalcSpotLight(SpotLight light, vec3 normal)
{
    vec3 direction = vec3(normalize(light.direction));
    vec3 lightDirection = normalize(DataIn.position - vec3(light.base.position));
    float spotlightAngle = dot(lightDirection, direction);

    if (spotlightAngle > light.cutoff) {
        vec4 color = CalcPointLight(light.base, normal);
        float intensitySpotlight = 1.f - ((1.f - spotlightAngle) / (1.f - light.cutoff));
        return color * intensitySpotlight;
    }
    
    return vec4(0);
}

float CalcFogFactor()
{
    float fogEnd = 500.f;
    float fogDensity = .5f;

    float distance = length(DataIn.position);
    float distRatio = 4.f * distance / fogEnd;
    return exp(-distRatio * distRatio * fogDensity * fogDensity);
}

void main()
{
    vec3 normal = normalize(DataIn.normal);
    vec4 lightTotal = mat.emissive;

    lightTotal += directionalLightToggle * CalcDirectionalLight(normal);

    for (int i = 0; i < pointLightNum; i++) {
        lightTotal += CalcPointLight(pointLightArray[i], normal);
    }

    for (int i = 0; i < spotLightNum; i++) {
        lightTotal += CalcSpotLight(spotLightArray[i], normal);
    }

    if (texMode == 0) {
        // no texture
        colorOut = lightTotal;
    } else if (texMode == 1) {
        // tiled grass
        float tilingFactor1 = 23.f;
        float tilingFactor2 = 121.f;

        vec4 texel1 = texture(texmap_grass, DataIn.texCoord * tilingFactor1);
        vec4 texel2 = texture(texmap_grass, DataIn.texCoord * tilingFactor2);
        colorOut = mix(texel1, texel2, 0.5f) * lightTotal;
    } else if (texMode == 2) {
        // texel only, use stone.tga
        colorOut = texture(texmap_stone, DataIn.texCoord) * lightTotal;
    } else if (texMode == 3) {
        // window texture
        colorOut = texture(texmap_window, DataIn.texCoord) * vec4(lightTotal.xyz, 1.f);
    } else if (texMode == 4) {
        // billboard grass texture
        vec4 texel = texture(texmap_bbgrass, DataIn.texCoord);
        if (texel.a < 0.1f) discard;
        colorOut = texel * lightTotal;
    }  else if (texMode == 5) {
        // lightwood texture
        colorOut = texture(texmap_lightwood, DataIn.texCoord) * lightTotal;
    } 

    if (fogColor != vec4(0)) {
        colorOut = mix(fogColor, colorOut, CalcFogFactor());
    }
}

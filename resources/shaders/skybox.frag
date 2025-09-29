#version 330 core
in vec3 TexCoords;

out vec4 colorOut;

uniform vec4 fogColor;
uniform samplerCube skybox;

void main()
{
    colorOut = texture(skybox, TexCoords);

    if (fogColor != vec4(0)) {
        float height = normalize(TexCoords).y;
        float distRatio = 4.f * (height * height);
        float fogDensity = .5f;
        float fogFactor = (1 - height) * exp(-distRatio * fogDensity * fogDensity);

        colorOut = mix(colorOut, fogColor, fogFactor);
    }
}

#version 430

in vec2 TexCoords;
out vec4 color;

uniform sampler2D fontAtlasTexture;
uniform vec4 textColor;
void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(fontAtlasTexture, TexCoords).r);
    color = textColor * sampled;
}

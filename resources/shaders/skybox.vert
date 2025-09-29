#version 330 core
layout (location = 0) in vec3 position;

out vec3 TexCoords;

uniform mat4 projview;

void main()
{
    TexCoords = position.xyz;
    vec4 pos = projview * vec4(position, 1.f);
    gl_Position = pos.xyww;
}

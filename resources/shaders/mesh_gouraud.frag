#version 330 core

in Data {
	vec4 color;
} DataIn;

out vec4 colorOut;

void main() {
	colorOut = DataIn.color;
}
#include "sceneObject.h"
#include <iostream>

SceneObject::SceneObject(const std::vector<int> &meshes, int texMode_)
	: meshID(meshes), texMode(texMode_), collider(this) {}

void SceneObject::handleKeyInput(int) {}
void SceneObject::handleSpecialKeyInput(int) {}
void SceneObject::handleKeyRelease(int) {}
void SceneObject::handleSpecialKeyRelease(int) {}
void SceneObject::update(float) {}

void SceneObject::render(Renderer &renderer, gmu &mu)
{
	if (!active)
		return;

	mu.pushMatrix(gmu::MODEL);
	if (renderer.renderInverted()) {
		mu.translate(gmu::MODEL, pos[0], -pos[1], pos[2]);
	}
	else if (renderer.renderShadow())
	{
		float mat[16];
		float floor[4] = { 0,1,0,0 };
		float sunPos[4] = { 1000,1000,0.1,0 };

		mu.shadow_matrix(mat, floor, sunPos);
		mu.multMatrix(gmu::MODEL, mat);
		mu.translate(gmu::MODEL, pos[0], pos[1], pos[2]);
	}
	else {
		mu.translate(gmu::MODEL, pos[0], pos[1], pos[2]);
	}


	mu.rotate(gmu::MODEL, yaw, 0.0f, 1.0f, 0.0f);
	mu.rotate(gmu::MODEL, pitch, 1.0f, 0.0f, 0.0f);
	mu.rotate(gmu::MODEL, roll, 0.0f, 0.0f, 1.0f);
	if (renderer.renderInverted()) {
		mu.scale(gmu::MODEL, scale[0], -scale[1], scale[2]);
	}
	else {
		mu.scale(gmu::MODEL, scale[0], scale[1], scale[2]);
	}

	mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL);
	mu.computeNormalMatrix3x3();

	for (int mID : meshID)
	{
		dataMesh data;
		data.meshID = mID;
		data.texMode = texMode;
		if (renderer.renderShadow() && texMode != 5) {
			data.texMode = 0;
		}
		else if (renderer.renderShadow()) {
			data.texMode = 14; // billboard shadow
		}
		data.vm = mu.get(gmu::VIEW_MODEL);
		data.pvm = mu.get(gmu::PROJ_VIEW_MODEL);
		data.normal = mu.getNormalMatrix();

		renderer.renderMesh(data);
	}

	mu.popMatrix(gmu::MODEL);
}

void SceneObject::onCollision(Collider *other)
{
	(void)other;
	// std::cout << "SceneObject collided with another object!\n";
}

Collider *SceneObject::getCollider() { return &collider; }

float *SceneObject::getPosition()
{
	return pos;
}

float *SceneObject::getScale()
{
	return scale;
}

void SceneObject::setMeshes(const std::vector<int> &meshes)
{
	meshID = meshes;
}

void SceneObject::setPosition(float x, float y, float z)
{
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
}

void SceneObject::setRotation(float yaw_, float pitch_, float roll_)
{
	yaw = yaw_;
	pitch = pitch_;
	roll = roll_;
}

void SceneObject::setScale(float x, float y, float z)
{
	scale[0] = x;
	scale[1] = y;
	scale[2] = z;
}

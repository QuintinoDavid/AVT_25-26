#include "sceneObject.h"
#include <iostream>

SceneObject::SceneObject(const std::vector<int> &meshes, int texMode_)
	: meshID(meshes), texMode(texMode_), collider(this) {}

void SceneObject::handleKeyInput(int key) {}
void SceneObject::handleSpecialKeyInput(int key) {}
void SceneObject::handleKeyRelease(int key) {}
void SceneObject::handleSpecialKeyRelease(int key) {}
void SceneObject::update(float deltaTime) {}

void SceneObject::render(Renderer &renderer, gmu &mu)
{
	mu.pushMatrix(gmu::MODEL);
	mu.translate(gmu::MODEL, pos[0], pos[1], pos[2]);
	mu.rotate(gmu::MODEL, yaw, 0.0f, 1.0f, 0.0f);
	mu.rotate(gmu::MODEL, pitch, 1.0f, 0.0f, 0.0f);
	mu.rotate(gmu::MODEL, roll, 0.0f, 0.0f, 1.0f);
	mu.scale(gmu::MODEL, scale[0], scale[1], scale[2]);

	mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL);
	mu.computeNormalMatrix3x3();

	for (int mID : meshID)
	{
		dataMesh data;
		data.meshID = mID;
		data.texMode = texMode;
		data.vm = mu.get(gmu::VIEW_MODEL);
		data.pvm = mu.get(gmu::PROJ_VIEW_MODEL);
		data.normal = mu.getNormalMatrix();

		renderer.renderMesh(data);
	}

	mu.popMatrix(gmu::MODEL);
}

void SceneObject::onCollision(Collider *other)
{
	// std::cout << "SceneObject collided with another object!\n";
}

Collider *SceneObject::getCollider() { return &collider; }

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
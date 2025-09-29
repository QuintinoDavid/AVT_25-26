#pragma once
#include "collision.h"
#include "renderer.h"
#include "mathUtility.h"
#include <vector>
#include <iostream>

enum TexMode {
	TEXTURE_NONE, // ambient + diffuse + light colors only
	TEXTURE_FLOOR,
	TEXTURE_STONE,
	TEXTURE_WINDOW,
	TEXTURE_BBGRASS,
	TEXTURE_LIGHTWOOD
};

class SceneObject : public ICollidable
{
public:
	float pos[3] = {0.0f, 0.0f, 0.0f};
	float yaw = 0, pitch = 0, roll = 0;
	float scale[3] = {1.0f, 1.0f, 1.0f};
	std::vector<int> meshID;
	int texMode = 1;
	bool active = true;
	Collider collider;

public:
	SceneObject(const std::vector<int> &meshes, int texMode_ = 1);

	virtual void handleKeyInput(int key);
	virtual void handleSpecialKeyInput(int key);
	virtual void handleKeyRelease(int key);
	virtual void handleSpecialKeyRelease(int key);
	virtual void update(float deltaTime);
	virtual void render(Renderer &renderer, gmu &mu);

	void onCollision(Collider *other) override;

	Collider *getCollider();

	void setPosition(float x, float y, float z);
	void setRotation(float yaw_, float pitch_, float roll_);
	void setScale(float x, float y, float z);
	void toggle() { active = !active; }
};

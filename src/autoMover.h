#pragma once
#include "sceneObject.h"
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>
#include <iostream>

class Collider;

class AutoMover : public SceneObject
{
private:
	float dir[3] = {0.0f, 0.0f, 0.0f};
	float radius;
	float Speed;
	std::mt19937 gen{std::random_device{}()};
	std::uniform_real_distribution<float> dist_pos{-10.0f, 10.0f};
	std::uniform_real_distribution<float> one{-1.0f, 1.0f};

	void calcNewDir(float dir[3]);
	void calcNewPos(float pos[3]);
	void updateCollider();
	float calcDistFromXZ0();

public:
	AutoMover(const std::vector<int> &meshes, int texMode, float radius, float speed);
	virtual ~AutoMover();

	void update(float deltaTime) override;
	void onCollision(Collider *other) override;
};

#pragma once
#include "renderer.h"
#include "mathUtility.h"
#include <vector>
#include <iostream>

class ICollidable;
class Collider;
class CollisionSystem;

class ICollidable
{
public:
	virtual void onCollision(Collider *other) = 0;
};

class Collider
{
public:
	struct AABB
	{
		float min[3], max[3];
	};

private:
	AABB collisionBox;
	ICollidable *owner = nullptr;

public:
	Collider(ICollidable *ownerObj);
	void setBox(float minX, float minY, float minZ,
				float maxX, float maxY, float maxZ);
	const AABB &getBox() const;
	ICollidable *getOwner() const;
};

class CollisionSystem
{
private:
	int debugCubeMeshID = 2;
	std::vector<Collider *> colliders;

public:
	static CollisionSystem &getInstance();
	bool intersects(const Collider::AABB &a, const Collider::AABB &b);
	void setDebugCubeMesh(int meshID);
	void addCollider(Collider *c);
	void removeCollider(Collider *c);
	void checkCollisions();
	void showDebug(Renderer &renderer, gmu &mu);
};

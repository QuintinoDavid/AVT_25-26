#include "collision.h"

// --- Collider ---
Collider::Collider(ICollidable *ownerObj) : owner(ownerObj) {}

void Collider::setBox(float minX, float minY, float minZ,
					  float maxX, float maxY, float maxZ)
{
	collisionBox.min[0] = minX;
	collisionBox.min[1] = minY;
	collisionBox.min[2] = minZ;
	collisionBox.max[0] = maxX;
	collisionBox.max[1] = maxY;
	collisionBox.max[2] = maxZ;
}

const Collider::AABB &Collider::getBox() const { return collisionBox; }
ICollidable *Collider::getOwner() const { return owner; }

// --- CollisionSystem ---
bool CollisionSystem::intersects(const Collider::AABB &a, const Collider::AABB &b)
{
	return (a.min[0] <= b.max[0] && a.max[0] >= b.min[0] &&
			a.min[1] <= b.max[1] && a.max[1] >= b.min[1] &&
			a.min[2] <= b.max[2] && a.max[2] >= b.min[2]);
}

void CollisionSystem::setDebugCubeMesh(int meshID) { debugCubeMeshID = meshID; }

void CollisionSystem::addCollider(Collider *c) { colliders.push_back(c); }

void CollisionSystem::checkCollisions()
{
	for (size_t i = 0; i < colliders.size(); i++)
	{
		for (size_t j = i + 1; j < colliders.size(); j++)
		{
			if (intersects(colliders[i]->getBox(), colliders[j]->getBox()))
			{
				colliders[i]->getOwner()->onCollision(colliders[j]);
				colliders[j]->getOwner()->onCollision(colliders[i]);
			}
		}
	}
}

void CollisionSystem::showDebug(Renderer &renderer, gmu &mu)
{
	for (Collider *c : colliders)
	{
		const auto &box = c->getBox();

		float scaleX = box.max[0] - box.min[0];
		float scaleY = box.max[1] - box.min[1];
		float scaleZ = box.max[2] - box.min[2];
		float centerX = (box.min[0] + box.max[0]) * 0.5f;
		float centerY = (box.min[1] + box.max[1]) * 0.5f;
		float centerZ = (box.min[2] + box.max[2]) * 0.5f;

		mu.pushMatrix(gmu::MODEL);
		mu.loadIdentity(gmu::MODEL); // Reset transform
		mu.translate(gmu::MODEL,
					 centerX - scaleX * 0.5f,
					 centerY - scaleY * 0.5f,
					 centerZ - scaleZ * 0.5f);
		mu.scale(gmu::MODEL, scaleX, scaleY, scaleZ); // Scale to box size only

		mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL);
		mu.computeNormalMatrix3x3();

		// Enable wireframe
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		dataMesh data;
		data.meshID = debugCubeMeshID;
		data.texMode = 1;
		data.vm = mu.get(gmu::VIEW_MODEL);
		data.pvm = mu.get(gmu::PROJ_VIEW_MODEL);
		data.normal = mu.getNormalMatrix();

		renderer.renderMesh(data);

		// Restore fill mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		mu.popMatrix(gmu::MODEL);
	}
}

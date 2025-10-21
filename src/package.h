#pragma once
#include "sceneObject.h"
#include <functional>

class Drone; // forward declaration
class Collider;

class Package : public SceneObject
{
public:
	Package(const std::vector<int> &meshIDs = {}, int texMode_ = 1);
	virtual ~Package();

	std::function<void()> onDelivered;

	void update(float deltaTime) override;
	void pickUp(Drone *drone);
	void setDestination(SceneObject *dest, int destinationID);
	SceneObject *getDestination() const;
	void deliver();
	void reset(SceneObject *parent, float x, float y, float z);
	void updateCollider();
	void onCollision(Collider *collObj) override;

	// Getters
	bool getIsPickedUp() const { return isPickedUp; }
	bool getIsDelivered() const { return isDelivered; }

private:
	Drone *carrier = nullptr;
	SceneObject *destination = nullptr;
	bool isPickedUp = false;
	bool isDelivered = false;
	std::vector<int> previousDestinationMeshes;
	std::array<float, 3> previousDestinationPosition = {0.0f, 0.0f, 0.0f};
};

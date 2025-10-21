#include "package.h"
#include "drone.h"
#include <iostream>

Package::Package(const std::vector<int> &meshIDs, int texMode_)
	: SceneObject(meshIDs, texMode_) {}

Package::~Package() {}

void Package::update(float deltaTime)
{
	// If picked up, follow carrier
	if (isPickedUp && !isDelivered && carrier)
	{
		auto dpos = carrier->getPosition();
		setPosition(dpos[0] - 0.5f, dpos[1] - 2.0f, dpos[2] - 0.5f);

		updateCollider();
	}
}

void Package::pickUp(Drone *drone)
{
	if (!isPickedUp && !isDelivered)
	{
		isPickedUp = true;
		carrier = drone;
		// std::cout << "Package picked up!\n";
	}
}

void Package::setDestination(SceneObject *dest, int destinationID)
{
	// Restore old building mesh and position if any
	if (destination)
	{
		destination->setMeshes(previousDestinationMeshes);
		destination->setPosition(previousDestinationPosition[0], previousDestinationPosition[1], previousDestinationPosition[2]);
	}

	// Save new destination's current meshes and position
	previousDestinationMeshes = std::vector<int>(dest->meshID);
	auto pos = dest->getPosition();
	previousDestinationPosition[0] = pos[0];
	previousDestinationPosition[1] = pos[1];
	previousDestinationPosition[2] = pos[2];

	// Assign new delivery building
	destination = dest;
	destination->setMeshes({destinationID});
}

SceneObject *Package::getDestination() const
{
	return destination;
}

void Package::deliver()
{
	isDelivered = true;
	isPickedUp = false;
	carrier->addScore(carrier->getBatteryLevel());
	carrier->setBatteryLevel(Drone::MAX_BATTERY);
	carrier = nullptr;
	// std::cout << "Package delivered successfully!\n";
	if (onDelivered)
		onDelivered();
}

void Package::reset(SceneObject *newDest, float x, float y, float z)
{
	setPosition(x, y, z);
	isDelivered = false;
	isPickedUp = false;
	carrier = nullptr;
}

void Package::updateCollider()
{
	float size = 1.0f;
	collider.setBox(
		pos[0],		   // minX = mesh origin X
		pos[1],		   // minY = mesh origin Y
		pos[2],		   // minZ = mesh origin Z
		pos[0] + size, // maxX = origin + width
		pos[1] + size, // maxY = origin + height
		pos[2] + size  // maxZ = origin + depth
	);
}

// Trigger pickup on collision
void Package::onCollision(Collider *other)
{
	if (!isPickedUp && !isDelivered)
	{
		auto collObj = other->getOwner();
		if (auto dronePtr = dynamic_cast<Drone *>(collObj))
		{
			pickUp(dronePtr);
		}
	}
	if (isPickedUp && !isDelivered && destination)
	{
		if (other == destination->getCollider())
		{
			deliver();
		}
	}
}

#include "sceneObject.h"
#include "camera.cpp"
#include "mathUtility.h"
#include <cmath>
#include <algorithm>
#include <iostream>

class Drone : public SceneObject
{
private:
	Camera *cam = nullptr;

	// Drone physics state
	float velocity[3] = {0.0f, 0.0f, 0.0f}; // world-space velocity
	float verticalSpeed = 0.0f;				// current vertical speed
	float targetPitch = 0.0f;				// visual pitch (mesh tilt)
	float targetRoll = 0.0f;				// visual roll (mesh tilt)
	float currentYawSpeed = 0.0f;			// current yaw rotation speed

	// Input state
	bool keyW = false, keyS = false, keyA = false, keyD = false;
	bool keyUp = false, keyDown = false, keyLeft = false, keyRight = false;

	// CONSTANTS
	// Horizontal movement
	const float MAX_HORIZONTAL_SPEED = 10.0f;	 // max horizontal speed
	const float HORIZONTAL_ACCELERATION = 25.0f; // horizontal acceleration units/sec^2
	const float HORIZONTAL_DRAG = 5.0f;			 // drag slowing horizontal speed

	// Vertical movement
	const float MAX_VERTICAL_SPEED = 3.0f;	  // max vertical speed
	const float VERTICAL_ACCELERATION = 5.0f; // smooth vertical acceleration

	// Yaw rotation
	const float MAX_YAW_SPEED = 90.0f;	 // faster turning
	const float YAW_ACCELERATION = 5.0f; // ramp up quickly

	// Tilt / visual feedback
	const float MAX_TILT_ANGLE = 30.0f; // max pitch/roll tilt in degrees

	void updateDrone(float deltaTime)
	{
		// --- Smooth vertical acceleration ---
		float targetVerticalSpeed = 0.0f;
		if (keyW)
			targetVerticalSpeed += MAX_VERTICAL_SPEED;
		if (keyS)
			targetVerticalSpeed -= MAX_VERTICAL_SPEED;
		verticalSpeed += (targetVerticalSpeed - verticalSpeed) * VERTICAL_ACCELERATION * deltaTime;

		// --- Smooth yaw acceleration ---
		float yawInput = 0.0f;
		if (keyA)
			yawInput += 1.0f;
		if (keyD)
			yawInput -= 1.0f;

		float targetYawSpeed = yawInput * MAX_YAW_SPEED;

		currentYawSpeed += (targetYawSpeed - currentYawSpeed) * YAW_ACCELERATION * deltaTime;

		yaw += currentYawSpeed * deltaTime;

		// --- Horizontal acceleration based on pitch/roll keys ---
		float pitchInput = 0.0f;
		float rollInput = 0.0f;
		if (keyUp)
			pitchInput -= 1.0f;
		if (keyDown)
			pitchInput += 1.0f;
		if (keyLeft)
			rollInput -= 1.0f;
		if (keyRight)
			rollInput += 1.0f;

		float radYaw = DegToRad(yaw);
		float accelX = std::sin(radYaw) * pitchInput + std::cos(radYaw) * rollInput;
		float accelZ = std::cos(radYaw) * pitchInput - std::sin(radYaw) * rollInput;

		velocity[0] += accelX * HORIZONTAL_ACCELERATION * deltaTime;
		velocity[2] += accelZ * HORIZONTAL_ACCELERATION * deltaTime;

		// --- Apply horizontal drag ---
		if (accelX == 0.0f)
			velocity[0] -= velocity[0] * HORIZONTAL_DRAG * deltaTime;
		if (accelZ == 0.0f)
			velocity[2] -= velocity[2] * HORIZONTAL_DRAG * deltaTime;

		// --- Clamp horizontal speed ---
		float horizontalSpeed = std::sqrt(velocity[0] * velocity[0] + velocity[2] * velocity[2]);
		if (horizontalSpeed > MAX_HORIZONTAL_SPEED)
		{
			velocity[0] = velocity[0] / horizontalSpeed * MAX_HORIZONTAL_SPEED;
			velocity[2] = velocity[2] / horizontalSpeed * MAX_HORIZONTAL_SPEED;
		}

		// --- Update vertical velocity ---
		velocity[1] = verticalSpeed;

		// --- Update position ---
		pos[0] += velocity[0] * deltaTime;
		pos[1] += velocity[1] * deltaTime;
		pos[2] += velocity[2] * deltaTime;

		// --- Update mesh tilt based on horizontal velocity ---
		float localX = std::cos(radYaw) * velocity[0] - std::sin(radYaw) * velocity[2]; // right
		float localZ = std::sin(radYaw) * velocity[0] + std::cos(radYaw) * velocity[2]; // forward

		float desiredPitch = localZ / MAX_HORIZONTAL_SPEED * MAX_TILT_ANGLE;
		float desiredRoll = -localX / MAX_HORIZONTAL_SPEED * MAX_TILT_ANGLE;

		pitch = desiredPitch;
		roll = desiredRoll;
	}

	void updateCamera(float prevPos[3])
	{
		(void)prevPos;

		if (!cam)
			return;

		SphericalCoords sc = cam->getSpherical();

		// Convert spherical to Cartesian offset
		float alpha = sc.alpha * M_PI / 180.0f;
		float beta = sc.beta * M_PI / 180.0f;

		float x = sc.r * sin(alpha) * cos(beta);
		float y = sc.r * sin(beta);
		float z = sc.r * cos(alpha) * cos(beta);

		// Camera follows drone's corrected pos
		cam->setPosition(pos[0] + x, pos[1] + y, pos[2] + z);
		cam->setTarget(pos[0], pos[1], pos[2]);
	}

	void updateCollision()
	{
		float halfSizeX = 2.0f;
		float halfSizeY = 1.4f;
		float halfSizeZ = 2.0f;

		// Since we want the bottom of the box at drone position,
		// we offset the box vertically by half of its height
		collider.setBox(
			pos[0] - halfSizeX, pos[1], pos[2] - halfSizeZ,					  // min corner
			pos[0] + halfSizeX, pos[1] + halfSizeY * 2.0f, pos[2] + halfSizeZ // max corner
		);
	}

public:
	Drone(Camera *cam = nullptr, const std::vector<int> &meshIDs = {}, int texMode_ = 1)
		: SceneObject(meshIDs, texMode_), cam(cam) {}

	void update(float deltaTime) override
	{
		float prevPos[3] = {pos[0], pos[1], pos[2]};

		updateDrone(deltaTime);

		updateCollision();

		updateCamera(prevPos);

		// std::cout << "Drone Position: (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")\n";
		// std::cout << "Velocity: (" << velocity[0] << ", " << velocity[1] << ", " << velocity[2] << ")\n";
		// std::cout << "Pitch: " << pitch << ", Roll: " << roll << ", Yaw: " << yaw << "\n";
		// std::cout << "-------------------------\n";
	}

	// --- Collision handling ---
	void onCollision(Collider *other) override
	{
		// Get the collided object's AABB
		const auto &box = other->getBox();

		// Get drone AABB
		const auto &droneBox = collider.getBox();

		// Compute overlap on each axis
		float overlapX = std::min(droneBox.max[0], box.max[0]) - std::max(droneBox.min[0], box.min[0]);
		float overlapY = std::min(droneBox.max[1], box.max[1]) - std::max(droneBox.min[1], box.min[1]);
		float overlapZ = std::min(droneBox.max[2], box.max[2]) - std::max(droneBox.min[2], box.min[2]);

		// Find the smallest overlap axis (simplest way to resolve collision)
		if (overlapY < overlapX && overlapY < overlapZ)
		{
			// Drone hit from above or below, stop vertical movement
			if (velocity[1] < 0)
			{						 // falling
				pos[1] = box.max[1]; // place on top of floor
			}
			else
			{ // hitting ceiling
				pos[1] = box.min[1] - (droneBox.max[1] - droneBox.min[1]);
			}
			velocity[1] = 0;
		}
		else if (overlapX < overlapZ)
		{
			// Collision on X axis
			if (velocity[0] > 0)
				pos[0] -= overlapX;
			else
				pos[0] += overlapX;
			velocity[0] = 0;
		}
		else
		{
			// Collision on Z axis
			if (velocity[2] > 0)
				pos[2] -= overlapZ;
			else
				pos[2] += overlapZ;
			velocity[2] = 0;
		}
	}

	// --- Input handling ---
	void handleKeyInput(int key) override
	{
		switch (key)
		{
		case 'w':
			keyW = true;
			break;
		case 's':
			keyS = true;
			break;
		case 'a':
			keyA = true;
			break;
		case 'd':
			keyD = true;
			break;
		}
	}

	void handleSpecialKeyInput(int key) override
	{
		switch (key)
		{
		case 101:
			keyUp = true;
			break;
		case 103:
			keyDown = true;
			break;
		case 100:
			keyLeft = true;
			break;
		case 102:
			keyRight = true;
			break;
		}
	}

	void handleKeyRelease(int key) override
	{
		switch (key)
		{
		case 'w':
			keyW = false;
			break;
		case 's':
			keyS = false;
			break;
		case 'a':
			keyA = false;
			break;
		case 'd':
			keyD = false;
			break;
		}
	}

	void handleSpecialKeyRelease(int key) override
	{
		switch (key)
		{
		case 101:
			keyUp = false;
			break;
		case 103:
			keyDown = false;
			break;
		case 100:
			keyLeft = false;
			break;
		case 102:
			keyRight = false;
			break;
		}
	}
};

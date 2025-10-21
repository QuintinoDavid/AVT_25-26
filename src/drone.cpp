#include "drone.h"
#include "package.h"
#include "camera.h"
#include "light.h"
#include "autoMover.h"
#include "mathUtility.h"

void Drone::updateDrone(float deltaTime)
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

void Drone::updateBattery(float deltaTime)
{
	float throttleLevel = 0.0f;
	throttleLevel += (keyW || keyS) ? 0.3f : 0.0f;
	throttleLevel += (keyA || keyD) ? 0.2f : 0.0f;
	throttleLevel += (keyUp || keyDown || keyLeft || keyRight) ? 0.5f : 0.0f;

	if (throttleLevel <= 0.0f)
		return; // no drain if not moving

	float drainRate = BATTERY_DRAIN_MOVEMENT * throttleLevel;
	batteryLevel -= drainRate * deltaTime;
	batteryLevel = std::clamp(batteryLevel, 0.0f, MAX_BATTERY);
}

void Drone::updateCamera()
{
	if (!cam)
		return;

	SphericalCoords sc = cam->getSpherical();

	// Calculate what the new alpha should be (drone yaw + offset)
	float newAlpha = yaw + cameraAlphaOffset;

	// Convert spherical to Cartesian offset
	float alpha = newAlpha * PI_F / 180.0f;
	float beta = sc.beta * PI_F / 180.0f;

	float x = sc.r * sin(alpha) * cos(beta);
	float y = sc.r * sin(beta);
	float z = sc.r * cos(alpha) * cos(beta);

	// Camera follows drone's corrected pos
	cam->setPosition(pos[0] + x, pos[1] + y, pos[2] + z);
	cam->setTarget(pos[0], pos[1], pos[2]);

	// Update the spherical coordinates with the new alpha
	cam->setSpherical(newAlpha, sc.beta, sc.r);
}

void Drone::updateLights()
{
	float cosYaw = std::cos(yaw * PI_F / 180.0f);
	float yawDir = cosYaw / std::abs(cosYaw);
	float sinYaw = std::sin(yaw * PI_F / 180.0f);
	float cosPitch = std::cos(pitch * PI_F / 180.0f);
	float sinPitch = std::sin(pitch * PI_F / 180.0f);

	if (headlight_l != nullptr)
	{
		float x = -0.25f * cosYaw - 1.21f * sinYaw;
		float y = (-0.25f * sinYaw + 1.21f * cosYaw) * sinPitch;
		float z = (-0.25f * sinYaw + 1.21f * cosYaw) * cosPitch;
		float position[4] = {pos[0] + x, pos[1] + yawDir * y, pos[2] - z, 1.f};
		headlight_l->setRotation(yaw, pitch);
		headlight_l->setPosition(position);
	}
	if (headlight_r != nullptr)
	{
		float x = 0.25f * cosYaw - 1.21f * sinYaw;
		float y = (0.25f * yawDir * sinYaw + 1.21f * cosYaw) * sinPitch;
		float z = (0.25f * sinYaw + 1.21f * cosYaw) * cosPitch;
		float position[4] = {pos[0] + x, pos[1] + yawDir * y, pos[2] - z, 1.f};
		headlight_r->setRotation(yaw, pitch);
		headlight_r->setPosition(position);
	}
}

void Drone::update(float deltaTime)
{
	// Check if battery is depleted
	if (batteryLevel <= 0.0f)
	{
		disabled = true;
	}

	// Only update movement if not disabled
	if (!disabled)
	{
		updateDrone(deltaTime);
		updateBattery(deltaTime);
	}
	else
	{
		// Descend when disabled
		verticalSpeed -= FALL_SPEED * deltaTime;
		velocity[1] = verticalSpeed;
		pos[1] += velocity[1] * deltaTime;

		// Prevent sinking below ground
		const float groundY = 0.0f; // ground plane
		const float droneBottom = collider.getBox().min[1];
		if (droneBottom <= groundY)
		{
			pos[1] += (groundY - droneBottom);
			verticalSpeed = 0.0f;
			velocity[1] = 0.0f;
		}

		// Optional: slowly zero horizontal velocity
		velocity[0] *= 0.98f;
		velocity[2] *= 0.98f;

		// Keep drone level when disabled
		pitch = 0.0f;
		roll = 0.0f;
		currentYawSpeed = 0.0f;
	}

	updateCollision();
	updateCamera();
	updateLights();

	// std::cout << "Drone Battery: " << batteryLevel << "%\n";
	// if (disabled)
	//	std::cout << "Drone is disabled!\n";
	// std::cout << "Drone Position: (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")\n";
	// std::cout << "Velocity: (" << velocity[0] << ", " << velocity[1] << ", " << velocity[2] << ")\n";
	// std::cout << "Pitch: " << pitch << ", Roll: " << roll << ", Yaw: " << yaw << "\n";
	// std::cout << "-------------------------\n";
}

void Drone::updateCollision()
{
	float halfsizex = 1.4f; // half-width
	float halfsizez = 1.8f; // half-depth
	float halfsizey = 0.6f; // half-height

	float radYaw = DegToRad(yaw);
	float cosY = abs(cos(radYaw)), sinY = abs(sin(radYaw));

	float rotatedX = halfsizex * cosY + halfsizez * sinY; // width after yaw
	float rotatedZ = halfsizex * sinY + halfsizez * cosY; // depth after yaw

	collider.setBox(pos[0] - rotatedX, pos[1], pos[2] - rotatedZ,
					pos[0] + rotatedX, pos[1] + 1.2f, pos[2] + rotatedZ);

	// Remove colliders no longer overlapping
	for (auto it = activeCollisions.begin(); it != activeCollisions.end();)
	{
		Collider *c = *it;
		if (!CollisionSystem::getInstance().intersects(collider.getBox(), c->getBox()))
			it = activeCollisions.erase(it); // no longer colliding
		else
			++it;
	}
}

void Drone::addHeadlight(Light &light_left, Light &light_right)
{
	float position_l[4] = {pos[0] - 0.25f, pos[1], pos[2] - 1.21f, 1.f};
	light_left.setPosition(position_l);
	float position_r[4] = {pos[0] + 0.25f, pos[1], pos[2] - 1.21f, 1.f};
	light_right.setPosition(position_r);

	headlight_l = &light_left;
	headlight_r = &light_right;
}

void Drone::updateCameraOffset()
{
	if (!cam)
		return;
	SphericalCoords sc = cam->getSpherical();
	cameraAlphaOffset = sc.alpha - yaw;
}

void Drone::setBatteryLevel(float level)
{
	batteryLevel = std::clamp(level, 0.0f, MAX_BATTERY);
}

float Drone::getBatteryLevel() const
{
	return batteryLevel;
}

void Drone::addScore(float points)
{
	score += points;
}

void Drone::setScore(float points)
{
	score = points;
}

float Drone::getScore() const
{
	return score;
}

void Drone::enable()
{
	disabled = false;
}

void Drone::disable()
{
	disabled = true;
}

// --- Collision handling ---
void Drone::onCollision(Collider *other)
{
	// std::cout << "[Drone::onCollision] Collision detected with another object.\n";

	ICollidable *collidableOwner = other->getOwner();
	if (auto *autoMover = dynamic_cast<AutoMover *>(collidableOwner))
	{
		// reset drone position & velocity
		// pos[0] = 0.0f;
		// pos[1] = 5.0f; // start a bit above ground
		// pos[2] = 0.0f;
		// velocity[0] = velocity[1] = velocity[2] = 0.0f;
		// verticalSpeed = 0.0f;
		// currentYawSpeed = 0.0f;
		// pitch = roll = yaw = 0.0f;
		// return;
	}

	// Ignore battery penalty if colliding with a Package
	bool isPackage = dynamic_cast<Package *>(collidableOwner) != nullptr;

	// Get the collided object's AABB
	const auto &box = other->getBox();

	// Get drone AABB
	const auto &droneBox = collider.getBox();

	// Compute overlap on each axis
	float overlapX = std::max(0.0f, std::min(droneBox.max[0], box.max[0]) - std::max(droneBox.min[0], box.min[0]));
	float overlapY = std::max(0.0f, std::min(droneBox.max[1], box.max[1]) - std::max(droneBox.min[1], box.min[1]));
	float overlapZ = std::max(0.0f, std::min(droneBox.max[2], box.max[2]) - std::max(droneBox.min[2], box.min[2]));

	// Find the smallest overlap axis (simplest way to resolve collision)
	if (overlapX < overlapY && overlapX < overlapZ)
	{
		// X axis resolution
		if (velocity[0] > 0)
			pos[0] -= overlapX;
		else if (velocity[0] < 0)
			pos[0] += overlapX;
		velocity[0] = 0;
	}
	else if (overlapY < overlapZ)
	{
		// Y axis resolution
		if (velocity[1] > 0)
			pos[1] -= overlapY;
		else if (velocity[1] < 0)
			pos[1] += overlapY;
		velocity[1] = 0;
	}
	else
	{
		// Z axis resolution
		if (velocity[2] > 0)
			pos[2] -= overlapZ;
		else if (velocity[2] < 0)
			pos[2] += overlapZ;
		velocity[2] = 0;
	}

	// --- Battery penalty on collision ---
	if (!isPackage && activeCollisions.find(other) == activeCollisions.end())
	{
		batteryLevel -= MAX_BATTERY * 0.2f;
		batteryLevel = std::clamp(batteryLevel, 0.0f, MAX_BATTERY);
		activeCollisions.insert(other);
	}
}

// --- Input handling ---
void Drone::handleKeyInput(int key)
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

void Drone::handleSpecialKeyInput(int key)
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

void Drone::handleKeyRelease(int key)
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

void Drone::handleSpecialKeyRelease(int key)
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
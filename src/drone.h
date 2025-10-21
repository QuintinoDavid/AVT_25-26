#pragma once
#include "sceneObject.h"
#include <unordered_set>
#include <vector>

class Camera;
class Light;
class Collider;
class AutoMover;

class Drone : public SceneObject
{
public:
	static constexpr float MAX_BATTERY = 100.f;

	Drone(Camera *cam = nullptr, const std::vector<int> &meshIDs = {}, int texMode_ = 1)
		: SceneObject(meshIDs, texMode_), cam(cam), batteryLevel(MAX_BATTERY) {}

	void update(float deltaTime) override;
	void onCollision(Collider *other) override;

	void handleKeyInput(int key) override;
	void handleKeyRelease(int key) override;
	void handleSpecialKeyInput(int key) override;
	void handleSpecialKeyRelease(int key) override;

	void addHeadlight(Light &light_left, Light &light_right);
	void updateCameraOffset();

	void setBatteryLevel(float level);
	float getBatteryLevel() const;

	void addScore(float points);
	void setScore(float points);
	float getScore() const;

	void enable();
	void disable();
	bool isDisabled() const { return disabled; }

private:
	// Movement & physics
	void updateDrone(float deltaTime);
	void updateBattery(float deltaTime);
	void updateCamera();
	void updateLights();
	void updateCollision();

	float velocity[3] = {0.0f, 0.0f, 0.0f};
	float verticalSpeed = 0.0f;
	float targetPitch = 0.0f;
	float targetRoll = 0.0f;
	float currentYawSpeed = 0.0f;
	float batteryLevel;
	float score = 0.0f;

	bool disabled = false;

	Camera *cam;
	Light *headlight_l;
	Light *headlight_r;
	std::unordered_set<Collider *> activeCollisions;

	bool keyW = false, keyS = false, keyA = false, keyD = false;
	bool keyUp = false, keyDown = false, keyLeft = false, keyRight = false;
	float cameraAlphaOffset = 0.0f;

	// Constants
	static constexpr float BATTERY_DRAIN_IDLE = 0.1f;
	static constexpr float BATTERY_DRAIN_MOVEMENT = 2.f;
	static constexpr float MAX_HORIZONTAL_SPEED = 10.f;
	static constexpr float HORIZONTAL_ACCELERATION = 25.f;
	static constexpr float HORIZONTAL_DRAG = 5.f;
	static constexpr float MAX_VERTICAL_SPEED = 3.f;
	static constexpr float VERTICAL_ACCELERATION = 5.f;
	static constexpr float MAX_YAW_SPEED = 90.f;
	static constexpr float YAW_ACCELERATION = 5.f;
	static constexpr float MAX_TILT_ANGLE = 30.f;
	static constexpr float FALL_SPEED = 1.5f;
};

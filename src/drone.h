#include "camera.h"
#include "sceneObject.h"

class Drone : public SceneObject
{
private:
	Camera *cam = nullptr;

	float throttle = 0.0f;
	float velocity[3] = {0.0f, 0.0f, 0.0f}; // replace direction + speed combo

	// Target angles for smooth leveling
	float targetPitch = 0.0f;
	float targetRoll = 0.0f;

	// Physics parameters
	const float maxHorizontalSpeed = 2.0f;
	const float maxVerticalSpeed = 0.5f;
	const float acceleration = 0.02f;	  // horizontal
	const float drag = 0.95f;			  // horizontal
	const float throttleStrength = 0.01f; // vertical acceleration
	const float throttleDrag = 0.95f;	  // vertical drag
	const float angleChangeRate = 2.0f;	  // input rate
	const float levelingRate = 0.1f;	  // pitch/roll self-leveling

	void changeThrottle(float delta)
	{
		throttle = std::max(-1.0f, std::min(1.0f, throttle + delta));
	}

	void changeYaw(float delta) { yaw += delta; }
	void changePitch(float delta) { pitch += delta; }
	void changeRoll(float delta) { roll += delta; }

	void updateVelocity()
	{
		// --- Smoothly move pitch/roll toward target (leveling)
		pitch += (targetPitch - pitch) * levelingRate;
		roll += (targetRoll - roll) * levelingRate;

		// --- Horizontal acceleration from tilt
		float pitchRad = pitch * M_PI / 180.0f;
		float rollRad = roll * M_PI / 180.0f;
		float yawRad = yaw * M_PI / 180.0f;

		float ax = sin(rollRad) * acceleration;
		float az = -sin(pitchRad) * acceleration;

		// Rotate according to yaw
		float sinYaw = sin(yawRad);
		float cosYaw = cos(yawRad);
		float axWorld = ax * cosYaw - az * sinYaw;
		float azWorld = ax * sinYaw + az * cosYaw;

		velocity[0] += axWorld;
		velocity[2] += azWorld;

		// Apply horizontal drag
		velocity[0] *= drag;
		velocity[2] *= drag;

		// Limit horizontal speed
		float horizSpeed = sqrt(velocity[0] * velocity[0] + velocity[2] * velocity[2]);
		if (horizSpeed > maxHorizontalSpeed)
		{
			float scale = maxHorizontalSpeed / horizSpeed;
			velocity[0] *= scale;
			velocity[2] *= scale;
		}

		// --- Vertical movement from throttle
		velocity[1] += throttle * throttleStrength;
		velocity[1] *= throttleDrag;

		// Gradually reduce throttle toward 0 (if no input)
		throttle *= 0.9f;
		if (fabs(throttle) < 0.01f)
			throttle = 0.0f;

		// Clamp vertical speed
		if (velocity[1] > maxVerticalSpeed)
			velocity[1] = maxVerticalSpeed;
		if (velocity[1] < -maxVerticalSpeed)
			velocity[1] = -maxVerticalSpeed;
	}

public:
	Drone(Camera *cam = nullptr, int meshID_ = -1, int texMode_ = 1) : SceneObject(meshID_, texMode_), cam(cam) {}

	void update() override
	{
		updateVelocity();

		// Update position
		pos[0] += velocity[0];
		pos[1] += velocity[1];
		pos[2] += velocity[2];
	}

	void handleInput(int key) override
	{
		switch (key)
		{
		case 'w':
			changeThrottle(0.1f);
			break;
		case 's':
			changeThrottle(-0.1f);
			break;
		case 'a':
			changeYaw(-angleChangeRate);
			break;
		case 'd':
			changeYaw(angleChangeRate);
			break;
		}
	}

	// Handle arrow keys for pitch and roll
	void handleSpecialInput(int key) override
	{
		switch (key)
		{
		case 101: // Up arrow - pitch forward
			changePitch(-angleChangeRate);
			break;
		case 103: // Down arrow - pitch backward
			changePitch(angleChangeRate);
			break;
		case 100: // Left arrow - roll left
			changeRoll(-angleChangeRate);
			break;
		case 102: // Right arrow - roll right
			changeRoll(angleChangeRate);
			break;
		}
	}

	// Getter methods for debugging
	float getThrottle() const { return throttle; }
	float getVelocityX() const { return velocity[0]; }
	float getVelocityY() const { return velocity[1]; }
	float getVelocityZ() const { return velocity[2]; }
};

#include "camera.h"

// Position setters and getters
void Camera::setPosition(float x, float y, float z)
{
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
}
float Camera::getX() const { return pos[0]; }
float Camera::getY() const { return pos[1]; }
float Camera::getZ() const { return pos[2]; }
void Camera::setX(float x) { pos[0] = x; }
void Camera::setY(float y) { pos[1] = y; }
void Camera::setZ(float z) { pos[2] = z; }

// Target setters and getters
void Camera::setTarget(float x, float y, float z)
{
	target[0] = x;
	target[1] = y;
	target[2] = z;
}
float Camera::getTargetX() const { return target[0]; }
float Camera::getTargetY() const { return target[1]; }
float Camera::getTargetZ() const { return target[2]; }
void Camera::setTargetX(float x) { target[0] = x; }
void Camera::setTargetY(float y) { target[1] = y; }
void Camera::setTargetZ(float z) { target[2] = z; }

// Up setters and getters
void Camera::setUp(float x, float y, float z)
{
	up[0] = x;
	up[1] = y;
	up[2] = z;
}
float Camera::getUpX() const { return up[0]; }
float Camera::getUpY() const { return up[1]; }
float Camera::getUpZ() const { return up[2]; }
void Camera::setUpX(float x) { up[0] = x; }
void Camera::setUpY(float y) { up[1] = y; }
void Camera::setUpZ(float z) { up[2] = z; }

// Projection type setters and getters
void Camera::setProjectionType(ProjectionType projectionType)
{
	type = projectionType;
}

ProjectionType Camera::getProjectionType() const
{
	return type;
}

SphericalCoords Camera::getSpherical() const
{
	float dx = pos[0] - target[0];
	float dy = pos[1] - target[1];
	float dz = pos[2] - target[2];

	SphericalCoords s;
	s.r = std::sqrt(dx * dx + dy * dy + dz * dz);
	s.alpha = std::atan2(dx, dz) * 180.0f / PI_F; // azimuth
	s.beta = std::asin(dy / s.r) * 180.0f / PI_F; // elevation

	return s;
}

void Camera::setSpherical(float alphaDeg, float betaDeg, float r)
{
	// Convert to radians
	float alpha = alphaDeg * PI_F / 180.0f;
	float beta = betaDeg * PI_F / 180.0f;

	// Compute Cartesian coordinates relative to target
	float x = r * sin(alpha) * cos(beta);
	float y = r * sin(beta);
	float z = r * cos(alpha) * cos(beta);

	// Set position in world space
	pos[0] = target[0] + x;
	pos[1] = target[1] + y;
	pos[2] = target[2] + z;
}

std::string Camera::toString() const
{
	std::ostringstream oss;
	oss << "Camera {\n";
	oss << "  Position: (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")\n";
	oss << "  Target: (" << target[0] << ", " << target[1] << ", " << target[2] << ")\n";
	oss << "  Up: (" << up[0] << ", " << up[1] << ", " << up[2] << ")\n";
	oss << "  ProjectionType: " << (type == ProjectionType::Perspective ? "Perspective" : "Orthographic") << "\n";
	oss << "}";
	return oss.str();
}

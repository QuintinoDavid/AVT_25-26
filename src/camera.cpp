#include <cmath>
#include <sstream>
#include <string>

enum class ProjectionType
{
	Orthographic,
	Perspective
};

struct SphericalCoords
{
	float alpha;
	float beta;
	float r;
};

class Camera
{
private:
	float pos[3] = {0.0f, 0.0f, 0.0f};
	float target[3] = {0.0f, 0.0f, 0.0f};
	float up[3] = {0.0f, 1.0f, 0.0f}; // Added up vector
	ProjectionType type = ProjectionType::Perspective;

public:
	// Position setters and getters
	void setPosition(float x, float y, float z)
	{
		pos[0] = x;
		pos[1] = y;
		pos[2] = z;
	}
	float getX() const { return pos[0]; }
	float getY() const { return pos[1]; }
	float getZ() const { return pos[2]; }
	void setX(float x) { pos[0] = x; }
	void setY(float y) { pos[1] = y; }
	void setZ(float z) { pos[2] = z; }

	// Target setters and getters
	void setTarget(float x, float y, float z)
	{
		target[0] = x;
		target[1] = y;
		target[2] = z;
	}
	float getTargetX() const { return target[0]; }
	float getTargetY() const { return target[1]; }
	float getTargetZ() const { return target[2]; }
	void setTargetX(float x) { target[0] = x; }
	void setTargetY(float y) { target[1] = y; }
	void setTargetZ(float z) { target[2] = z; }

	// Up setters and getters
	void setUp(float x, float y, float z)
	{
		up[0] = x;
		up[1] = y;
		up[2] = z;
	}
	float getUpX() const { return up[0]; }
	float getUpY() const { return up[1]; }
	float getUpZ() const { return up[2]; }
	void setUpX(float x) { up[0] = x; }
	void setUpY(float y) { up[1] = y; }
	void setUpZ(float z) { up[2] = z; }

	// Projection type setters and getters
	void setProjectionType(ProjectionType projectionType)
	{
		type = projectionType;
	}

	ProjectionType getProjectionType() const
	{
		return type;
	}

	SphericalCoords getSpherical() const
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

	void setSpherical(float alphaDeg, float betaDeg, float r)
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

	std::string toString() const
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
};

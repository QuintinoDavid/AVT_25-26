#pragma once
#include "mathUtility.h"
#include <cmath>
#include <string>
#include <sstream>

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
	float up[3] = {0.0f, 1.0f, 0.0f};
	ProjectionType type = ProjectionType::Perspective;

public:
	// Position
	void setPosition(float x, float y, float z);
	float getX() const;
	float getY() const;
	float getZ() const;
	void setX(float x);
	void setY(float y);
	void setZ(float z);

	// Target
	void setTarget(float x, float y, float z);
	float getTargetX() const;
	float getTargetY() const;
	float getTargetZ() const;
	void setTargetX(float x);
	void setTargetY(float y);
	void setTargetZ(float z);

	// Up vector
	void setUp(float x, float y, float z);
	float getUpX() const;
	float getUpY() const;
	float getUpZ() const;
	void setUpX(float x);
	void setUpY(float y);
	void setUpZ(float z);

	// Projection
	void setProjectionType(ProjectionType projectionType);
	ProjectionType getProjectionType() const;

	// Spherical coordinates
	SphericalCoords getSpherical() const;
	void setSpherical(float alphaDeg, float betaDeg, float r);

	// Debug string
	std::string toString() const;
};

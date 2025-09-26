#pragma once

#include "renderer.h"
#include "mathUtility.h"

class SceneObject
{
protected:
	float pos[3] = {0.0f, 0.0f, 0.0f};
	float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
	float scale[3] = {1.0f, 1.0f, 1.0f};
	int meshID = -1;
	int texMode = 1;

public:
	SceneObject(int meshID_ = -1, int texMode_ = 1) : meshID(meshID_), texMode(texMode_) {}

	virtual void handleSpecialInput(int) {} // called when a special key is pressed
	virtual void handleInput(int) {}		// called when a key is pressed
	virtual void update() {}					// Optional per-frame logic

	virtual void render(Renderer &renderer, gmu &mu)
	{
		mu.pushMatrix(gmu::MODEL);
		mu.translate(gmu::MODEL, pos[0], pos[1], pos[2]);
		mu.rotate(gmu::MODEL, yaw, 0.0f, 1.0f, 0.0f);
		mu.rotate(gmu::MODEL, pitch, 1.0f, 0.0f, 0.0f);
		mu.rotate(gmu::MODEL, roll, 0.0f, 0.0f, 1.0f);
		mu.scale(gmu::MODEL, scale[0], scale[1], scale[2]);

		mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL);
		mu.computeNormalMatrix3x3();

		dataMesh data;
		data.meshID = meshID;
		data.texMode = texMode;
		data.vm = mu.get(gmu::VIEW_MODEL);
		data.pvm = mu.get(gmu::PROJ_VIEW_MODEL);
		data.normal = mu.getNormalMatrix();

		renderer.renderMesh(data);

		mu.popMatrix(gmu::MODEL);
	}

	void setPosition(float x, float y, float z)
	{
		pos[0] = x;
		pos[1] = y;
		pos[2] = z;
	}
	void setRotation(float yaw_, float pitch_, float roll_)
	{
		yaw = yaw_;
		pitch = pitch_;
		roll = roll_;
	}
	void setScale(float x, float y, float z)
	{
		scale[0] = x;
		scale[1] = y;
		scale[2] = z;
	}
};

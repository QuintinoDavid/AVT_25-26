#pragma once
#include "sceneObject.h"
#include <cmath>
#include <random>
#include <iostream>


class Particle : public SceneObject {
private:  
    float	original_life;	
	float	fade;	    // fade speed(life decrease)	 
	GLfloat x, y, z;    // original position
    GLfloat ovx, ovy, ovz; // original velocity
	GLfloat vx, vy, vz; // velocity
	GLfloat ax, ay, az; // acceleration 
    float camX, camY, camZ; // camera
    public:
    
    float	curr_life;

    Particle(const std::vector<int>& meshes, int texMode, float original_life, float fade, GLfloat x, GLfloat y, GLfloat z, GLfloat vx, GLfloat vy, GLfloat vz, GLfloat ax, GLfloat ay, GLfloat az)
        : SceneObject(meshes, texMode), original_life(original_life), fade(fade), x(x), y(y), z(z), ovx(vx), ovy(vy), ovz(vz), ax(ax), ay(ay), az(az) 
        {
            setPosition(x, y, z);
            vx = ovx; vy = ovy; vz = ovz;
            curr_life = original_life; 
        }

    void update(float deltaTime) override {
        if (curr_life > 0.0f) {
            float prevPos[3] = {pos[0], pos[1], pos[2]};
            
            pos[0] = prevPos[0] + vx * deltaTime;
            pos[1] = prevPos[1] + vy * deltaTime;
            pos[2] = prevPos[2] + vz * deltaTime;
            
            vx += ax * deltaTime;
            vy += ay * deltaTime;
            vz += az * deltaTime;   
            curr_life -= fade * deltaTime;
        }
    }

    void reset() {
        setPosition(x, y, z);
        vx = ovx; vy = ovy; vz = ovz;
        curr_life = original_life; // reset current life
    }

    void setCameraPos(float x, float y, float z) {
        camX = x;
        camY = y;
        camZ = z;
    }

    void render(Renderer &renderer, gmu &mu) {
        if (!active) return;

            mu.pushMatrix(gmu::MODEL);
            mu.translate(gmu::MODEL, pos[0], pos[1], pos[2]);
            
            // Face camera
            float dirX = camX - pos[0];
            float dirY = camY - pos[1];
            float dirZ = camZ - pos[2];
            
            yaw   = atan2f(dirX, dirZ) * 180.0f / PI_F;
            float lenXZ = sqrtf(dirX*dirX + dirZ*dirZ);
            pitch = atan2f(dirY, lenXZ) * 180.0f / PI_F;
            
            mu.rotate(gmu::MODEL, yaw, 0.0f, 1.0f, 0.0f);
            mu.rotate(gmu::MODEL, pitch, 1.0f, 0.0f, 0.0f);
            mu.rotate(gmu::MODEL, roll, 0.0f, 0.0f, 1.0f);
            mu.scale(gmu::MODEL, scale[0], scale[1], scale[2]);

            mu.computeDerivedMatrix(gmu::PROJ_VIEW_MODEL);
            mu.computeNormalMatrix3x3();

            for (int mID : meshID)
            {
                dataMesh data;
                data.meshID = mID;
                data.texMode = texMode;
                data.vm = mu.get(gmu::VIEW_MODEL);
                data.pvm = mu.get(gmu::PROJ_VIEW_MODEL);
                data.normal = mu.getNormalMatrix();

                renderer.renderMesh(data);
            }

            mu.popMatrix(gmu::MODEL);
    }
};

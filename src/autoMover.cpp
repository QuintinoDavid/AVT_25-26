#pragma once
#include "sceneObject.h"
#include <cmath>
#include <random>
#include <iostream>


class AutoMover : public SceneObject {
private:
    float dir[3] = { 0.0f, 0.0f, 0.0f }; // direction
    float radius;      // How far from the center to move before turning back
    float Speed;          // rotações em radianos por segundo
    std::uniform_real_distribution<float> dist_pos{ -10.0f, 10.0f };
    std::uniform_real_distribution<float> one{ -1.0f, 1.0f };
    std::mt19937 gen{ std::random_device{}() }; // random engine for distF

    void calcNewDir(float dir[3]) {
        float theta = std::uniform_real_distribution<float>(0.f, 2.f * PI_F)(gen);
        float u = std::uniform_real_distribution<float>(-1.f, 1.f)(gen);
        float s = std::sqrt(1 - u * u);
        dir[0] = s * std::cos(theta);
        dir[1] = s * std::sin(theta);
        dir[2] = u;
    }

    void calcNewPos(float pos[3]) {
        pos[0] = dist_pos(gen);
        pos[1] = std::fabs(dist_pos(gen));
        pos[2] = dist_pos(gen);
    }


    void updateCollider() {
        // Usa o maior componente da escala como base para um AABB simples (esfera aproximada)
        float r = std::max(scale[0], std::max(scale[1], scale[2])) * 0.5f;
        collider.setBox(pos[0], pos[1], pos[2],
                        pos[0] + 2*r, pos[1] + 2*r, pos[2] + 2*r);
    }

    float calcDistFromOrigin() {
        return std::sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
    }

public:
    AutoMover(const std::vector<int>& meshes, int texMode, float radius, float speed)
        : SceneObject(meshes, texMode), radius(radius), Speed(speed) {
            calcNewDir(dir);
        updateCollider();
    }

    void update(float deltaTime) override {
        float prevPos[3] = {pos[0], pos[1], pos[2]};
        float dist = calcDistFromOrigin();
        float reverse = 1.0f;

        if (dist >= radius || pos[1] <= 0.1f) {
            //reset position and add new direction
            calcNewDir(dir);
            calcNewPos(prevPos);    
        }
        pos[0] = prevPos[0] + dir[0] * Speed * deltaTime;
        pos[1] = prevPos[1] + dir[1] * Speed * deltaTime;
        pos[2] = prevPos[2] + dir[2] * Speed * deltaTime;
        updateCollider();
        /*
        std::cerr << "[AutoMover::update] dt=" << deltaTime
              << " pos=(" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")"
              << " dir=(" << dir[0] << ", " << dir[1] << ", " << dir[2] << ")\n";
        */
    }

    // Mantemos colisão passiva: não reage, mas o drone detecta a sua AABB
    void onCollision(Collider* other) override { (void)other; }
};

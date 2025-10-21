#include "autoMover.h"

AutoMover::AutoMover(const std::vector<int> &meshes, int texMode, float radius, float speed)
    : SceneObject(meshes, texMode), radius(radius), Speed(speed)
{
    calcNewDir(dir);
}

AutoMover::~AutoMover() {}

void AutoMover::calcNewDir(float dir[3])
{
    std::uniform_real_distribution<float> variation(-10.f, 10.f);
    std::uniform_real_distribution<float> variationY(-5.f, 5.f);
    float newdir[3] = {-pos[0] + variation(gen), variationY(gen), -pos[2] + variation(gen)};

    // Normalize
    float length = std::sqrt(newdir[0] * newdir[0] + newdir[1] * newdir[1] + newdir[2] * newdir[2]);
    if (length > 1e-6f)
    {
        dir[0] = newdir[0] / length;
        dir[1] = newdir[1] / length;
        dir[2] = newdir[2] / length;
    }
    else
    {
        dir[0] = 1.0f;
        dir[1] = 0.0f;
        dir[2] = 0.0f; // default direction
    }
}

void AutoMover::calcNewPos(float pos[3])
{
    std::normal_distribution<float> dist(0.f, 1.f);
    float dir[3];
    calcNewDir(dir);
    float x, z, len2;
    do
    {
        x = dist(gen);
        z = dist(gen);
        len2 = x * x + z * z;
    } while (len2 < 1e-6f); // avoid zero vector
    float invLen = 1.0f / std::sqrt(len2);

    pos[0] = x * invLen * radius;
    pos[1] = dist(gen) * 5.f + 5.f;
    pos[2] = z * invLen * radius;
}

void AutoMover::updateCollider()
{
    float r = std::max(scale[0], std::max(scale[1], scale[2])) * 2.f;
    float ry = std::max(scale[0], std::max(scale[1], scale[2])) * 0.5f;
    collider.setBox(pos[0] - r, pos[1] - ry, pos[2] - r,
                    pos[0] + r, pos[1] + ry, pos[2] + r);
}

float AutoMover::calcDistFromXZ0()
{
    return std::sqrt(pos[0] * pos[0] + pos[2] * pos[2]);
}

void AutoMover::update(float deltaTime)
{
    updateCollider();
    float prevPos[3] = {pos[0], pos[1], pos[2]};
    float PrevRot[3] = {yaw, pitch, roll};
    float dist = calcDistFromXZ0();

    if (dist >= radius || pos[1] <= 0.1f || pos[1] >= 20.0f)
    {
        // reset position and add new direction
        calcNewDir(dir);
        calcNewPos(prevPos);
    }
    pos[0] = prevPos[0] + dir[0] * Speed * deltaTime;
    pos[1] = prevPos[1] + dir[1] * Speed * deltaTime;
    pos[2] = prevPos[2] + dir[2] * Speed * deltaTime;

    setRotation(PrevRot[0] + 1000 * deltaTime, PrevRot[1], PrevRot[2]);
    /*
    std::cerr << "[AutoMover::update] dt=" << deltaTime
          << " pos=(" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")"
          << " dir=(" << dir[0] << ", " << dir[1] << ", " << dir[2] << ")\n";
    */
}

// Mantemos colisão passiva: não reage, mas o drone detecta a sua AABB
void AutoMover::onCollision(Collider *other) { (void)other; }

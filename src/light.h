#pragma once

#include "renderer.h"
#include "mathUtility.h"
#include "sceneObject.h"

enum LightType {
    DIRECTIONAL,
    POINTLIGHT,
    SPOTLIGHT
};

class Light
{

private:
    float color[4];
    float ambient = 0.1f;
    float diffuse = 1.f;
    LightType type;

    float position[4] = {0.f, 0.f, 0.f, 1.f};
    float direction[4] = {0.f, 0.f, 0.f, 0.f};
    float cutoff = 0.93f;
    bool active = true;
    bool debug = false;

    float attConstant = 1.f;
    float attLinear = 0.1f;
    float attExp = 0.01f;

    float yaw = 0.f, pitch = 0.f;
    bool rotated = false;

    SceneObject* object = nullptr;

public:

    Light(LightType type, float color[4]) : type(type)
    {
        for (int i = 0; i < 4; i++) this->color[i] = color[i];

    }

    Light& setAmbient(float a) { ambient = a; return *this; }
    Light& setDiffuse(float d) { diffuse = d; return *this; }
    Light& setCutoff (float c) { cutoff  = c; return *this; }

    Light& setColor(float c[4])
    {
        for (int i = 0; i < 4; i++) color[i] = c[i];
        return *this;
    }

    Light& setPosition(float p[4])
    {
        for (int i = 0; i < 3; i++) this->position[i] = p[i];
        if (object != nullptr) object->setPosition(p[0], p[1], p[2]);
        return *this;
    }

    Light& setDirection(float d[4])
    {
        for (int i = 0; i < 4; i++) this->direction[i] = d[i];
        return *this;
    }

    Light& setRotation(float yaw, float pitch) {
        rotated = true;
        this->yaw = yaw;
        this->pitch = pitch;
        return *this;
    }

    Light& setAttenuation(float constant, float linear, float exp)
    {
        attConstant = constant;
        attLinear = linear;
        attExp = exp;
        return *this;
    }

    Light& createObject(Renderer &renderer, std::vector<SceneObject*>& scene) {
        if (type == LightType::POINTLIGHT) {
            MyMesh sphere = createSphere(0.1f, 20);
            for (int i = 0; i < 4; i++) {
                sphere.mat.ambient[i] = 0.f;
                sphere.mat.diffuse[i] = 0.f;
                sphere.mat.specular[i] = 0.f;
                sphere.mat.emissive[i] = color[i];
            }
            int objectId = renderer.addMesh(std::move(sphere));
            object = new SceneObject(std::vector<int>{objectId}, TexMode::TEXTURE_NONE);
            object->setPosition(position[0], position[1], position[2]);
            scene.push_back(object);
        }

        if (type == LightType::SPOTLIGHT) {
            MyMesh cone = createCone(0.2f, 0.1f, 20);
            for (int i = 0; i < 4; i++) {
                cone.mat.ambient[i] = 0.f;
                cone.mat.diffuse[i] = 0.f;
                cone.mat.specular[i] = 0.f;
                cone.mat.emissive[i] = color[i];
            }
            int objectId = renderer.addMesh(std::move(cone));
            object = new SceneObject(std::vector<int>{objectId}, TexMode::TEXTURE_NONE);
            object->setPosition(position[0], position[1], position[2]);
            float yaw = std::atan2(direction[0], direction[2]) * (180.f / PI_F);
            float pitch = std::asin(-direction[1]) * (180.f / PI_F);
            object->setRotation(yaw, pitch - 90.f, 0.f);
            if (debug) object->setScale(0.5f, 5.f, 0.5f);
            scene.push_back(object);
        }

        toggleObj();
        return *this;
    }

    void setup(Renderer &renderer, gmu &mu)
    {
        if (!active || debug) return;

        float pos[4], dir[4];
        float localPosition[4];
        float localDirection[4];
        for (int i = 0; i < 4; i++) {
            pos[i] = position[i];
            dir[i] = direction[i];
        }
        if (rotated) {
            float yawRad = (yaw + 90.f) * PI_F / 180.0f;
            float pitchRad = pitch * PI_F / 180.0f;
            dir[0] = std::cos(pitchRad) * std::cos(yawRad);
            dir[1] = std::sin(pitchRad);
            dir[2] = std::cos(pitchRad) * std::sin(-yawRad);
        }
        if (object && rotated) object->setRotation(yaw, pitch + 90.f, 0.f);

        if (type == LightType::DIRECTIONAL)
        {
            mu.multMatrixPoint(gmu::VIEW, dir, localDirection);
            renderer.setDirectionalLight(color, ambient, diffuse, localDirection);
        }
        if (type == LightType::POINTLIGHT)
        {
            mu.multMatrixPoint(gmu::VIEW, pos, localPosition);
            renderer.setPointLight(color, ambient, diffuse, localPosition,
                                   attConstant, attLinear, attExp);
        }
        else if (type == LightType::SPOTLIGHT)
        {
            mu.multMatrixPoint(gmu::VIEW, pos, localPosition);
            mu.multMatrixPoint(gmu::VIEW, dir, localDirection);
            renderer.setSpotLight(color, ambient, diffuse, localDirection, cutoff, localPosition,
                                  attConstant, attLinear, attExp);
        }
    }

    SceneObject* getObject() { return object; }

    void invertY() {
        position[1] = -1 * position[1];
        direction[1] = -1 * direction[1];
    }

    Light& setDebug() { debug = !debug; return *this; }
    bool isDebug() { return debug; }
    bool isType(LightType t) { return (type == t) && !debug ; }
    void toggleObj() { if (object) object->toggle(); }
    void toggleLight() { active = !active; }
};

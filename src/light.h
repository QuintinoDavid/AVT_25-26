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

    float position[4];
    float direction[4];
    float cutoff = 0.93f;
    bool active = true;
    bool debug = false;

    float attConstant = 1.f;
    float attLinear = 0.1f;
    float attExp = 0.01f;

    SceneObject* object = nullptr;

public:

    Light(LightType type, float color[4]) : type(type)
    {
        for (int i = 0; i < 4; i++) this->color[i] = color[i];

    }

    Light& setAmbient(float a) { ambient = a; return *this; }
    Light& setDiffuse(float d) { diffuse = d; return *this; }
    Light& setCutoff (float c) { cutoff = c; return *this; }

    Light& setColor(float c[4])
    {
        for (int i = 0; i < 4; i++) color[i] = c[i];
        return *this;
    }

    Light& setPosition(float p[4])
    {
        for (int i = 0; i < 4; i++) this->position[i] = p[i];
        return *this;
    }

    Light& setDirection(float d[4])
    {
        for (int i = 0; i < 4; i++) this->direction[i] = d[i];
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
            memset(sphere.mat.ambient, 0, 4 * sizeof(float));
            memset(sphere.mat.diffuse, 0, 4 * sizeof(float));
            memset(sphere.mat.specular, 0, 4 * sizeof(float));
            memcpy(sphere.mat.emissive, color, 4 * sizeof(float));
            int objectId = renderer.addMesh(std::move(sphere));
            object = new SceneObject(std::vector<int>{objectId}, TexMode::TEXTURE_NONE);
            object->setPosition(position[0], position[1], position[2]);
            scene.push_back(object);
        }

        if (type == LightType::SPOTLIGHT) {
            MyMesh cone = createCone(0.2f, 0.1f, 20);
            memset(cone.mat.ambient, 0, 4 * sizeof(float));
            memset(cone.mat.diffuse, 0, 4 * sizeof(float));
            memset(cone.mat.specular, 0, 4 * sizeof(float));
            memcpy(cone.mat.emissive, color, 4 * sizeof(float));
            int objectId = renderer.addMesh(std::move(cone));
            object = new SceneObject(std::vector<int>{objectId}, TexMode::TEXTURE_NONE);
            object->setPosition(position[0], position[1], position[2]);
            float yaw = std::atan2(direction[0], direction[2]) * (180.f / M_PI);
            float pitch = std::asin(-direction[1]) * (180.f / M_PI);
            object->setRotation(yaw, pitch - 90.f, 0.f);
            if (debug) object->setScale(0.5f, 5.f, 0.5f);
            scene.push_back(object);
        }

        return *this;
    }

    void render(Renderer &renderer, gmu &mu)
    {
        if (!active || debug) return;

        float localDirection[4];
        float localPosition[4];

        if (type == LightType::DIRECTIONAL)
        {
            mu.multMatrixPoint(gmu::VIEW, direction, localDirection);
            renderer.setDirectionalLight(color, ambient, diffuse, localDirection);
        }
        if (type == LightType::POINTLIGHT)
        {
            mu.multMatrixPoint(gmu::VIEW, position, localPosition);
            renderer.setPointLight(color, ambient, diffuse, localPosition,
                                   attConstant, attLinear, attExp);
        }
        else if (type == LightType::SPOTLIGHT)
        {
            mu.multMatrixPoint(gmu::VIEW, position, localPosition);
            mu.multMatrixPoint(gmu::VIEW, direction, localDirection);
            renderer.setSpotLight(color, ambient, diffuse, localDirection, cutoff, localPosition,
                                  attConstant, attLinear, attExp);
        }
    }

    std::string str() {
        if (type == LightType::DIRECTIONAL) {
            return "DIRECTIONAL";
        }
        if (type == LightType::POINTLIGHT) {
            return "POINTLIGHT";
        }
        return "SPOTLIGHT";
    }

    std::string cstr() {
        return  "{ " + std::to_string(color[0]) + 
                ", " + std::to_string(color[1]) +
                ", " + std::to_string(color[2]) + " }";
    }

    std::string dpstr() {
        if (type == LightType::POINTLIGHT) {
            return  "{ " + std::to_string(position[0]) + 
                    ", " + std::to_string(position[1]) +
                    ", " + std::to_string(position[2]) + " }";
        } else {
            return  "{ " + std::to_string(direction[0]) + 
                    ", " + std::to_string(direction[1]) +
                    ", " + std::to_string(direction[2]) + " }";
        }
    }

    Light& setDebug() { debug = !debug; return *this; }
    bool isDebug() { return debug; }
    bool isType(LightType t) { return (type == t) && !debug ; }
    void toggleObj() { if (object) object->toggle(); }
    void toggleLight() { active = !active; }
};

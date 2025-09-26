#include "renderer.h"
#include "mathUtility.h"

enum lightType {
    DIRECTIONAL,
    POINTLIGHT,
    SPOTLIGHT
};

class Light
{

private:
    float color[4];
    float ambient; // Intensity
    float diffuse; // Intensity
    lightType type;

    float position[4];
    float direction[4];
    float cutoff;
    bool active = true;

    float attConstant;
    float attLinear;
    float attExp;

public:

    Light(float color[4], float ambient, float diffuse, float direction[4])
    : ambient(ambient), diffuse(diffuse)
    {
        this->type = DIRECTIONAL;

        for (int i = 0; i < 4; i++) {
            this->color[i] = color[i];
            this->direction[i] = direction[i];
        }
    }

    Light(float color[4], float ambient, float diffuse, float position[4],
          float attConstant, float attLinear, float attExp)
    : ambient(ambient), diffuse(diffuse), attConstant(attConstant), attLinear(attLinear), attExp(attExp)
    {
        this->type = POINTLIGHT;

        for (int i = 0; i < 4; i++) {
            this->color[i] = color[i];
            this->position[i] = position[i];
        }
    }

    Light(float color[4], float ambient, float diffuse, float position[4], float direction[4],
          float cutoff, float attConstant, float attLinear, float attExp)
    : ambient(ambient), diffuse(diffuse), cutoff(cutoff),
      attConstant(attConstant), attLinear(attLinear), attExp(attExp)
    {
        this->type = SPOTLIGHT;

        for (int i = 0; i < 4; i++) {
            this->color[i] = color[i];
            this->direction[i] = direction[i];
            this->position[i] = position[i];
        }
    }

    void render(Renderer &renderer, gmu &mu)
    {
        if (!active) return;

        float localDirection[4];
        float localPosition[4];

        for (int i = 0; i < 4; i++) {
            localDirection[i] = direction[i];
            localPosition[i] = position[i];
        }

        if (type == lightType::DIRECTIONAL)
        {
            mu.multMatrixPoint(gmu::VIEW, direction, localDirection);
            renderer.setDirectionalLight(color, ambient, diffuse, localDirection);
        }
        if (type == lightType::POINTLIGHT)
        {
            mu.multMatrixPoint(gmu::VIEW, position, localPosition);
            renderer.setPointLight(color, ambient, diffuse, localPosition,
                                   attConstant, attLinear, attExp);
        }
        else if (type == lightType::SPOTLIGHT)
        {
            mu.multMatrixPoint(gmu::VIEW, position, localPosition);
            mu.multMatrixPoint(gmu::VIEW, direction, localDirection);
            renderer.setSpotLight(color, ambient, diffuse, localDirection, cutoff, localPosition,
                                  attConstant, attLinear, attExp);
        }
    }

    bool isType(lightType t) { return type == t; }
    bool isSpotlight() { return type == lightType::SPOTLIGHT; }
    void toggle() { active = !active; }
};

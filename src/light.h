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

    float position[3];
    float direction[3];
    float cutoff;
    bool active = true;

    float attConstant;
    float attLinear;
    float attExponential;

public:

    Light(float color[4], float ambient, float diffuse, float direction[4])
    : type(DIRECTIONAL), ambient(ambient), diffuse(diffuse)
    {
        for (int i = 0; i < 4; i++) {
            this->color[i] = color[i];
        }

        for (int i = 0; i < 3; i++) {
            this->direction[i] = direction[i];
        }
    }

    Light(float color[4], float ambient, float diffuse, float position[4],
          float attConstant, float attLinear, float attExponential)
    : type(POINTLIGHT), ambient(ambient), diffuse(diffuse),
      attConstant(attConstant), attLinear(attLinear), attExponential(attExponential)
    {
        for (int i = 0; i < 4; i++) {
            this->color[i] = color[i];
        }

        for (int i = 0; i < 3; i++) {
            this->position[i] = position[i];
        }
    }

    Light(float color[4], float ambient, float diffuse, float position[4], float direction[4],
          float cutoff, float attConstant, float attLinear, float attExponential)
    : type(SPOTLIGHT), ambient(ambient), diffuse(diffuse), cutoff(cutoff),
      attConstant(attConstant), attLinear(attLinear), attExponential(attExponential)
    {
        for (int i = 0; i < 4; i++) {
            this->color[i] = color[i];
        }

        for (int i = 0; i < 3; i++) {
            this->direction[i] = direction[i];
            this->position[i] = position[i];
        }
    }

    void render(Renderer &renderer, gmu &mu)
    {
        if (!active) return;

        if (type == lightType::DIRECTIONAL)
        {
            renderer.setDirectionalLight(color, ambient, diffuse, direction);
            return;
        }
        
        float localPosition[4];
        mu.multMatrixPoint(gmu::VIEW, position, localPosition);
        
        if (type == lightType::POINTLIGHT)
        {
            renderer.setPointLight(color, ambient, diffuse, localPosition,
                                   attConstant, attLinear, attExponential);
        }
        else if (type == lightType::SPOTLIGHT)
        {
            renderer.setSpotLight(color, ambient, diffuse, direction, cutoff, localPosition,
                                  attConstant, attLinear, attExponential);
        }
    }

    bool isSpotlight() { return type == lightType::SPOTLIGHT; }
    void toggle() { active = !active; }
};

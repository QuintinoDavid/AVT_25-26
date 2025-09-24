#include "renderer.h"
#include "mathUtility.h"
#include "sceneObject.h"

enum lightType {
    DIRECTIONAL,
    POINTLIGHT,
    SPOTLIGHT
};

class Light : public SceneObject
{

private:
    float color[4];
    float ambient; // Intensity
    float diffuse; // Intensity
    lightType type;

    float position[4];
    float direction[4];
    float cutoff;

    float attConstant;
    float attLinear;
    float attExponential;

public:

    Light(float color[4], float ambient, float diffuse, float direction[4], int meshID_ = -1, int texMode_ = 1)
    : type(DIRECTIONAL), ambient(ambient), diffuse(diffuse), SceneObject(meshID_, texMode_)
    {
        for (int i = 0; i < 4; i++) {
            this->direction[i] = direction[i];
        }
    }

    Light(float color[4], float ambient, float diffuse, float position[4],
          float attConstant, float attLinear, float attExponential, int meshID_ = -1, int texMode_ = 1)
    : type(POINTLIGHT), ambient(ambient), diffuse(diffuse), SceneObject(meshID_, texMode_),
      attConstant(attConstant), attLinear(attLinear), attExponential(attExponential)
    {
        for (int i = 0; i < 4; i++) {
            this->position[i] = position[i];
        }
    }

    Light(float color[4], float ambient, float diffuse, float position[4], float direction[4], float cutoff,
          float attConstant, float attLinear, float attExponential, int meshID_ = -1, int texMode_ = 1)
    : type(SPOTLIGHT), ambient(ambient), diffuse(diffuse), cutoff(cutoff), SceneObject(meshID_, texMode_),
      attConstant(attConstant), attLinear(attLinear), attExponential(attExponential)
    {
        for (int i = 0; i < 4; i++) {
            this->direction[i] = direction[i];
            this->position[i] = position[i];
        }
    }

    void render(Renderer &renderer, gmu &mu)
    {
        if (type == lightType::DIRECTIONAL)
        {
            renderer.setDirectionalLight(color, ambient, diffuse, direction);
        }

        /*
        // send the light position in eye coordinates
        // renderer.setLightPos(lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord
        float lposAux[4];
        mu.multMatrixPoint(gmu::VIEW, lightPos, lposAux); // lightPos definido em World Coord so is converted to eye space
        renderer.setLightPos(lposAux);

        // Spotlight settings
        renderer.setSpotLightMode(spotlight_mode);
        renderer.setSpotParam(coneDir, 0.93);
        */
    }

    void handleInput(int key) override
    {
        switch (key)
        {
        case 'l': // toggle spotlights
            break;
        }
    }
};

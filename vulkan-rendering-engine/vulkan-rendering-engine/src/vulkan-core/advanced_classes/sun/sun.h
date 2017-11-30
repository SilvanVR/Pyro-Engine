#ifndef SUN_H_
#define SUN_H_

#include "vulkan-core/data/lighting/directional_light.h"
#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"

namespace Pyro
{

    class LightShafts;

    //---------------------------------------------------------------------------
    //  Sun class
    //---------------------------------------------------------------------------

    class Sun : public Renderable
    {

    public:
        Sun(const Vec3f& direction, const Vec2f& scale, 
            const Color& color, float intensity,
            ShadowInfo* shadowInfo = new ShadowInfo(10, 1.0f, 150));
        ~Sun();

        void update(float delta) override;

        DirectionalLight*   getDirLight(){ return &directionalLight; }
        void                toggleLightShafts() { lightShafts->toggleActive(); }
        void                setActiveLightShafts(bool b) { lightShafts->setActive(b); }
        void                toggleCameraDust();
        void                setActiveCameraDust(bool b);
        void                setLightShaftsRadialBlurScale(float scale) { lightShafts->setRadialBlurScale(scale); }
        void                setLightShaftsRadialBlurStrength(float strength) { lightShafts->setRadialBlurStrength(strength); }
        void                setColor(const Color& color);
        void                setRedChannel(float val);
        void                setGreenChannel(float val);
        void                setBlueChannel(float val);
        void                setTexture(TexturePtr tex, bool lightShaftTexAswell = true);

        Color               getColor() const { return directionalLight.getColor(); }

    private:
        // Directional-Light
        DirectionalLight directionalLight;

        // Shader for the rendering a "sun"-texture in the background (near zFar plane)
        ForwardShaderPtr sunDiskShader;

        // Texture for the sun
        TexturePtr sunTexture;

        // Reference to light-shaft shaders
        LightShafts* lightShafts;

        void setupShader();
    };



}





#endif // !SUN_H_


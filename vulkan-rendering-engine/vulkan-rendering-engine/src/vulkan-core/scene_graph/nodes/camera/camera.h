#ifndef CAMERA_H_
#define CAMERA_H_

#include "vulkan-core/scene_graph/layers/layer_mask.h"
#include "vulkan-core/scene_graph/nodes/node.h"
#include "vulkan-core/data/mapped_values.h"
#include "frustum.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Renderable;
    class Light;

    //---------------------------------------------------------------------------
    //  Camera class
    //---------------------------------------------------------------------------

    class Camera : public Node, public MappedValues
    {
    public:
        enum EMode
        {
            ORTHOGRAPHIC,
            PERSPECTIVE,
            CUSTOM  // The programmer is responsible for the view & projection matrix of this camera via setViewMatrix() & setProjectionMatrix().
        };

        Camera(const Transform& transform, Camera::EMode renderingMode = Camera::EMode::PERSPECTIVE, LayerMask layerMask = LayerMask({ LAYER_ALL_3D }));
        Camera(float fov, float _zNear, float _zFar, LayerMask layerMask = LayerMask({ LAYER_ALL_3D }));
        Camera(float _left, float _right, float _bottom, float _top, float _zNear, float _zFar, LayerMask layerMask = LayerMask({ LAYER_ALL_3D }));
        virtual ~Camera();

        // Update frustum and cached view-projection
        void update(float delta) override;
        void lateUpdate(float delta) override;

        // Record commands for rendering the given objects. 
        void render(VkCommandBuffer cmd, ShaderPtr shader, const std::vector<Renderable*>& renderables, bool cull = true);

        // Record commands for rendering a single object.
        void render(VkCommandBuffer cmd, ShaderPtr shader, Renderable* renderable, bool cull);

        // Record commands for rendering multiple lights.
        void render(VkCommandBuffer cmd, ShaderPtr shader, const std::vector<Light*>& lights, bool cull = true);

        // Record commands for rendering a single light.
        void render(VkCommandBuffer cmd, ShaderPtr shader, Light* light, bool cull = true);

        // Set the projection parameters for this camera
        void            setPerspectiveParams(float fov, float zNear, float zFar);

        // Set the view matrix directly. Note that the rendering-mode should be "CUSTOM", 
        // otherwise the view matrix will be recalculated each frame based on the camera's transform.
        void            setViewMatrix(const Mat4f& view);

        // Set the projection matrix directly. The Vulkan-Clip Matrix is also applied.
        // Note that the rendering-mode should be "CUSTOM", otherwise the projection will be reset on window-resize.
        void            setProjectionMatrix(const Mat4f& projection);

        // Set the projection parameters for this camera
        void            setOrthoParams(float left, float right, float bottom, float top, float zNear, float zFar);

        // Set the aspec ratio for a perspective camera
        void            setAspecRatio(float ar);

        // Change the rendering mode e.g. perspective -> orthographic
        void            setRenderingMode(Camera::EMode renderingMode);


        // Getters
        const std::vector<Renderable*>& getLastTimeRendered() { return lastTimeRendered; }
        const std::vector<Light*>& getLastTimeRenderedLights() { return lastTimeRenderedLights; }
        float           getFOV() { return fov; }
        float           getZNear() { return zNear; }
        float           getZFar() { return zFar; }
        Camera::EMode   getMode() { return mode; }
        float           getAspectRatio(){ return ar; }
        float           getLeft() { return left; }
        float           getRight(){ return right; }
        float           getTop() { return top; }
        float           getBottom() { return bottom; }
        LayerMask&      getLayerMask() { return layerMask; }

        // Setters
        void            setLeft(float l){ left = l; precalculateProjection(); }
        void            setRight(float r) { right = r;precalculateProjection(); }
        void            setTop(float t) { top = t; precalculateProjection(); }
        void            setBottom(float b) { bottom = b; precalculateProjection(); }
        void            setZNear(float zNear){ this->zNear = zNear; precalculateProjection(); }
        void            setZFar(float zFar){ this->zFar = zFar; precalculateProjection(); }
        void            setFOV(float fov){ this->fov = fov; precalculateProjection(); }
        void            setApplyVulkanClip(bool b) { applyVulkanClip = b; precalculateProjection(); }

        // Return the camera's view matrix 
        const Mat4f&    getViewMatrix();

        // Return the projection matrix from this camera.
        const Mat4f&    getProjection() { return projection; }

        // Return the view-projection matrix from this camera
        const Mat4f&    getViewProjection() { return viewProjection; }

    private:
        // Rendering-Mode this camera is using
        EMode           mode;

        // True if the vulkan-clip matrix should be applied
        bool            applyVulkanClip;

        // Params used for perspective- renderinmg
        float           fov;
        float           ar;
        float           zNear;
        float           zFar;

        // Params used for orthographic - rendering
        float           left;
        float           right;
        float           top;
        float           bottom;
        
        Mat4f           view;               // Cached view matrix
        Mat4f           projection;         // Cached projection matrix
        Mat4f           viewProjection;     // Cached view-projection matrix. Recalculated every update
        Frustum         frustum;            // Viewfrustum for this camera. Used to cull objects and lights.
        LayerMask       layerMask;          // Which layer this camera will render

        std::vector<Light*>         lastTimeRenderedLights; // List of lights this camera rendered last time
        std::vector<Renderable*>    lastTimeRendered;       // List of objects this camera rendered last time

        // Precalculate the projection matrix based on the Enum "mode"
        void precalculateProjection();

        // Called in constructor, init everything
        void init();

        // Check if the given node should be rendered
        bool checkNode(Node* node);
    };

}

#endif // !CAMERA_H_

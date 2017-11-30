#include "camera.h"

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/data/lighting/light.h"

namespace Pyro
{

    // Vulkan clip space has inverted Y and half Z, thats why the projection has to be multiplied by this matrix.
    static Mat4f vulkanClip = Mat4f(1.0f,  0.0f, 0.0f, 0.0f,
                                    0.0f, -1.0f, 0.0f, 0.0f,
                                    0.0f,  0.0f, 0.5f, 0.5f,
                                    0.0f,  0.0f, 0.0f, 1.0f);

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Camera::Camera(const Transform& transform, Camera::EMode renderingMode, LayerMask _layerMask)
        : Node("Camera", transform, Node::Dynamic), mode(renderingMode), 
          fov(60.0f), zNear(0.1f), zFar(3000.0f), ar(1.0f), left(-50), right(50), bottom(-50), top(50),
          frustum(this), layerMask(_layerMask), applyVulkanClip(true)
    {
        init();
    }

    Camera::Camera(float _fov, float _zNear, float _zFar, LayerMask _layerMask)
        : Node("Camera", Node::Dynamic), mode(EMode::PERSPECTIVE), 
         fov(_fov), zNear(_zNear), zFar(_zFar), ar(1.0f), left(-50), right(50), bottom(-50), top(50),
         frustum(this), layerMask(_layerMask), applyVulkanClip(true)
    {
        init();
    }

    Camera::Camera(float _left, float _right, float _bottom, float _top, float _zNear, float _zFar, LayerMask _layerMask)
        : Node("Camera", Node::Dynamic), mode(EMode::ORTHOGRAPHIC), 
          left(_left), right(_right), bottom(_bottom), top(_top), zNear(_zNear), zFar(_zFar), fov(60.0f), ar(1.0f),
          frustum(this), layerMask(_layerMask), applyVulkanClip(true)
    {
        init();
    }

    // Called in constructor, init everything
    void Camera::init()
    {
        precalculateProjection();
        createDescriptorSets("DescriptorSets#CAMERA");
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Camera::~Camera()
    {}

    //---------------------------------------------------------------------------
    //  Update Methods
    //---------------------------------------------------------------------------

    // Update the frustum.
    void Camera::update(float delta)
    {
        Node::update(delta);
        switch (mode)
        {
        case ORTHOGRAPHIC: case PERSPECTIVE:
            // Update cached matrices
            view = getWorldRotation().conjugate().toMatrix4x4() * Mat4f::translation(-getWorldPosition());
            //view = Mat4f::view(getWorldPosition(), getWorldRotation().getForward(), getWorldRotation().getUp());
            break;
        case CUSTOM:
            break;
        }

        viewProjection = projection * view;

        // Update frustum class
        frustum.update(viewProjection);

        // Update descriptor-set
        setVec3f("position", getWorldPosition());
        setMat4f("viewProjection", viewProjection);
        setMat4f("viewMatInv", view.inversed());
        setMat4f("projMatInv", projection.inversed());
    }

    void Camera::lateUpdate(float delta)
    {
        Node::lateUpdate(delta);
        lastTimeRendered.clear();
        lastTimeRenderedLights.clear();
    }

    //---------------------------------------------------------------------------
    //  Render Methods
    //---------------------------------------------------------------------------

    void Camera::render(VkCommandBuffer cmd, ShaderPtr shader, const std::vector<Renderable*>& renderables, bool cull)
    {
        for (const auto& renderable : renderables)
            if (!cull || checkNode(renderable))
            {
                if (!(LayerMask({LAYER_BOUNDING_BOX}) & renderable->getLayerMask())) 
                    lastTimeRendered.push_back(renderable);
                renderable->render(cmd, shader);
            }
    }

    void Camera::render(VkCommandBuffer cmd, ShaderPtr shader, Renderable* renderable, bool cull)
    {
        if (!cull || checkNode(renderable))
        {
            if (!(LayerMask({ LAYER_BOUNDING_BOX }) & renderable->getLayerMask())) lastTimeRendered.push_back(renderable);
            renderable->render(cmd, shader);
        }
    }

    void Camera::render(VkCommandBuffer cmd, ShaderPtr shader, const std::vector<Light*>& lights, bool cull)
    {
        for (const auto& light : lights)
            if (!cull || checkNode(light))
            {
                lastTimeRenderedLights.push_back(light);
                light->render(cmd, shader);
            }
    }

    void Camera::render(VkCommandBuffer cmd, ShaderPtr shader, Light* light, bool cull)
    {
        if (!cull || checkNode(light))
        {
            lastTimeRenderedLights.push_back(light);
            light->render(cmd, shader);
        }
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Return the camera's view matrix 
    const Mat4f& Camera::getViewMatrix() 
    { 
        // Make sure that the view-matrix is up-to-date
        if (mode != EMode::CUSTOM) 
            view = getWorldRotation().conjugate().toMatrix4x4() * Mat4f::translation(-getWorldPosition());
        return view; 
    }

    void Camera::setPerspectiveParams(float fov, float zNear, float zFar)
    {
        this->fov = fov;
        this->zNear = zNear;
        this->zFar = zFar;

        precalculateProjection();
    }

    void Camera::setViewMatrix(const Mat4f& view)
    {
        this->view = view;
        this->viewProjection = projection * view;
        frustum.update(viewProjection);
    }

    void Camera::setProjectionMatrix(const Mat4f& projection) 
    { 
        this->projection = projection;
        if (applyVulkanClip)
            this->projection = vulkanClip * this->projection;
    }

    void Camera::setOrthoParams(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        this->left = left;
        this->right = right;
        this->bottom = bottom;
        this->top = top;
        this->zNear = zNear;
        this->zFar = zFar;

        precalculateProjection();
    }

    void Camera::setAspecRatio(float ar)
    {
        if (this->type == EMode::PERSPECTIVE)
        {
            this->ar = ar;
            precalculateProjection();
        }
    }

    void Camera::setRenderingMode(Camera::EMode renderingMode)
    {
        this->mode = renderingMode;
        if(this->mode != EMode::CUSTOM)
            precalculateProjection();
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    void Camera::precalculateProjection()
    {
        switch (mode)
        {
        case EMode::PERSPECTIVE:
            this->projection = Mat4f::perspective(Mathf::deg2Rad(fov), ar, zNear, zFar);
            break;
        case EMode::ORTHOGRAPHIC:
            this->projection = Mat4f::ortho(left, right, bottom, top, zNear, zFar);
            break;
        }

        if(applyVulkanClip)
            this->projection = vulkanClip * this->projection;
    }

    // Check if the given node should be rendered
    bool Camera::checkNode(Node* node)
    {
        // Check if object is active
        if(!node->isActive()) return false;

        // Check if this camera should render the layer from the object
        bool layerHit = node->getLayerMask() & layerMask;
        if(!layerHit) return false;

        // Check if object is within view-frustum
        bool inFrustum = node->cull(&frustum);
        if(!inFrustum) return false;

        return true;
    }

}
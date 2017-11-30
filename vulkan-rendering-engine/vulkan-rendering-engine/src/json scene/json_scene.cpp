#include "json_scene.h"

#include "logger/logger.h"
#include "file_system/vfs.h"

#include "vulkan-core/pipelines/shaders/forward_shader.h"
#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/rendering_engine.h"

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/scene_graph/nodes/renderables/skybox.h"

#include "scripts/interaction_script.h"
#include "scripts/camera_mov.h"

#include "json_defines.hpp"

namespace Pyro
{

    Vec3f getVec3WithModifiedComponent(const std::string& xyz, const Vec3f& vec, float val)
    {
        Vec3f v(vec);
        if (xyz == "x")
            v.x() = val;
        else if (xyz == "y")
            v.y() = val;
        else if (xyz == "z")
            v.z() = val;
        return v;
    }

    // Checks if "name" is present in the given JSON, print a warning if not (if it's enabled)
    static bool isExistent(const JSON& json, const char* name)
    {
        if (json.count(name))
            return true;

        if (REPORT_WARNINGS)
            Logger::Log("JSON Field '" + std::string(name) + "' is not present. Using the default value instead", LOGTYPE_WARNING);
        return false;
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void JSONScene::init(RenderingEngine* renderer)
    {
        if (json.count(JSON_NAME_SETTINGS))
            parseSettings(renderer, json[JSON_NAME_SETTINGS]);

        if (json.count(JSON_NAME_CAMERA))
            createCamera(renderer, json[JSON_NAME_CAMERA]);
        else
            Logger::Log("No Camera! The given JSON-File has to have at least a camera", LOGTYPE_ERROR);

        if (json.count(JSON_NAME_SHADERS))
            createForwardShaders(json[JSON_NAME_SHADERS]);

        if (json.count(JSON_NAME_SKYBOX))
            createSkybox(json[JSON_NAME_SKYBOX]);

        if (json.count(JSON_NAME_MODELS))
            createModels(json[JSON_NAME_MODELS]);

        if (json.count(JSON_NAME_MATERIALS))
            createMaterials(json[JSON_NAME_MATERIALS]);

        if (json.count(JSON_NAME_OBJECTS))
            createObjects(json[JSON_NAME_OBJECTS]);

        if (json.count(JSON_NAME_LIGHTS))
            createLights(json[JSON_NAME_LIGHTS]);

        attachInputFunc(KeyCodes::N, [=] {
            static int i = 0;
            std::string filePath = "result_#" + std::to_string(i++) + ".png";
            renderer->setRenderCallback([=](const ImageData& imageData) {
                // NEVER USE BY REFERENCE "[&]" for a lambda in a lambda 
                WRITE_IMAGE(filePath, imageData);
            });
        }, Input::KEY_PRESSED);
    }

    void JSONScene::modify(const JSON& newJsonText)
    {
        JSON diff = JSON::diff(json, newJsonText);
        if (diff.empty())
            return;
        //std::cout << diff.dump(4) << std::endl;

        json = newJsonText;

        for (const auto& patch : diff)
            executePatch(patch);
    }

    void JSONScene::executePatch(const JSON& patch)
    {
        std::string paths = patch["path"];
        PathManager pathManager(paths);

        OP op = stringToOP(patch["op"]);
        switch (op)
        {
        case OP::ADD:
            addValue(pathManager, patch["value"]);
            break;
        case OP::REMOVE:
            removeValue(pathManager);
            break;
        case OP::REPLACE:
            replaceValue(pathManager, patch["value"]);
            break;
        default:
            Logger::Log("JSONScene::executePatch(): Unknown Operator.", LOGTYPE_WARNING);
        }
    }

    void JSONScene::reload(RenderingEngine* renderer)
    {
        Scene::reload(renderer); // Will find a camera and make it active

        // Restore settings
        if (json.count(JSON_NAME_SETTINGS))
            parseSettings(renderer, json[JSON_NAME_SETTINGS]);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    void JSONScene::parseSettings(RenderingEngine* renderer, const JSON& json)
    {
        // Ignore resolution request if renderer renders into a window
        if (!renderer->hasWindow() && json.count(JSON_NAME_RESOLUTION) != 0)
        {
            auto res = json[JSON_NAME_RESOLUTION];
            uint32_t width = parsePrimitive<uint32_t>(res, JSON_NAME_WIDTH, DEFAULT_WIDTH);
            uint32_t height = parsePrimitive<uint32_t>(res, JSON_NAME_HEIGHT, DEFAULT_HEIGHT);

            width = width <= 0 ? DEFAULT_WIDTH : width;
            height = height <= 0 ? DEFAULT_HEIGHT : height;

            renderer->setFinalResolution(Vec2ui(width, height));
            Logger::Log("Parsed Resolution: [" + std::to_string(width) + "," + std::to_string(height) + "]", LOGTYPE_INFO);
        }

        if (json.count(JSON_NAME_IBL) != 0)
        {
            auto ibl = json[JSON_NAME_IBL];

            bool enabled = parsePrimitive<bool>(ibl, JSON_NAME_ENABLED, true);
            renderer->setUseIBL(enabled);

            if (ibl.count(JSON_NAME_AMBIENT_INTENSITY) != 0)
                renderer->setAmbientIntensity(ibl[JSON_NAME_AMBIENT_INTENSITY]);
            if (ibl.count(JSON_NAME_SPECULAR_INTENSITY) != 0)
                renderer->setEnvMapSpecularIntensity(ibl[JSON_NAME_SPECULAR_INTENSITY]);

            if (ibl.count(JSON_NAME_ENVIRONMENT_MAP))
            {
                auto envProps = ibl[JSON_NAME_ENVIRONMENT_MAP];

                if (envProps.count(JSON_NAME_SRC) != 0)
                {
                    if (renderEnvironmentMaps)
                    {
                        std::string filePath = envProps[JSON_NAME_SRC];
                        CubemapPtr env = CUBEMAP(filePath);

                        uint32_t irradianceSize = parsePrimitive<uint32_t>(envProps, JSON_NAME_IRRADIANCE_SIZE, 128);
                        irradianceMap = IRRADIANCEMAP(irradianceSize, env);

                        uint32_t premSize = parsePrimitive<uint32_t>(envProps, JSON_NAME_PREM_SIZE, 512);
                        prem = PREM(premSize, env);

                        renderEnvironmentMaps = false;
                    }
                    if (irradianceMap.isValid())
                        renderer->setAmbientIrradianceMap(irradianceMap);
                    if (prem.isValid())
                        renderer->setEnvironmentMap(prem);
                }
            }
        }
        if (json.count(JSON_NAME_SAMPLER) != 0)
        {
            auto samplerProps = json[JSON_NAME_SAMPLER];
            SSampler sampler = SSampler(new Sampler());
            if (samplerProps.count(JSON_NAME_MAX_ANISO) != 0)
                sampler->setMaxAnisotropy(samplerProps[JSON_NAME_MAX_ANISO]);
            if (samplerProps.count(JSON_NAME_FILTER) != 0)
            {
                std::string jsonFilter = samplerProps[JSON_NAME_FILTER];
                Filter filter = jsonFilter == "nearest" ? FILTER_NEAREST : FILTER_LINEAR;
                sampler->setFilter(filter, filter);
            }
            if (samplerProps.count(JSON_NAME_MIPMAP_MODE) != 0)
            {
                std::string jsonMipmapMode = samplerProps[JSON_NAME_MIPMAP_MODE];
                MipmapMode mipmapMode = jsonMipmapMode == "nearest" ? MIPMAP_MODE_NEAREST : MIPMAP_MODE_LINEAR;
                sampler->setMipmapMode(mipmapMode);
            }
            Texture::setDefaultSampler(sampler);
        }
        if (json.count(JSON_NAME_POST_PROCESSING) != 0)
            parsePostProcessing(renderer, json[JSON_NAME_POST_PROCESSING]);
    }

    void JSONScene::parsePostProcessing(RenderingEngine* renderer, const JSON& json)
    {
        float resMod = parsePrimitive<float>(json, JSON_NAME_RES_MOD, 1.0f);
        renderer->setResolutionMod(resMod);

        bool fxaaEnabled = parsePrimitive<bool>(json, JSON_NAME_FXAA, true);
        SHADER(SHADER_FXAA)->setActive(fxaaEnabled);

        bool bloomEnabled = parsePrimitive<bool>(json, JSON_NAME_BLOOM, false);
        SHADER(SHADER_HDR_BLOOM)->setActive(bloomEnabled);

        bool doaEnabled = parsePrimitive<bool>(json, JSON_NAME_DEPTH_OF_FIELD, false);
        SHADER(SHADER_DEPTH_OF_FIELD)->setActive(doaEnabled);

        bool fogEnabled = parsePrimitive<bool>(json, JSON_NAME_FOG, false);
        SHADER(SHADER_FOG)->setActive(fogEnabled);

        bool lightShaftsEnabled = parsePrimitive<bool>(json, JSON_NAME_LIGHT_SHAFTS, false);
        SHADER(SHADER_LIGHT_SHAFTS)->setActive(lightShaftsEnabled);
    }

    void JSONScene::createSkybox(const JSON& json)
    {
        CubemapPtr cube = CUBEMAP_GET(CUBEMAP_DEFAULT);
        if (json.count(JSON_NAME_CUBEMAP) != 0)
        {
            std::string path = json[JSON_NAME_CUBEMAP];
            cube = CUBEMAP(path);
        }
        std::string shaderName = getName() + "#Skybox";
        skybox = new Skybox(cube, nullptr, shaderName);
    }

    void JSONScene::createModels(const JSON& json)
    {
        for (auto it = json.begin(); it != json.end(); it++)
        {
            std::string modelName = it.key();
            std::string modelPath = parseString(it.value());
            if (modelPath == "")
                Logger::Log("Model path from model #" + modelName + " is not valid. Please put a valid path in it", LOGTYPE_ERROR);

            MeshPtr mesh = MESH(modelPath);
            mesh->setName(modelName);
            meshes[modelName] = mesh;
        }
    }

    void JSONScene::createMaterials(const JSON& json)
    {
        for (auto it = json.begin(); it != json.end(); it++)
        {
            std::string name = it.key();
            auto matProps = it.value();

            createNewMaterial(name, matProps);
        }
    }

    void JSONScene::createNewMaterial(const std::string& name, const JSON::value_type& props)
    {
        if (props.count(JSON_NAME_SHADER) != 0)
            createForwardShaderMaterial(props, name);
        else
            createPBRMaterial(props, name);
    }

    void JSONScene::createForwardShaderMaterial(const JSON& matProps, const std::string& matName)
    {
        std::string name = matProps[JSON_NAME_SHADER];
        std::string shaderName = getFullShaderName(name);
        if (forwardShaders.count(shaderName) == 0)
            Logger::Log("JSONScene::createForwardShaderMaterial(): "
                "Could not find a shader with name '" + shaderName + "'", LOGTYPE_ERROR);

        auto shader = forwardShaders[shaderName];
        MaterialPtr material = MATERIAL(shader, matName);
        materials[matName] = material;

        // Add props to material
        for (auto it = matProps.begin(); it != matProps.end(); it++)
        {
            std::string propName = it.key();
            auto val = it.value();
            if (propName == "shader") 
                continue;

            addParamToForwardShaderMaterial(material, propName, val);
        }
    }

    void JSONScene::addParamToForwardShaderMaterial(MaterialPtr material, const std::string& propName, const JSON::value_type& val)
    {
        // Get type from material-class and set the appropriate data-field
        DataType type = material->getDataType(propName);
        switch (type)
        {
        case DataType::Int:             material->setInt(propName, val); break;
        case DataType::Float:           material->setFloat(propName, val); break;
        case DataType::Texture2D:       material->setTexture(propName, parseTexture(val)); break;
        case DataType::TextureCubemap:  material->setTexture(propName, parseCubemap(val)); break;
        case DataType::Vec3:
            propName == JSON_NAME_COLOR ? material->setColor(propName, parseRawColor(val)) :
                material->setVec3f(propName, parseRawVec3f(val)); break;
        case DataType::Vec4:
            propName == JSON_NAME_COLOR ? material->setColor(propName, parseRawColor(val)) :
                material->setVec4f(propName, parseRawVec4f(val)); break;
        default:
            Logger::Log("JSONScene::addParamToForwardShaderMaterial(): Unknown/Unsupported data-type of field '"
                        + propName + "' for material '" + material->getName() + "'", LOGTYPE_WARNING);
        }
    }

    void JSONScene::createPBRMaterial(const JSON& matProps, const std::string& matName)
    {
        // the diffuse texture must be specified
        auto diffuse = parseTexture(matProps, JSON_NAME_MATERIAL_DIFFUSE);
        if (!diffuse.isValid())
        {
            Logger::Log("There is no diffuse texture specified for material #" + matName + ". "
                "Using the default texture instead.", LOGTYPE_WARNING);
            diffuse = TEXTURE_GET(TEX_DEFAULT);
        }

        PBRMaterialPtr material = PBRMATERIAL({ diffuse, matName });
        materials[matName] = material;

        // Color
        Color col = parseColor(matProps, Color::WHITE);
        material->setMatColor(col);

        // UV-Scale
        float uvScale = parsePrimitive<float>(matProps, JSON_NAME_MATERIAL_UVSCALE, 1.0f);
        material->setMatUVScale(uvScale);

        // Normal
        auto normal = parseTexture(matProps, JSON_NAME_MATERIAL_NORMAL);
        if (normal.isValid()) material->setMatNormalMap(normal);

        // Ambient Occlusion 
        auto aoMap = parseTexture(matProps, JSON_NAME_MATERIAL_AO);
        if (aoMap.isValid()) material->setMatAOMap(aoMap);

        // Roughness
        if (matProps.count(JSON_NAME_MATERIAL_ROUGHNESS) != 0)
        {
            auto prop = matProps[JSON_NAME_MATERIAL_ROUGHNESS];
            if (prop.is_number()) // Property is a number
            {
                material->setMatRoughness(prop);
            }
            else // Property is a filepath to a texture
            {
                auto roughness = parseTexture(matProps, JSON_NAME_MATERIAL_ROUGHNESS);
                material->setMatRoughnessMap(roughness);
            }
        }

        // Metallic
        if (matProps.count(JSON_NAME_MATERIAL_METALLIC) != 0)
        {
            auto prop = matProps[JSON_NAME_MATERIAL_METALLIC];
            if (prop.is_number()) // Property is a number
            {
                material->setMatMetallic(prop);
            }
            else // Property is a filepath to a texture
            {
                auto metallic = parseTexture(matProps, JSON_NAME_MATERIAL_METALLIC);
                material->setMatMetallicMap(metallic);
            }
        }
    }

    void JSONScene::createObjects(const JSON& json)
    {
        for (auto obj = json.begin(); obj != json.end(); obj++)
        {
            std::string name = obj.key();
            auto objProps = obj.value();
            createNewObject(name, objProps);
        }
    }

    void JSONScene::createNewObject(const std::string& name, const JSON::value_type& props)
    {
        Renderable* renderable = nullptr;
        Transform trans = parseTransform(props);

        // Mesh specified?
        MeshPtr mesh = MESH(MODEL_CUBE_DEFAULT);
        if (props.count(JSON_NAME_MODEL) != 0)
        {
            std::string modelName = props[JSON_NAME_MODEL];
            // not found? Exit program
            if (meshes.count(modelName) == 0)
                Logger::Log("Model with name #" + modelName + " does not exist in object #" + name, LOGTYPE_ERROR);
            mesh = meshes[modelName];
        }
        else
            Logger::Log("Can't find 'model' field in object #" + name + ". "
                "Using another one instead", LOGTYPE_WARNING);

        // Material specified?
        MaterialPtr mat = nullptr;
        if (props.count(JSON_NAME_MATERIAL) != 0)
        {
            std::string matName = props[JSON_NAME_MATERIAL];
            if (materials.count(matName) != 0)
                mat = materials[matName];
        }
        else if (mesh->hasMaterials()) // Does the mesh has maybe materials?
        {
            renderable = new Renderable(mesh, trans);
            renderable->setName(name);
        }

        if (!renderable) // Mesh had no materials, so create a renderable with the found material (or default one)
        {
            if (!mat.isValid())
            {
                mat = MATERIAL_GET(MATERIAL_DEFAULT);
                Logger::Log("Can't find 'material' field in object #" + name + ". "
                            "Using the default material instead", LOGTYPE_WARNING);
            }
            renderable = new Renderable(mesh, mat, trans);
            renderable->setName(name);
        }

        objects[name] = renderable;
    }

    void JSONScene::createLights(const JSON& json)
    {
        for (auto lightPair = json.begin(); lightPair != json.end(); lightPair++)
        {
            std::string lightName = lightPair.key();
            JSON::value_type lightProps = lightPair.value();
            createNewLight(lightName, lightProps);
        }
    }

    void JSONScene::createNewLight(const std::string& lightName, const JSON::value_type& lightProps)
    {
        if (!lightProps.count(JSON_NAME_LIGHT_TYPE))
            Logger::Log("Light #" + lightName + " has no type! "
                "Supported are: 'directional', 'point' and 'spot'", LOGTYPE_ERROR);

        std::string type = lightProps[JSON_NAME_LIGHT_TYPE];

        Color       col = parseColor(lightProps, Color::WHITE);
        ShadowInfo* shadowInfo = parseShadowInfo(lightProps);

        Light* light = nullptr;
        if (type == JSON_NAME_LIGHT_DIRECTIONAL)
        {
            Vec3f direction = parseVec3f(lightProps, JSON_NAME_LIGHT_DIRECTION, Vec3f(0, -1, 0));
            float intensity = parsePrimitive<float>(lightProps, JSON_NAME_LIGHT_INTENSITY, 2.0f);

            light = new DirectionalLight(col, intensity, direction, shadowInfo);
            light->setName(lightName);
        }
        else if (type == JSON_NAME_LIGHT_POINT)
        {
            Point3f position = parsePoint3f(lightProps, JSON_NAME_POSITION, Point3f(0, 0, 0));
            Vec3f attenuation = parseVec3f(lightProps, JSON_NAME_LIGHT_ATTENUATION, Vec3f(0, 0, 1));
            float intensity = parsePrimitive<float>(lightProps, JSON_NAME_LIGHT_INTENSITY, 300.0f);

            light = new PointLight(col, intensity, attenuation, position, shadowInfo);
            light->setName(lightName);
        }
        else if (type == JSON_NAME_LIGHT_SPOT)
        {
            Vec3f direction = parseVec3f(lightProps, JSON_NAME_LIGHT_DIRECTION, Vec3f(1, 0, 0));
            Point3f position = parsePoint3f(lightProps, JSON_NAME_POSITION, Point3f(0, 0, 0));
            Vec3f attenuation = parseVec3f(lightProps, JSON_NAME_LIGHT_ATTENUATION, Vec3f(0, 0, 1));
            float intensity = parsePrimitive<float>(lightProps, JSON_NAME_LIGHT_INTENSITY, 500.0f);
            float fov = parsePrimitive<float>(lightProps, JSON_NAME_LIGHT_SPOT_FOV, 90.0f);

            light = new SpotLight(col, intensity, fov, attenuation, position, direction, shadowInfo);
            light->setName(lightName);
        }
        else
        {
            delete shadowInfo;
            Logger::Log("Light-Type '" + type + "' is unknown. Supported are: 'directional', 'point' and 'spot'", LOGTYPE_WARNING);
        }
        lights[lightName] = light;
    }

    void JSONScene::createCamera(RenderingEngine* renderer, const JSON& json)
    {
        Camera::EMode cameraMode(Camera::PERSPECTIVE);
        Transform transform = parseTransform(json);

        if (isExistent(json, JSON_NAME_CAMERA_MODE))
        {
            auto m = json[JSON_NAME_CAMERA_MODE];
            if (m == JSON_NAME_CAMERA_MODE_ORTHOGRAPHIC)
                cameraMode = Camera::ORTHOGRAPHIC;
            else if (m == JSON_NAME_CAMERA_MODE_PERSPECTIVE)
                cameraMode = Camera::PERSPECTIVE;
        }

        camera = new Camera(transform, cameraMode);

        if (json.count(JSON_NAME_CAMERA_FOV) != 0)
        {
            float fov = parsePrimitive<float>(json, JSON_NAME_CAMERA_FOV, 60.0f);
            camera->setFOV(fov);
        }
#if ADD_CAMERA_COMPONENTS
        camera->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        Node* generalScriptNode = new Node("InteractionMaster");
        generalScriptNode->addComponent(new CInteract(70.0f));
#endif
        renderer->setCamera(camera);
    }

    void JSONScene::createForwardShaders(const JSON& json)
    {
        for (auto it = json.begin(); it != json.end(); it++)
        {
            std::string shaderName =it.key();
            std::string shaderPath = it.value();
            createNewForwardShader(shaderName, shaderPath);
        }
    }

    void JSONScene::createNewForwardShader(const std::string& name, const std::string& path)
    {
        std::string shaderName = getFullShaderName(name);
        if (!SHADER_EXISTS(shaderName))
        {
            ForwardShaderParams shaderParams(shaderName, path);
            forwardShaders[shaderName] = FORWARD_SHADER(shaderParams);
        }
        else
        {
            Logger::Log("JSONScene::createNewShader(): Shader with name '" + name + "' already exist. "
                        "Can't create another one.", LOGTYPE_WARNING);
        }
    }

    TexturePtr JSONScene::parseTexture(const std::string& virtualPath)
    {
        if (VFS::fileExists(virtualPath))
            return TEXTURE(virtualPath);
        else
        {
            Logger::Log("JSONScene::parseTexture(): Can't find texture with path '" + virtualPath
                + ". Using default Texture instead.", LOGTYPE_WARNING);
            return TEXTURE_GET(TEX_DEFAULT);
        }
    }

    TexturePtr JSONScene::parseTexture(const JSON& json, const std::string& name)
    {
        if (json.count(name) == 0)
            return nullptr;

        std::string virtualPath = json[name];
        return parseTexture(virtualPath);
    }

    CubemapPtr JSONScene::parseCubemap(const std::string& virtualPath)
    {
        return CUBEMAP(virtualPath);
    }

    CubemapPtr JSONScene::parseCubemap(const JSON& json, const std::string& name)
    {
        if (json.count(name) == 0)
            return nullptr;

        std::string path = json[name];
        return parseCubemap(path);
    }

    Transform JSONScene::parseTransform(const JSON& json)
    {
        Transform trans(Point3f(0, 0, 0), Vec3f(1, 1, 1), Quatf::identity);

        if (isExistent(json, JSON_NAME_TRANSFORM))
        {
            auto jsonTrans = json[JSON_NAME_TRANSFORM];
            trans.position = parsePoint3f(jsonTrans, JSON_NAME_POSITION);
            trans.scale = parseVec3f(jsonTrans, JSON_NAME_SCALE);
            trans.rotation = parseVec3f(jsonTrans, JSON_NAME_ROTATION).toQuaternion();
        }

        return trans;
    }

    Point3f JSONScene::parsePoint3f(const JSON& json, const char* name, const Point3f& p)
    {
        if (!isExistent(json, name))
            return p;

        auto vec = json[name];
        return Point3f(vec[JSON_NAME_X_COMPONENT], vec[JSON_NAME_Y_COMPONENT], vec[JSON_NAME_Z_COMPONENT]);
    }

    Vec3f JSONScene::parseVec3f(const JSON& json, const char* name, const Vec3f& v)
    {
        if (!isExistent(json, name))
            return v;

        auto vec = json[name];
        return parseRawVec3f(vec);
    }

    Vec3f JSONScene::parseRawVec3f(const JSON& json)
    {
        return Vec3f(json[JSON_NAME_X_COMPONENT], json[JSON_NAME_Y_COMPONENT], json[JSON_NAME_Z_COMPONENT]);
    }

    Vec4f JSONScene::parseVec4f(const JSON& json, const char* name, const Vec4f& v)
    {
        if (!isExistent(json, name))
            return v;

        auto vec = json[name];
        return parseRawVec4f(vec);
    }

    Vec4f JSONScene::parseRawVec4f(const JSON& v)
    {
        return Vec4f(v[JSON_NAME_X_COMPONENT], v[JSON_NAME_Y_COMPONENT], v[JSON_NAME_Z_COMPONENT], v[JSON_NAME_W_COMPONENT]);
    }

    Color JSONScene::parseColor(const JSON& json, const Color& color)
    {
        Color c = color;
        if (json.count(JSON_NAME_COLOR) != 0)
        {
            auto col = json[JSON_NAME_COLOR];
            return parseRawColor(col);
        }
        return c;
    }

    Color JSONScene::parseRawColor(const JSON& col)
    {
        Color c;
        if (col.is_object())
        {
            c = Color(col[JSON_NAME_COLOR_R_COMPONENT], col[JSON_NAME_COLOR_G_COMPONENT], col[JSON_NAME_COLOR_B_COMPONENT]);
            if (col.count(JSON_NAME_COLOR_A_COMPONENT))
                c.a() = col[JSON_NAME_COLOR_A_COMPONENT];
        }
        else if (col.is_string())
        {
            std::string hex = col;
            c = Color(hex);
        }
        return c;
    }

    std::string JSONScene::parseString(const JSON& json)
    {
        return json.is_string() ? json : "";
    }

    ShadowInfo* JSONScene::parseShadowInfo(const JSON& lightProps)
    {
        if (isExistent(lightProps, JSON_NAME_LIGHT_SHADOW_INFO))
        {
            auto shadowProps = lightProps[JSON_NAME_LIGHT_SHADOW_INFO];

            uint32_t size = parsePrimitive<uint32_t>(shadowProps, JSON_NAME_LIGHT_SHADOW_INFO_SIZE, 10);
            float blurStrength = parsePrimitive<float>(shadowProps, JSON_NAME_LIGHT_SHADOW_INFO_BLUR, 1.0f);
            float distance = parsePrimitive<float>(shadowProps, JSON_NAME_LIGHT_SHADOW_INFO_DISTANCE, 100.0f);

            return new ShadowInfo(size, blurStrength, distance);
        }
        return nullptr;
    }

    TexturePtr JSONScene::getTexture(const std::string& virtualPath)
    {
        std::string physicalPath = VFS::resolvePhysicalPath(virtualPath);
        if (!FileSystem::fileExists(physicalPath))
        {
            Logger::Log("Texture '" + physicalPath + "' does not exist. Using default texture instead.", LOGTYPE_WARNING);
            return TEXTURE_GET(TEX_DEFAULT);
        }
        return TEXTURE(physicalPath);
    }

    MeshPtr JSONScene::getMesh(const std::string& name)
    {
        if (meshes.count(name) == 0)
        {
            Logger::Log("JSONScene::getMesh(): Could not find mesh with name '" + name + "'. "
                "Using a cube instead", LOGTYPE_WARNING);
            return MESH(MODEL_CUBE_DEFAULT);
        }
        return meshes[name];
    }

    MaterialPtr JSONScene::getMaterial(const std::string& name)
    {
        if (materials.count(name) == 0)
        {
            Logger::Log("JSONScene::getMaterial(): Could not find material with name '" + name + "'. "
                "Using default one instead", LOGTYPE_WARNING);
            return MATERIAL_GET(MATERIAL_DEFAULT);
        }
        return materials[name];
    }

    JSONScene::OP JSONScene::stringToOP(const std::string& op)
    {
        if (op == "replace")
            return OP::REPLACE;
        if (op == "add")
            return OP::ADD;
        if (op == "remove")
            return OP::REMOVE;

        return OP::UNKNOWN;
    }

    void JSONScene::resetMaterialsOfObjects()
    {
        if (json.count(JSON_NAME_OBJECTS) != 0)
        {
            auto jsonObjects = json[JSON_NAME_OBJECTS];
            for (auto obj = jsonObjects.begin(); obj != jsonObjects.end(); obj++)
            {
                std::string name = obj.key();
                auto objProps = obj.value();
                if (objProps.count(JSON_NAME_MATERIAL) != 0)
                    if (objects[name] != nullptr)
                        objects[name]->setMaterial(materials[objProps[JSON_NAME_MATERIAL]]);
            }
        }
    }

    void JSONScene::resetModelsOfObjects()
    {
        if (json.count(JSON_NAME_OBJECTS) != 0)
        {
            auto jsonObjects = json[JSON_NAME_OBJECTS];
            for (auto obj = jsonObjects.begin(); obj != jsonObjects.end(); obj++)
            {
                std::string name = obj.key();
                auto objProps = obj.value();
                if (objProps.count(JSON_NAME_MODEL) != 0)
                    if (objects[name] != nullptr)
                        objects[name]->setMesh(meshes[objProps[JSON_NAME_MODEL]]);
            }
        }
    }

    //---------------------------------------------------------------------------
    //  Private Methods - REPLACEMENTS
    //---------------------------------------------------------------------------

    void JSONScene::replaceValue(PathManager& pathManager, const JSON::value_type& val)
    {
        std::string currentPath = pathManager.next();
        if (currentPath == JSON_NAME_CAMERA)
        {
            replaceValueInCamera(pathManager, val);
        }
        else if (currentPath == JSON_NAME_OBJECTS)
        {
            replaceValueInObject(pathManager, val);
        }
        else if (currentPath == JSON_NAME_SKYBOX)
        {
            replaceValueInSkybox(pathManager, val);
        }
        else if (currentPath == JSON_NAME_LIGHTS)
        {
            replaceValueInLight(pathManager, val);
        }
        else if (currentPath == JSON_NAME_MODELS)
        {
            replaceValueInModel(pathManager, val);
        }
        else if (currentPath == JSON_NAME_MATERIALS)
        {
            replaceValueInMaterial(pathManager, val);
        }
        else if (currentPath == JSON_NAME_SHADERS)
        {
            replaceValueInShader(pathManager, val);
        }
        else if (currentPath == JSON_NAME_SETTINGS)
        {
            replaceValueInSettings(pathManager, val);
        }
    }

    void JSONScene::replaceValueInObject(PathManager& pathManager, const JSON::value_type& val)
    {
        const std::string& objectName = pathManager.next();
        auto obj = objects[objectName];

        std::string objectFieldName = pathManager.next();
        if (objectFieldName == JSON_NAME_MODEL)
        {
            obj->setMesh(getMesh(val));
        }
        else if (objectFieldName == JSON_NAME_MATERIAL)
        {
            obj->setMaterial(getMaterial(val));
        }
        else if (objectFieldName == JSON_NAME_TRANSFORM)
        {
            replaceTransformValue(obj, pathManager, val);
        }
    }

    void JSONScene::replaceValueInCamera(PathManager& pathManager, const JSON::value_type& val)
    {
        std::string cameraFieldName = pathManager.next();
        if (cameraFieldName == JSON_NAME_CAMERA_MODE)
        {
            Camera::EMode renderingMode = Camera::PERSPECTIVE;
            if (val == JSON_NAME_CAMERA_MODE_ORTHOGRAPHIC)
                renderingMode = Camera::ORTHOGRAPHIC;
            camera->setRenderingMode(renderingMode);
        }
        else if (cameraFieldName == JSON_NAME_TRANSFORM)
        {
            replaceTransformValue(camera, pathManager, val);
        }
        else if (cameraFieldName == JSON_NAME_CAMERA_FOV)
        {
            camera->setFOV(val);
        }
    }

    void JSONScene::replaceValueInSkybox(PathManager& pathManager, const JSON::value_type& val)
    {
        std::string cubemapPath = val;
        CubemapPtr cube = CUBEMAP(cubemapPath);
        skybox->setCubemap(cube);
    }

    void JSONScene::replaceValueInLight(PathManager& pathManager, const JSON::value_type& val)
    {
        const std::string& lightName = pathManager.next();
        Light* light = lights[lightName];

        const std::string& fieldName = pathManager.next();
        if (fieldName == JSON_NAME_LIGHT_TYPE)
        {
            delete light;
            createNewLight(lightName, json[JSON_NAME_LIGHTS][lightName]);
        }
        else if (fieldName == JSON_NAME_LIGHT_INTENSITY)
        {
            light->setIntensity(val);
        }
        else if (fieldName == JSON_NAME_COLOR)
        {
            if (val.is_object()) // From hex to { "r": 0, "g" : 0, "b": 1 }
            {
                Color& currentColor = light->getColor();
                currentColor.r() = val[JSON_NAME_COLOR_R_COMPONENT];
                currentColor.g() = val[JSON_NAME_COLOR_G_COMPONENT];
                currentColor.b() = val[JSON_NAME_COLOR_B_COMPONENT];
                if (val.count(JSON_NAME_COLOR_A_COMPONENT) != 0)
                    currentColor.a() = val;
            }
            else
            {
                if (pathManager.hasNext())
                {
                    Color& currentColor = light->getColor();

                    const std::string& colField = pathManager.next();
                    if (colField == JSON_NAME_COLOR_R_COMPONENT)
                        currentColor.r() = val;
                    else if (colField == JSON_NAME_COLOR_G_COMPONENT)
                        currentColor.g() = val;
                    else if (colField == JSON_NAME_COLOR_B_COMPONENT)
                        currentColor.b() = val;
                    else if (colField == JSON_NAME_COLOR_A_COMPONENT)
                        currentColor.a() = val;
                }
                else
                {
                    std::string hex = val;
                    light->setColor(Color(hex));
                }
            }
        }
        else if (fieldName == JSON_NAME_LIGHT_DIRECTION)
        {
            Vec3f direction;
            switch (light->getLightType())
            {
                case Light::DirectionalLight:
                {
                    auto dirLight = dynamic_cast<DirectionalLight*>(light);
                    Vec3f newDir = getVec3WithModifiedComponent(pathManager.next(), dirLight->getDirection(), val);
                    dirLight->setDirection(newDir);
                    break;
                }
                case Light::SpotLight:
                {
                    auto spotLight = dynamic_cast<SpotLight*>(light);
                    Vec3f newDir = getVec3WithModifiedComponent(pathManager.next(), spotLight->getDirection(), val);
                    spotLight->setDirection(newDir);
                    break;
                }
            }
        }
        else if (fieldName == JSON_NAME_LIGHT_ATTENUATION)
        {
            PointLight* pl = dynamic_cast<PointLight*>(light);
            Vec3f newAttenuation = getVec3WithModifiedComponent(pathManager.next(), pl->getAttenuation(), val);
            pl->setAttenuation(newAttenuation);
        }
        else if (fieldName == JSON_NAME_POSITION)
        {
            PointLight* pl = dynamic_cast<PointLight*>(light);
            if (val.is_object())
            {
                pl->setPosition(Point3f(val[JSON_NAME_X_COMPONENT], val[JSON_NAME_Y_COMPONENT], val[JSON_NAME_Z_COMPONENT]));
            }
            else
            {
                Point3f newPos = (Point3f)getVec3WithModifiedComponent(pathManager.next(), pl->getWorldPosition(), val);
                pl->setPosition(newPos);
            }
        }
        else if (fieldName == JSON_NAME_LIGHT_SPOT_FOV)
        {
            SpotLight* spot = dynamic_cast<SpotLight*>(light);
            spot->setFOV(val);
        }
        else if (fieldName == JSON_NAME_LIGHT_SHADOW_INFO)
        {
            if (pathManager.hasNext())
            {
                const std::string& shadowInfoField = pathManager.next();

                if (shadowInfoField == JSON_NAME_LIGHT_SHADOW_INFO_BLUR)
                {
                    light->setShadowBlurScale(val);
                }
                else if (shadowInfoField == JSON_NAME_LIGHT_SHADOW_INFO_DISTANCE)
                {
                    light->setShadowDistance(val);
                }
                else if (shadowInfoField == JSON_NAME_LIGHT_SHADOW_INFO_SIZE)
                {
                    Logger::Log("Modifying shadowmap size is not supported at runtime. Sorry :(", LOGTYPE_WARNING);
                }
            }
            else
            {
                light->setShadows(true);
            }
        }
    }

    void JSONScene::replaceValueInModel(PathManager& pathManager, const JSON::value_type& val)
    {
        const std::string& modelName = pathManager.next();
        MeshPtr previousMesh = meshes[modelName];
        meshes[modelName] = MESH(val);
        for (auto& pair : objects)
        {
            Renderable* obj = pair.second;
            if (obj->getMesh() == previousMesh)
                obj->setMesh(meshes[modelName]);
        }
    }

    void JSONScene::replaceValueInMaterial(PathManager& pathManager, const JSON::value_type& val)
    {
        const std::string& matName = pathManager.next();
        MaterialPtr mat = materials[matName];
        PBRMaterialPtr pbrMat = static_cast<PBRMaterialPtr>(mat);

        const std::string& fieldName = pathManager.next();
        if (fieldName == JSON_NAME_MATERIAL_UVSCALE)
        {
            pbrMat->setMatUVScale(val);
        }
        else if (fieldName == JSON_NAME_COLOR && pbrMat.get() != nullptr)
        {
            if (val.is_object()) // From hex to { "r": 0, "g" : 0, "b": 1 }
            {
                Color newColor = pbrMat->getMatColor();
                newColor.r() = val[JSON_NAME_COLOR_R_COMPONENT];
                newColor.g() = val[JSON_NAME_COLOR_G_COMPONENT];
                newColor.b() = val[JSON_NAME_COLOR_B_COMPONENT];
                if (val.count(JSON_NAME_COLOR_A_COMPONENT) != 0)
                    newColor.a() = val;
                pbrMat->setMatColor(newColor);
            }
            else
            {
                if (pathManager.hasNext())
                {
                    Color newColor = pbrMat->getMatColor();

                    const std::string& colField = pathManager.next();
                    if (colField == JSON_NAME_COLOR_R_COMPONENT)
                        newColor.r() = val;
                    else if (colField == JSON_NAME_COLOR_G_COMPONENT)
                        newColor.g() = val;
                    else if (colField == JSON_NAME_COLOR_B_COMPONENT)
                        newColor.b() = val;
                    else if (colField == JSON_NAME_COLOR_A_COMPONENT)
                        newColor.a() = val;

                    pbrMat->setMatColor(newColor);
                }
                else
                {
                    std::string hex = val;
                    pbrMat->setMatColor(Color(hex));
                }
            }
        }
        else if (fieldName == JSON_NAME_MATERIAL_DIFFUSE)
        {
            std::string filePath = val;
            auto tex = getTexture(filePath);
            pbrMat->setMatDiffuseTexture(tex);
        }
        else if (fieldName == JSON_NAME_MATERIAL_NORMAL)
        {
            std::string filePath = val;
            auto tex = getTexture(filePath);
            pbrMat->setMatNormalMap(tex);
        }
        else if (fieldName == JSON_NAME_MATERIAL_AO)
        {
            std::string filePath = val;
            auto tex = getTexture(filePath);
            pbrMat->setMatAOMap(tex);
        }
        else if (fieldName == JSON_NAME_MATERIAL_METALLIC)
        {
            if (val.is_number())
            {
                pbrMat->setMatMetallic(val);
            }
            else
            {
                std::string filePath = val;
                auto tex = getTexture(filePath);
                pbrMat->setMatMetallicMap(tex);
            }
        }
        else if (fieldName == JSON_NAME_MATERIAL_ROUGHNESS)
        {
            if (val.is_number())
            {
                pbrMat->setMatRoughness(val);
            }
            else
            {
                std::string filePath = val;
                auto tex = getTexture(filePath);
                pbrMat->setMatRoughnessMap(tex);
            }
        }
        else if (fieldName == JSON_NAME_SHADER)
        {
            std::string newShaderName = val;
            std::string shaderName = getName() + "#" + newShaderName;
            if (forwardShaders.count(shaderName) == 0)
                Logger::Log("JSONScene::replaceValueInMaterial(): "
                    "Could not find a shader with name '" + shaderName + "'", LOGTYPE_ERROR);
            mat->setShader(forwardShaders[shaderName]);
            readdForwardShaderMaterialParams(mat);
        }
        else
        {
            // Param was maybe part of a forward-shader, so try to add it.
            addParamToForwardShaderMaterial(mat, fieldName, val);
        }
    }

    void JSONScene::readdForwardShaderMaterialParams(MaterialPtr material)
    {
        // Add props to material
        auto& matProps = json[JSON_NAME_MATERIALS][material->getName()];
        for (auto it = matProps.begin(); it != matProps.end(); it++)
        {
            std::string propName = it.key();
            auto val = it.value();
            if (propName == "shader")
                continue;

            addParamToForwardShaderMaterial(materials[material->getName()], propName, val);
        }
    }

    void JSONScene::replaceValueInShader(PathManager& pathManager, const JSON::value_type& val)
    {
        const std::string& shaderName = getFullShaderName( pathManager.next() );
        assert(forwardShaders.count(shaderName) != 0);

        // Make sure the old forward shader has been destroyed
        const auto& materialsFromPreviousShader = forwardShaders[shaderName]->getMaterialsFromCurrentScene();
        for (auto& mat : materialsFromPreviousShader)
            mat->setShader(nullptr);
        forwardShaders.erase(shaderName);

        // Create a new forward shader and attach all materials to it
        createNewForwardShader(shaderName, val);
        for (auto& material : materialsFromPreviousShader)
        {
            material->setShader(forwardShaders[shaderName]);

            // Add props to material TODO -> descriptorSetLayout throws exceptions in material
            // readdForwardShaderMaterialParams(materials[material->getName()]);
        }
    }

    void JSONScene::replaceTransformValue(Node* obj, PathManager& pathManager, const JSON::value_type& val)
    {
        const std::string& transformationFieldName = pathManager.next();
        if (transformationFieldName == JSON_NAME_POSITION)
        {
            const std::string& xyz = pathManager.next();
            if (xyz == "x")
                obj->getTransform().position.x() = val;
            else if (xyz == "y")
                obj->getTransform().position.y() = val;
            else if (xyz == "z")
                obj->getTransform().position.z() = val;
        }
        else if (transformationFieldName == JSON_NAME_SCALE)
        {
            const std::string& xyz = pathManager.next();
            if (xyz == "x")
                obj->getTransform().scale.x() = val;
            else if (xyz == "y")
                obj->getTransform().scale.y() = val;
            else if (xyz == "z")
                obj->getTransform().scale.z() = val;
        }
        else if (transformationFieldName == JSON_NAME_ROTATION)
        {
            Vec3f rot = obj->getTransform().rotation.toEulerAngles();
            const std::string& xyz = pathManager.next();
            if (xyz == "x")
                rot.x() = val;
            else if (xyz == "y")
                rot.y() = val;
            else if (xyz == "z")
                rot.z() = val;
            obj->getTransform().rotation = rot.toQuaternion();
        }
    }

    void JSONScene::replaceValueInSettings(PathManager& pathManager, const JSON::value_type& val)
    {
        // Settings will be reloaded anyway so just check here if something from the
        // environment maps has changed and set a flag to true, so the next time the
        // scene gets reloaded, env + prem map will be rerendered.
        if (pathManager.hasNext())
        {
            const std::string& settingName = pathManager.next();
            if (settingName == JSON_NAME_IBL)
            {
                if (pathManager.hasNext())
                    {
                    const std::string& iblSettingName = pathManager.next();

                    if (pathManager.hasNext())
                    {
                        if (iblSettingName == JSON_NAME_ENVIRONMENT_MAP)
                        {
                            renderEnvironmentMaps = true;
                        }
                    }
                }
            }
            else if (settingName == JSON_NAME_SAMPLER)
            {
                Logger::Log("JSONScene::modify(): Setting the sampler at runtime does only effect "
                            "subsequent created textures.", LOGTYPE_WARNING);
            }
        }
    }

    //---------------------------------------------------------------------------
    //  Private Methods - REMOVING
    //---------------------------------------------------------------------------

    void JSONScene::removeValue(PathManager& pathManager)
    {
        const std::string& currentPath = pathManager.next();
        if (currentPath == JSON_NAME_CAMERA)
        {
            removeValueInCamera(pathManager);
        }
        else if (currentPath == JSON_NAME_OBJECTS)
        {
            removeValueInObject(pathManager);
        }
        else if (currentPath == JSON_NAME_SKYBOX)
        {
            removeValueInSkybox(pathManager);
        }
        else if (currentPath == JSON_NAME_LIGHTS)
        {
            removeValueInLight(pathManager);
        }
        else if (currentPath == JSON_NAME_MODELS)
        {
            removeValueInModel(pathManager);
        }
        else if (currentPath == JSON_NAME_MATERIALS)
        {
            removeValueInMaterial(pathManager);
        }
        else if (currentPath == JSON_NAME_SHADERS)
        {
            removeValueInShader(pathManager);
        }
    }

    void JSONScene::removeValueInCamera(PathManager& pathManager)
    {
        if (pathManager.hasNext())
        {
            const std::string& fieldName = pathManager.next();
            if (fieldName == JSON_NAME_CAMERA_MODE)
            {
                camera->setRenderingMode(Camera::PERSPECTIVE);
            }
            else if (fieldName == JSON_NAME_TRANSFORM)
            {
                // do nothing
            }
            else if (fieldName == JSON_NAME_CAMERA_FOV)
            {
                camera->setFOV(60);
            }
        }
        else
        {
            Logger::Log("JSONScene::removeValueInCamera(): It is not possible to delete the camera.", LOGTYPE_WARNING);
        }
    }

    void JSONScene::removeValueInObject(PathManager& pathManager)
    {
        if (!pathManager.hasNext())
        {
            // /objects removed -> Delete all objects
            for (auto& objPair : objects)
                delete objPair.second;
            objects.clear();
        }
        else
        {
            const std::string& objectName = pathManager.next();
            auto& obj = objects[objectName];

            if (!pathManager.hasNext())
            {
                delete obj;
                objects.erase(objectName);
            }
            else
            {
                const std::string& fieldName = pathManager.next();
                if (fieldName == JSON_NAME_MODEL)
                {
                    obj->setMesh(MESH(MODEL_CUBE_DEFAULT));
                }
                else if (fieldName == JSON_NAME_MATERIAL)
                {
                    obj->setMaterial(MATERIAL_GET(MATERIAL_DEFAULT));
                }
                else if (fieldName == JSON_NAME_TRANSFORM)
                {
                    // Do nothing. Other option would be to reset it to default.
                }
            }
        }
    }

    void JSONScene::removeValueInSkybox(PathManager& pathManager)
    {
        if (pathManager.hasNext())
        {
            const std::string& fieldName = pathManager.next();
            if (fieldName == JSON_NAME_CUBEMAP)
            {
                skybox->setCubemap(CUBEMAP_GET(CUBEMAP_DEFAULT));
            }
        }
        else
        {
            delete skybox;
            skybox = nullptr;
        }
    }

    void JSONScene::removeValueInLight(PathManager& pathManager)
    {
        if (!pathManager.hasNext())
        {
            for (auto& light : lights)
                delete light.second;
            lights.clear();
            return;
        }

        const std::string& lightName = pathManager.next();
        Light* light = lights[lightName];

        if (!pathManager.hasNext())
        {
            delete light;
            lights.erase(lightName);
        }
        else
        {
            const std::string& fieldName = pathManager.next();
            if (fieldName == JSON_NAME_LIGHT_SHADOW_INFO)
            {
                light->setShadows(false);
            }
            else if (fieldName == JSON_NAME_COLOR)
            {
                light->setColor(Color::WHITE);
            }
            else if (fieldName == JSON_NAME_LIGHT_TYPE)
            {
            }
            else if (fieldName == JSON_NAME_LIGHT_INTENSITY)
            {
            }
            else if (fieldName == JSON_NAME_LIGHT_DIRECTION)
            {
            }
            else if (fieldName == JSON_NAME_LIGHT_ATTENUATION)
            {
            }
            else if (fieldName == JSON_NAME_POSITION)
            {
            }
            else if (fieldName == JSON_NAME_LIGHT_SPOT_FOV)
            {
            }
        }
    }

    void JSONScene::removeValueInModel(PathManager& pathManager)
    {
        if (pathManager.hasNext())
        {
            const std::string& modelName = pathManager.next();
            MeshPtr previousMesh = meshes[modelName];
            meshes.erase(modelName);
            for (auto& pair : objects)
            {
                Renderable* obj = pair.second;
                if (obj->getMesh() == previousMesh)
                    obj->setMesh(MESH(MODEL_CUBE_DEFAULT));
            }
        }
        else
        {
            for (auto& pair : objects)
            {
                Renderable* obj = pair.second;
                obj->setMesh(MESH(MODEL_CUBE_DEFAULT));
            }
            meshes.clear();
        }
    }

    void JSONScene::removeValueInMaterial(PathManager& pathManager)
    {
        if (!pathManager.hasNext())
        {
            for (auto& obj : objects)
                obj.second->setMaterial(nullptr);
            materials.clear();
            return;
        }

        const std::string& matName = pathManager.next();
        MaterialPtr mat = materials[matName];
        PBRMaterialPtr pbrMat = static_cast<PBRMaterialPtr>(mat);

        if (!pathManager.hasNext())
        {
            for (auto& obj : objects)
                if (obj.second->getMaterial() == mat)
                    obj.second->setMaterial(nullptr);
            materials.erase(matName);
        }
        else {
            const std::string& fieldName = pathManager.next();
            if (fieldName == JSON_NAME_MATERIAL_UVSCALE)
            {
                pbrMat->setMatUVScale(1.0f);
            }
            else if (fieldName == JSON_NAME_COLOR && pbrMat.get() != nullptr)
            {
                pbrMat->setMatColor(Color::WHITE);
            }
            else if (fieldName == JSON_NAME_MATERIAL_DIFFUSE)
            {
                pbrMat->setMatDiffuseTexture(nullptr);
            }
            else if (fieldName == JSON_NAME_MATERIAL_NORMAL)
            {
                pbrMat->setMatNormalMap(nullptr);
            }
            else if (fieldName == JSON_NAME_MATERIAL_AO)
            {
                pbrMat->setMatAOMap(nullptr);
            }
            else if (fieldName == JSON_NAME_MATERIAL_METALLIC)
            {
                pbrMat->hasMetallicMap() ? pbrMat->setMatMetallicMap(nullptr) : pbrMat->setMatMetallic(0.0f);
            }
            else if (fieldName == JSON_NAME_MATERIAL_ROUGHNESS)
            {
                pbrMat->hasRoughnessMap() ? pbrMat->setMatRoughnessMap(nullptr) : pbrMat->setMatRoughness(1.0f);
            }
            else if (fieldName == JSON_NAME_SHADER)
            {
                mat->setShader(SHADER(SHADER_GBUFFER));
            }
        }
    }

    void JSONScene::removeValueInShader(PathManager& pathManager)
    {
        if (!pathManager.hasNext())
        {
            forwardShaders.clear();
        }
        else
        {
            const std::string& shaderName = getFullShaderName(pathManager.next());
            forwardShaders.erase(shaderName);
        }
        Logger::Log("JSONScene::remove(): Explicit deletion of shaders are not supported. "
             "A shader will be automatically deleted if all materials using it are deleted.", LOGTYPE_WARNING);
    }

    //---------------------------------------------------------------------------
    //  Private Methods - ADDING
    //---------------------------------------------------------------------------

    void JSONScene::addValue(PathManager& pathManager, const JSON::value_type& val)
    {
        const std::string& currentPath = pathManager.next();
        if (currentPath == JSON_NAME_CAMERA)
        {
            addValueInCamera(pathManager, val);
        }
        else if (currentPath == JSON_NAME_OBJECTS)
        {
            addValueInObject(pathManager, val);
        }
        else if (currentPath == JSON_NAME_SKYBOX)
        {
            addValueInSkybox(pathManager, val);
        }
        else if (currentPath == JSON_NAME_LIGHTS)
        {
            addValueInLight(pathManager, val);
        }
        else if (currentPath == JSON_NAME_MODELS)
        {
            addValueInModel(pathManager, val);
        }
        else if (currentPath == JSON_NAME_MATERIALS)
        {
            addValueInMaterial(pathManager, val);
        }
        else if (currentPath == JSON_NAME_SHADERS)
        {
            addValueInShader(pathManager, val);
        }
    }

    void JSONScene::addValueInObject(PathManager& pathManager, const JSON::value_type& val)
    {
        if (!pathManager.hasNext())
        {
            if (val.is_object()) // Several objects were added
            {
                createObjects(val);
            }
            return;
        }

        const std::string& objectName = pathManager.next();
        if (!pathManager.hasNext())
        {
            createNewObject(objectName, val);
        }
        else
        {
            auto obj = objects[objectName];

            std::string objectFieldName = pathManager.next();
            if (objectFieldName == JSON_NAME_MODEL)
            {
                obj->setMesh(getMesh(val));
            }
            else if (objectFieldName == JSON_NAME_MATERIAL)
            {
                obj->setMaterial(getMaterial(val));
            }
            else if (objectFieldName == JSON_NAME_TRANSFORM)
            {
                addTransformValue(obj, pathManager, val);
            }
        }
    }

    void JSONScene::addTransformValue(Node* obj, PathManager& pathManager, const JSON::value_type& val)
    {
        if (pathManager.hasNext())
        {
            const std::string& transFieldName = pathManager.next();

            if (!pathManager.hasNext())
            {
                // position, scale or rotation added
                if (transFieldName == JSON_NAME_POSITION)
                {
                    obj->getTransform().position = Point3f(val[JSON_NAME_X_COMPONENT], val[JSON_NAME_Y_COMPONENT], val[JSON_NAME_Z_COMPONENT]);
                }
                else if (transFieldName == JSON_NAME_SCALE)
                {
                    obj->getTransform().scale = Vec3f(val[JSON_NAME_X_COMPONENT], val[JSON_NAME_Y_COMPONENT], val[JSON_NAME_Z_COMPONENT]);
                }
                else if (transFieldName == JSON_NAME_ROTATION)
                {
                    obj->getTransform().rotation = Vec3f(val[JSON_NAME_X_COMPONENT], val[JSON_NAME_Y_COMPONENT], val[JSON_NAME_Z_COMPONENT]).toQuaternion();
                }
            }
            else
            {
                // Single component added 'x', 'y' or 'z'
            }
        }
    }

    void JSONScene::addValueInCamera(PathManager& pathManager, const JSON::value_type& val)
    {
        if (pathManager.hasNext())
        {
            std::string cameraFieldName = pathManager.next();
            if (cameraFieldName == JSON_NAME_CAMERA_MODE)
            {
                Camera::EMode renderingMode = Camera::PERSPECTIVE;
                if (val == JSON_NAME_CAMERA_MODE_ORTHOGRAPHIC)
                    renderingMode = Camera::ORTHOGRAPHIC;
                camera->setRenderingMode(renderingMode);
            }
            else if (cameraFieldName == JSON_NAME_CAMERA_FOV)
            {
                camera->setFOV(val);
            }
            else if (cameraFieldName == JSON_NAME_TRANSFORM)
            {
                addTransformValue(camera, pathManager, val);
            }
        }
    }

    void JSONScene::addValueInSkybox(PathManager& pathManager, const JSON::value_type& val)
    {
        if (pathManager.hasNext())
        {
            replaceValueInSkybox(pathManager, val);
        }
        else
        {
            createSkybox(val);
        }
    }

    void JSONScene::addValueInLight(PathManager& pathManager, const JSON::value_type& val)
    {
        if (!pathManager.hasNext())
        {
            if (val.is_object()) // several lights were added
            {
                createLights(val);
            }
            return;
        }

        const std::string& lightName = pathManager.next();
        if (!pathManager.hasNext()) // a completely new light was added
        {
            createNewLight(lightName, val);
        }
        else // Fields in an existing light were added
        {
            pathManager.revertOne();
            replaceValueInLight(pathManager, val);
        }
    }

    void JSONScene::addValueInModel(PathManager& pathManager, const JSON::value_type& val)
    {
        if (!pathManager.hasNext())
        {
            if (val.is_object())
            {
                createModels(val);
                resetModelsOfObjects();
            }
        }
        else
        {
            const std::string& modelName = pathManager.next();
            meshes[modelName] = MESH(val);
            resetModelsOfObjects();
        }
    }

    void JSONScene::addValueInMaterial(PathManager& pathManager, const JSON::value_type& val)
    {
        if (!pathManager.hasNext())
        {
            if (val.is_object()) // several new materials were added
            {
                createMaterials(val);
                resetMaterialsOfObjects();
            }
            return;
        }

        const std::string& matName = pathManager.next();
        if (!pathManager.hasNext()) // new material was added
        {
            createNewMaterial(matName, val);
            resetMaterialsOfObjects();
        }
        else
        {
            pathManager.revertOne();
            replaceValueInMaterial(pathManager, val);
        }
    }

    void JSONScene::addValueInShader(PathManager& pathManager, const JSON::value_type& val)
    {
        if (!pathManager.hasNext())
        {
            if (val.is_object())
                createForwardShaders(val);
        }
        else
        {
            const std::string& shaderName = pathManager.next();
            createNewForwardShader(shaderName, val);
        }
    }


}
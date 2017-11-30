#ifndef JSON_SCENE_H_
#define JSON_SCENE_H_

#include "vulkan-core/scene_graph/scene.h"
#include "math/math_interface.h"
#include "utils/json.hpp"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    // Will be picked if a data-field is not present and not otherwise specified
    #define DEFAULT_COLOR               Color::WHITE
    #define DEFAULT_VEC3F               Vec3f(1,1,1)
    #define DEFAULT_VEC4F               Vec4f(1,1,1,1)
    #define DEFAULT_POINT3F             Point3f(0,0,0)
    #define JSON_SCENE_NO_IDENTIFIER    "NO_IDENTIFIER"

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Skybox;

    //---------------------------------------------------------------------------
    //  JSON scene class
    //---------------------------------------------------------------------------

    using JSON = nlohmann::json;

    class JSONScene : public Scene
    {
        JSON json;
        bool renderEnvironmentMaps = true;

    public:
        JSONScene(const JSON& jsonText) : Scene("JSONScene"), json(jsonText), onDeleteCallback(nullptr) {}
        JSONScene(const std::string& identifier, const JSON& jsonText)
            : Scene(identifier), json(jsonText), onDeleteCallback(nullptr) {}
        ~JSONScene() { if(onDeleteCallback) onDeleteCallback(this); }

        void init(RenderingEngine* renderer) override;
        void reload(RenderingEngine* renderer) override;
        void modify(const JSON& newJsonText);

        void onDelete(const std::function<void(JSONScene*)>& func) { onDeleteCallback = func; }

    private:
        void parseSettings(RenderingEngine* renderer, const JSON& json);
        void createCamera(RenderingEngine* renderer, const JSON& json);
        void createLights(const JSON& json);
        void createNewLight(const std::string& name, const JSON::value_type& props);
        void createObjects(const JSON& json);
        void createNewObject(const std::string& name, const JSON::value_type& props);
        void createMaterials(const JSON& json);
        void createNewMaterial(const std::string& name, const JSON::value_type& props);
        void createForwardShaderMaterial(const JSON& json, const std::string& matName);
        void addParamToForwardShaderMaterial(MaterialPtr material, const std::string& propName, const JSON::value_type& val);
        void createPBRMaterial(const JSON& json, const std::string& matName);
        void createModels(const JSON& json);
        void createSkybox(const JSON& json);
        void createForwardShaders(const JSON& json);
        void createNewForwardShader(const std::string& name, const std::string& path);
        void parsePostProcessing(RenderingEngine* renderer, const JSON& json);

        Transform   parseTransform(const JSON& json);
        Point3f     parsePoint3f(const JSON& json, const char* name, const Point3f& p = DEFAULT_POINT3F);
        Vec3f       parseVec3f(const JSON& json, const char* name, const Vec3f& v = DEFAULT_VEC3F);
        Vec3f       parseRawVec3f(const JSON& json);
        Vec4f       parseVec4f(const JSON& json, const char* name, const Vec4f& v = DEFAULT_VEC4F);
        Vec4f       parseRawVec4f(const JSON& json);
        Color       parseColor(const JSON& json, const Color& color = DEFAULT_COLOR);
        Color       parseRawColor(const JSON& col);
        TexturePtr  parseTexture(const JSON& json, const std::string& name);
        CubemapPtr  parseCubemap(const JSON& json, const std::string& name);
        TexturePtr  parseTexture(const std::string& virtualPath);
        CubemapPtr  parseCubemap(const std::string& virtualPath);
        std::string parseString(const JSON& json);

        template <typename T>
        T parsePrimitive(const JSON& json, const char* name, T default = 0);

        ShadowInfo* parseShadowInfo(const JSON& info);

        Skybox* skybox = nullptr;
        Camera* camera = nullptr;
        std::map<std::string, MeshPtr> meshes;
        std::map<std::string, ForwardShaderPtr> forwardShaders;
        std::map<std::string, MaterialPtr> materials;
        std::map<std::string, Renderable*> objects;
        std::map<std::string, Light*> lights;

        IrradianceMapPtr    irradianceMap;
        PremPtr             prem;

        std::function<void(JSONScene*)> onDeleteCallback;

        // <<<<<<<<<<<<<<<<<<<< RELOADING STUFF >>>>>>>>>>>>>>>>>>>>>>>>>
        enum class OP
        {
            UNKNOWN,
            REPLACE,
            REMOVE,
            ADD
        };
        OP stringToOP(const std::string& op);
        void executePatch(const JSON& patch);

        //---------------------------------------------------------------------------
        //  PathManager class
        //---------------------------------------------------------------------------

        class PathManager
        {
        public:
            PathManager(const std::string& paths) : m_nextIndex(0)
            {
                m_paths = splitString(paths, '/');
            }
            const std::string& next()
            {
                assert(m_nextIndex < m_paths.size());
                const std::string& nextPath = m_paths[m_nextIndex];
                m_nextIndex++;
                return nextPath;
            }
            void revertOne(){ assert(m_nextIndex != 0); m_nextIndex--; }
            bool hasNext() { return m_nextIndex != m_paths.size(); }
        private:
            std::vector<std::string> m_paths;
            int m_nextIndex;
        };

        void replaceValue(PathManager& pathManager, const JSON::value_type& val);
        void replaceValueInObject(PathManager& pathManager, const JSON::value_type& val);
        void replaceValueInCamera(PathManager& pathManager, const JSON::value_type& val);
        void replaceValueInSkybox(PathManager& pathManager, const JSON::value_type& val);
        void replaceValueInLight(PathManager& pathManager, const JSON::value_type& val);
        void replaceValueInModel(PathManager& pathManager, const JSON::value_type& val);
        void replaceValueInMaterial(PathManager& pathManager, const JSON::value_type& val);
        void replaceValueInShader(PathManager& pathManager, const JSON::value_type& val);
        void replaceTransformValue(Node* node, PathManager& pathManager, const JSON::value_type& val);
        void replaceValueInSettings(PathManager& pathManager, const JSON::value_type& val);

        void removeValue(PathManager& pathManager);
        void removeValueInObject(PathManager& pathManager);
        void removeValueInCamera(PathManager& pathManager);
        void removeValueInSkybox(PathManager& pathManager);
        void removeValueInLight(PathManager& pathManager);
        void removeValueInModel(PathManager& pathManager);
        void removeValueInMaterial(PathManager& pathManager);
        void removeValueInShader(PathManager& pathManager);

        void addValue(PathManager& pathManager, const JSON::value_type& val);
        void addValueInObject(PathManager& pathManager, const JSON::value_type& val);
        void addValueInCamera(PathManager& pathManager, const JSON::value_type& val);
        void addValueInSkybox(PathManager& pathManager, const JSON::value_type& val);
        void addValueInLight(PathManager& pathManager, const JSON::value_type& val);
        void addValueInModel(PathManager& pathManager, const JSON::value_type& val);
        void addValueInMaterial(PathManager& pathManager, const JSON::value_type& val);
        void addValueInShader(PathManager& pathManager, const JSON::value_type& val);
        void addTransformValue(Node* r, PathManager& pathManager, const JSON::value_type& val);

        void resetMaterialsOfObjects();
        void resetModelsOfObjects();
        void readdForwardShaderMaterialParams(MaterialPtr mat);

        MeshPtr getMesh(const std::string& name);
        MaterialPtr getMaterial(const std::string& name);
        TexturePtr getTexture(const std::string& virtualPath);

        friend class JSONPatchObjects;
        friend class JSONPatchConcreteObject;

        std::string getFullShaderName(const std::string& shaderName) { return getName() + "#" + shaderName; }
    };

    template <typename T>
    T JSONScene::parsePrimitive(const JSON& json, const char* name, T default)
    {
        return json.count(name) != 0 ? json[name] : static_cast<T>(default);
    }

}


#endif // !JSON_SCENE_H_

#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_

namespace Pyro
{

    template<class T1, class T2>
    class Resource;

    class Texture;
    class Cubemap;
    class Font;
    class IrradianceMap;
    class Prem;
    class ForwardShader;
    class Shader;
    class Mesh;
    class SubMesh;
    class Material;
    class PBRMaterial;
    class Scene;

    using TexturePtr        = Resource<Texture, Texture>;
    using CubemapPtr        = Resource<Cubemap, Texture>;
    using FontPtr           = Resource<Font, Texture>;
    using IrradianceMapPtr  = Resource<IrradianceMap, Texture>;
    using PremPtr           = Resource<Prem, Texture>;
    using ShaderPtr         = Resource<Shader, Shader>;
    using ForwardShaderPtr  = Resource<ForwardShader, Shader>;
    using MeshPtr           = Resource<Mesh, Mesh>;
    using MaterialPtr       = Resource<Material, Material>;
    using PBRMaterialPtr    = Resource<PBRMaterial, Material>;

}


#endif // !DATA_TYPES_H_


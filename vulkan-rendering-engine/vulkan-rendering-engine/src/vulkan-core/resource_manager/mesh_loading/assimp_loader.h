#ifndef ASSIMP_LOADER_H_
#define ASSIMP_LOADER_H_

#include <string>

//---------------------------------------------------------------------------
//  Forward Declarations
//---------------------------------------------------------------------------

struct aiScene;

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Mesh;

    //---------------------------------------------------------------------------
    //  AssimpLoader class
    //---------------------------------------------------------------------------

    class AssimpLoader
    {
    public:
        // Load a mesh from the given filePath
        // "preTransformVertices" is needed for Collada-Files
        static Mesh* loadMesh(const std::string& filePath, bool preTransformVertices);

    private:
        // Load all textures specified in the scene object and make materials from it
        static void loadMaterials(const std::string& filePath, Mesh* mesh, const aiScene* scene);
    };


}




#endif // !ASSIMP_LOADER_H_


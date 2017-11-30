#include "assimp_loader.h"

#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/data/material/texture/texture.h"
#include "vulkan-core/data/material/pbr_material.h"
#include "vulkan-core/data/mesh/mesh.h"
#include "file_system/vfs.h"

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace Pyro
{

    // Calculates the 3D-centroid for a given set of vertices
    Point3f compute3DCentroid(const std::vector<Vertex>& vertices)
    {
        Point3f sum;
        for(const auto& vertex : vertices)
            sum += vertex.position;
        return sum / static_cast<float>(vertices.size());
    }

    // Calculate the Dimension of a given set of vertices. If "computeCentroid" = false, centroid will be (0,0,0).
    // If "computeCentroid" = true all given vertices will be modified by it and the "centroid" itself becomes the
    // local position of this object then.
    Dimension calculateDimension(std::vector<Vertex>& vertices, bool computeCentroidAndOffsetVertices)
    {
        Dimension dimension;

        // Compute the 3D-Centroid
        if (computeCentroidAndOffsetVertices)
            dimension.localPosition = compute3DCentroid(vertices);

        for (auto& vertex : vertices)
        {
            // If we computed the centroid modify every vertex position by it
            if (computeCentroidAndOffsetVertices)
                vertex.position -= dimension.localPosition;

            // Calculate max + min dimension
            dimension.max = dimension.max.maxVec(vertex.position);
            dimension.min = dimension.min.minVec(vertex.position);

            // Calculate distance from vertex-point to center, avoid costly square-root
            float sqrtDistance = vertex.position.sqrtMagnitude();
            if (dimension.maxRadius < sqrtDistance)
                dimension.maxRadius = sqrtDistance;
        }
        // Now do the costly square-root to get the maximum distance from the farthest vertex
        dimension.maxRadius = sqrt(dimension.maxRadius);
        dimension.calculateSize();

        return dimension;
    }

    Mesh* AssimpLoader::loadMesh(const std::string& virtualPath, bool preTransformVertices)
    {
        std::string physicalPath = VFS::resolvePhysicalPath(virtualPath);

        int defaultFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace
                           | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_RemoveRedundantMaterials 
                           | aiProcess_GenUVCoords | aiProcess_FindInvalidData;
        if (preTransformVertices)
            defaultFlags |= aiProcess_PreTransformVertices;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(physicalPath.c_str(), defaultFlags);

        // If the import failed, report it
        if (!scene)
            Logger::Log(std::string("Assimp Error Message: ") + importer.GetErrorString(), LOGTYPE_ERROR);

        // Setup data structures
        Mesh* mesh = new Mesh(virtualPath);
        Dimension&              totalDimension  = mesh->dimension;
        std::vector<Vertex>&    vertices        = mesh->vertices;
        std::vector<uint32_t>&  indices         = mesh->indices;
        std::vector<SubMesh*>&  subMeshes       = mesh->subMeshes;

        // Create submeshes for each mesh in the aiScene
        aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
        for (unsigned int m = 0; m < scene->mNumMeshes; m++)
        {
            aiMesh* aMesh = scene->mMeshes[m];
            bool hasTextureCoords       = aMesh->HasTextureCoords(0);
            bool hasNormals             = aMesh->HasNormals();
            bool hasTangentsBitangents  = aMesh->HasTangentsAndBitangents();

            if(!hasTextureCoords)
                Logger::Log(TS(m) + "th Submesh of mesh '" + physicalPath + "' has no UV-Coordinates.\n"
                            "That means ASSIMP could not generate tangents / bitangents for it. Lighting wont work.", LOGTYPE_WARNING);
            if(!hasNormals)
                Logger::Log(TS(m) + "th Submesh of mesh '" + physicalPath + "' has no Normals", LOGTYPE_WARNING);

            SubMesh* newSubMesh = new SubMesh(mesh);
            newSubMesh->startVertIndex  = static_cast<uint32_t>(vertices.size());
            newSubMesh->startIndex      = static_cast<uint32_t>(indices.size());
            newSubMesh->materialIndex   = aMesh->mMaterialIndex;

            // Fill vertices
            std::vector<Vertex> subMeshVertices;
            for (unsigned int j = 0; j < aMesh->mNumVertices; j++)
            {
                aiVector3D* pPos        = &(aMesh->mVertices[j]);
                aiVector3D* pTexCoord   = hasTextureCoords ? &(aMesh->mTextureCoords[0][j]) : &Zero3D;
                aiVector3D* pNormal     = hasNormals ? &(aMesh->mNormals[j]) : &Zero3D;
                aiVector3D* pTangent    = hasTangentsBitangents ? &(aMesh->mTangents[j]) : &Zero3D;
                aiVector3D* pBiTangent  = hasTangentsBitangents ? &(aMesh->mBitangents[j]) : &Zero3D;

                // Create vertex
                Vertex vertex{
                    Vec3f(pPos->x, pPos->y, pPos->z),
                    Vec2f(pTexCoord->x, pTexCoord->y),
                    Vec3f(pNormal->x, pNormal->y, pNormal->z),
                    Vec3f(pTangent->x, pTangent->y, pTangent->z),
                    Vec3f(pBiTangent->x, pBiTangent->y, pBiTangent->z)
                };

                subMeshVertices.push_back(std::move(vertex));
            }

            // Fill Indices
            for (unsigned int k = 0; k < aMesh->mNumFaces; k++)
            {
                const aiFace& Face = aMesh->mFaces[k];
                if (Face.mNumIndices != 3)
                {
                    Logger::Log("Mesh '" + mesh->getFilePath() + "' has faces without 3 indices. Assimp could not triangulate the mesh. "
                                "The mesh might be incorrect.", LOGTYPE_WARNING, LOG_LEVEL_NOT_IMPORTANT);
                    continue;
                }

                indices.push_back(Face.mIndices[0]);
                indices.push_back(Face.mIndices[1]);
                indices.push_back(Face.mIndices[2]);
            }
            newSubMesh->numIndices = static_cast<uint32_t>(indices.size()) - newSubMesh->startIndex;

            // Calculate the SubMesh dimension and add it to the total dimension of the "whole" mesh.
            // Skip calculating the centroid for the first Mesh.
            bool computeCentroidAndOffsetVertices = m > 0 ? true : false;
            newSubMesh->dimension = calculateDimension(subMeshVertices, computeCentroidAndOffsetVertices);
            totalDimension.unionDimensions(newSubMesh->dimension);

            // Finally save the subMesh and add the SubMesh-Vertices to the "whole" mesh
            subMeshes.push_back(std::move(newSubMesh));
            vertices.insert(vertices.end(), subMeshVertices.begin(), subMeshVertices.end());
        }

        // Load materials from the scene
        if (scene->HasMaterials())
            loadMaterials(physicalPath, mesh, scene);

        mesh->uploadDataToGPU();

        return mesh;
    }

    // Check if the given material is the default one or a real material
    // The only way to do this in Assimp currently is to check the name
    bool isDefaultMaterial(const aiMaterial* material)
    {
        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        return std::string(name.C_Str()) == AI_DEFAULT_MATERIAL_NAME;
    }

    // Tries to load a texture from the given material.
    // Return nullptr if texture does not exist.
    TexturePtr loadTexture(const aiMaterial* material, aiTextureType textureType, const std::string& filePath, bool logMissingTextureWarning)
    {
        aiString texturePath;
        if (material->GetTextureCount(textureType) > 0 && material->GetTexture(textureType, 0, &texturePath) == AI_SUCCESS)
        {
            const std::string fullTexturePath = FileSystem::getDirectoryPath(filePath) + texturePath.C_Str();
            if (FileSystem::fileExists(fullTexturePath))
                return TEXTURE(fullTexturePath);
            else if(logMissingTextureWarning)
                Logger::Log("Could not find texture '" + fullTexturePath + "'", LOGTYPE_WARNING);
        }
        // Texture type does not exist in material so just return nullptr
        return nullptr;
    }

    // Load all textures specified in the scene object and make materials from it
    void AssimpLoader::loadMaterials(const std::string& filePath, Mesh* mesh, const aiScene* scene)
    {
        std::vector<TexturePtr>& textures           = mesh->textures;
        std::map<uint32_t, MaterialPtr>& materials  = mesh->materials;

        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            const aiMaterial* material = scene->mMaterials[i];

            // Because Mesh-Files without a material file still have one material, i have to check it manually
            // This can cause problems if the first material is named "DefaultMaterial"
            if (scene->mNumMaterials == 1 && isDefaultMaterial(material))
                continue;

            PBRMaterialPtr newMaterial = PBRMATERIAL({ nullptr });

            // Diffuse-Texture
            auto diffuseMap = loadTexture(material, aiTextureType_DIFFUSE, filePath, true);
            bool hasDiffuseMap = diffuseMap != nullptr;
            if (hasDiffuseMap)
            {
                newMaterial->setTexture(SHADER_DIFFUSE_MAP_NAME, diffuseMap);
                textures.push_back(diffuseMap);
            } else { // Diffuse-Texture is not even present in the material-class
                std::string missingTextureMessage = "There is no diffuse texture specified for material #" + TS(i) +
                                                    " for file " + filePath;
                Logger::Log(missingTextureMessage, LOGTYPE_WARNING);
            }

            if (hasDiffuseMap)
            {
                // Normal-Map
                auto normalMap = loadTexture(material, aiTextureType_NORMALS, filePath, false);
                if (normalMap != nullptr)
                {
                    newMaterial->setMatNormalMap(normalMap);
                    textures.push_back(normalMap);
                }

                // AO-Map
                auto aoMap = loadTexture(material, aiTextureType_AMBIENT, filePath, false);
                if (aoMap != nullptr)
                {
                    newMaterial->setMatAOMap(aoMap);
                    textures.push_back(aoMap);
                }

                // Metalness (Specular)-Map 
                auto metallicMap = loadTexture(material, aiTextureType_SPECULAR, filePath, false);
                if (metallicMap != nullptr)
                {
                    newMaterial->setMatMetallicMap(metallicMap);
                    textures.push_back(metallicMap);
                }

                // Roughness-Map
                auto roughnessMap = loadTexture(material, aiTextureType_SHININESS, filePath, false);
                if (roughnessMap != nullptr)
                {
                    newMaterial->setMatRoughnessMap(roughnessMap);
                    textures.push_back(roughnessMap);
                }

                // Displacement-Map (aiTextureType_DISPLACEMENT or aiTextureType_HEIGHT)
                auto displacementMap = loadTexture(material, aiTextureType_DISPLACEMENT, filePath, false);
                if (!displacementMap.isValid()) 
                    displacementMap = loadTexture(material, aiTextureType_HEIGHT, filePath, false);
                if (displacementMap.isValid())
                {
                    newMaterial->setMatDisplacementMap(displacementMap);
                    textures.push_back(displacementMap);
                }

                //int normal = material->GetTextureCount(aiTextureType_NORMALS);
                //int specular = material->GetTextureCount(aiTextureType_SPECULAR);
                //int disp = material->GetTextureCount(aiTextureType_DISPLACEMENT);
                //int height = material->GetTextureCount(aiTextureType_HEIGHT);
                //int ambent = material->GetTextureCount(aiTextureType_AMBIENT);
                //int emmissive = material->GetTextureCount(aiTextureType_EMISSIVE);
                //int shininess = material->GetTextureCount(aiTextureType_SHININESS);

                // Set Material-Properties
                for (unsigned int j = 0; j < material->mNumProperties; j++)
                {
                    aiMaterialProperty* prop = material->mProperties[j];
                    // Add parameters to material
                }
            }
            materials[i] = newMaterial;
        }
    }


}
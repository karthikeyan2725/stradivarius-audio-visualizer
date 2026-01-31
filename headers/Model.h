#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

#include <Mesh.h>

class Model{
    std::string directory;
    std::vector<Mesh> meshes;
    const aiTextureType types[28] = {
            aiTextureType_NONE,
            aiTextureType_DIFFUSE,
            aiTextureType_SPECULAR,
            aiTextureType_AMBIENT,
            aiTextureType_EMISSIVE,
            aiTextureType_HEIGHT,
            aiTextureType_NORMALS,
            aiTextureType_SHININESS,
            aiTextureType_OPACITY,
            aiTextureType_DISPLACEMENT,
            aiTextureType_LIGHTMAP,
            aiTextureType_REFLECTION,
            aiTextureType_BASE_COLOR,
            aiTextureType_NORMAL_CAMERA,
            aiTextureType_EMISSION_COLOR,
            aiTextureType_METALNESS,
            aiTextureType_DIFFUSE_ROUGHNESS,
            aiTextureType_AMBIENT_OCCLUSION,
            aiTextureType_UNKNOWN,
            aiTextureType_SHEEN,
            aiTextureType_CLEARCOAT,
            aiTextureType_TRANSMISSION,
            aiTextureType_MAYA_BASE,
            aiTextureType_MAYA_SPECULAR,
            aiTextureType_MAYA_SPECULAR_COLOR,
            aiTextureType_MAYA_SPECULAR_ROUGHNESS,
            aiTextureType_ANISOTROPY,
            aiTextureType_GLTF_METALLIC_ROUGHNESS
        };
public:
    Model(const char* filePath){
        load(filePath);
    }

    ~Model(){

    }

    void load(std::string filePath){
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(filePath, aiProcess_Triangulate  );
        
        std::cout << "Loading Model: " << filePath << std::endl;

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            std::cout << "Failed to load model: " << filePath << std::endl;
            std::cout << importer.GetErrorString() << std::endl;
            return;
        } 
        
        directory = filePath.substr(0, filePath.find_last_of('/'));
        
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode *node, const aiScene *scene){
        for(unsigned int i = 0; i < node->mNumMeshes; i++){
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        
        for(unsigned int i = 0; i < node->mNumChildren; i++){
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene){
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<std::shared_ptr<Texture>> textures;

        for(unsigned int i = 0; i < mesh->mNumVertices; i++){
            Vertex vertex;
            glm::vec3 position;
            position.x = mesh->mVertices[i].x;
            position.y = mesh->mVertices[i].y;
            position.z = mesh->mVertices[i].z;
            vertex.position = position;

            if(mesh->mTextureCoords[0]){
                glm::vec2 texCoord;
                texCoord.x = mesh->mTextureCoords[0][i].x;
                texCoord.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoord = texCoord;
            } else {
                vertex.texCoord = glm::vec2(0.0f);
            }   

            glm::vec3 normal;
            normal.x = mesh->mNormals[i].x;
            normal.y = mesh->mNormals[i].y;
            normal.z = mesh->mNormals[i].z;
            vertex.normal = normal;

            vertices.push_back(vertex);
        }

        for(unsigned int j = 0; j < mesh->mNumFaces; j++){
            aiFace face = mesh->mFaces[j];
            for(unsigned int k = 0; k < face.mNumIndices; k++){
                indices.push_back(face.mIndices[k]);
            }
        }
        
        std::cout << "Mesh Name: " << mesh->mName.C_Str() << std::endl;
        if(mesh->mMaterialIndex >= 0){
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            std::cout << "Material Name : " << material->GetName().C_Str() << std::endl;
            
            aiTextureType loadType;
            std::string typeString;
            if(material->GetTextureCount(aiTextureType_BASE_COLOR) > 0){
                loadType = aiTextureType_BASE_COLOR;
                typeString = "base_color";
            } else if(material->GetTextureCount(aiTextureType_DIFFUSE) > 0){
                loadType = aiTextureType_DIFFUSE;
                typeString = "diffuse_color";
            }else {
                loadType = aiTextureType_EMISSIVE;
                typeString = "emissive_color";
            }

            std::vector<std::shared_ptr<Texture>> diffuseTexture = loadMaterialTextures(material, loadType, typeString);
            textures.insert(textures.end(), diffuseTexture.begin(), diffuseTexture.end());
        }
    
        Mesh meshResult(vertices, indices, textures);
        return meshResult;
    }

    std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial *material, aiTextureType type, std::string typeName){
        std::vector<std::shared_ptr<Texture>> textures;
        std::cout << typeName << " count: " << material->GetTextureCount(type) << std::endl;
        for(unsigned int i = 0; i < material->GetTextureCount(type); i++){
            aiString name;
            material->GetTexture(type, i, &name);
            std::string fullPath = directory + "/" +  std::string(name.C_Str());
            std::shared_ptr<Texture> texture = Texture::load(fullPath.c_str(), GL_LINEAR, GL_CLAMP_TO_EDGE);
            textures.push_back(texture);
            std::cout << "TEXTURE: " << fullPath << std::endl;
        }

        return textures;
    }

    void draw(Shader *shader){
        // for(int i = meshes.size()-1; i >=0; i--) meshes[i].draw(shader);
        for(Mesh mesh: meshes) mesh.draw(shader);
    }

    void draw(Shader *shader, int instanceCount){
        // for(int i = meshes.size()-1; i >=0; i--) meshes[i].draw(shader);
        for(Mesh mesh: meshes) mesh.draw(shader, instanceCount);
    }
};
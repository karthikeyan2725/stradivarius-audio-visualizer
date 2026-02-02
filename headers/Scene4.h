#pragma once
#include <Model.h>
#include <memory>

class Scene4{
    std::shared_ptr<Model> tunnelModel;
    std::shared_ptr<Shader> tunnelShader;
    std::shared_ptr<Mesh> boxMesh;
    std::shared_ptr<Shader> boxShader;
public:
    Scene4(){
        tunnelModel = std::make_shared<Model>("./models/forest_well/scene.gltf");
        tunnelShader = std::make_shared<Shader>("src/shaders/tunnel.vs", "src/shaders/tunnel.fs");
        boxMesh = getBoxMesh();
        boxShader = std::make_shared<Shader>("src/shaders/box.vs", "src/shaders/box.fs");
    }

    void draw(float boxZ, float tunnelZ, float aspect, float boxRotation, float tunnelRotationZ){
        float fov = 90.0f;
        glm::mat4 projMatrix = glm::perspective(glm::radians(fov), aspect, 0.1f, 10000.0f);
        
        // Tunnel
        int numTunnel = 10;
        float tunnelScale = 0.003f;
        float rotateX = 90.0f;
        float offsetAmount = 0.3f;
        glm::vec3 boxCenter = glm::vec3(0.0, 0.0, -(boxZ+0.5f));
        glm::vec3 boxColor = glm::vec3(0.5, 0.0, 0.0);
        tunnelShader->use();
        for(int i = 0; i < numTunnel; i++){
            float currentOffset = offsetAmount * i;
            
            glm::mat4 tunnelModelMat = glm::mat4(1.0f); 
            tunnelModelMat = glm::translate(tunnelModelMat, glm::vec3(0, 0, -(currentOffset + tunnelZ)));
            tunnelModelMat = glm::rotate(tunnelModelMat, glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
            tunnelModelMat = glm::rotate(tunnelModelMat, glm::radians(tunnelRotationZ), glm::vec3(0.0f, 1.0f, 0.0f));
            tunnelModelMat = glm::scale(tunnelModelMat, glm::vec3(tunnelScale));
            
            std::string uniformString = "modelInstanced[" + std::to_string(i) + "]";
            tunnelShader->setUniformMatrix4fv(uniformString.c_str(), tunnelModelMat);
        }
        tunnelShader->setUniformMatrix4fv("projection", projMatrix);
        tunnelShader->setUniform3fv("lightColor", boxColor);
        tunnelShader->setUniform3fv("lightPos", boxCenter);
        tunnelModel->draw(tunnelShader, numTunnel);

        // Light Box
        float boxScale = 0.5f;
        glm::mat4 boxModelMat = glm::mat4(1.0f);
        boxModelMat = glm::translate(boxModelMat, glm::vec3(0.0f, 0.0f, -boxZ));
        boxModelMat = glm::rotate(boxModelMat, glm::radians(boxRotation), glm::vec3(1.0f, 0.5f, 1.0f));
        boxModelMat = glm::scale(boxModelMat, glm::vec3(boxScale)); 
        boxShader->use();
        boxShader->setUniformMatrix4fv("projection", projMatrix);
        boxShader->setUniformMatrix4fv("model", boxModelMat);
        boxMesh->draw(boxShader);
    }

    std::shared_ptr<Mesh> getBoxMesh(){
        std::vector<Vertex> boxVertices;
        std::vector<unsigned int> boxIndices;
        std::vector<std::shared_ptr<Texture>> boxTextures;

        // Box Vertices
        // Bottom
        boxVertices.push_back({glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // A 0
        boxVertices.push_back({glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // B 1
        boxVertices.push_back({glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // C 2
        boxVertices.push_back({glm::vec3(-0.5, -0.5f, 0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // D 3
        // Top
        boxVertices.push_back({glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // G 4
        boxVertices.push_back({glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // F 5
        boxVertices.push_back({glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // E 6
        boxVertices.push_back({glm::vec3(-0.5, 0.5f, 0.5f), glm::vec3(0.0f), glm::vec3(0.0f)}); // H 7
        
        // Box Indices
        // TOP
        // ABC
        boxIndices.push_back(0);
        boxIndices.push_back(1);
        boxIndices.push_back(2);
        // CDA
        boxIndices.push_back(2);
        boxIndices.push_back(3);
        boxIndices.push_back(0);

        // BOTTOM
        // EFG
        boxIndices.push_back(6);
        boxIndices.push_back(5);
        boxIndices.push_back(4);
        // GHE
        boxIndices.push_back(4);
        boxIndices.push_back(7);
        boxIndices.push_back(6);

        // LEFT
        // GFA
        boxIndices.push_back(4);
        boxIndices.push_back(5);
        boxIndices.push_back(0);
        // FBA
        boxIndices.push_back(5);
        boxIndices.push_back(1);
        boxIndices.push_back(0);

        // RIGHT
        // EHD
        boxIndices.push_back(6);
        boxIndices.push_back(7);
        boxIndices.push_back(3);
        // DCE
        boxIndices.push_back(3);
        boxIndices.push_back(2);
        boxIndices.push_back(6);

        // FRONT
        // ADH
        boxIndices.push_back(0);
        boxIndices.push_back(3);
        boxIndices.push_back(7);
        // HGA
        boxIndices.push_back(7);
        boxIndices.push_back(4);
        boxIndices.push_back(0);

        // BACK
        // FEB
        boxIndices.push_back(5);
        boxIndices.push_back(6);
        boxIndices.push_back(1);
        // BEC
        boxIndices.push_back(1);
        boxIndices.push_back(6);
        boxIndices.push_back(2);

        return std::make_shared<Mesh>(boxVertices, boxIndices, boxTextures);
    }
};

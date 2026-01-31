#pragma once
#include <glad/glad.h>
#include <Texture.h>
#include <glm/glm.hpp>
#include <Shader.h>
#include <memory>

struct Vertex{
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
};

class Mesh{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture>> textures;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures){
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    ~Mesh(){
        // glDeleteBuffers(1, &vbo);
        // glDeleteBuffers(1, &ebo);
        // glDeleteVertexArrays(1, &vao);
    }

    void draw(Shader *shader){
        shader->use();
        int number = 0;
        for(std::shared_ptr<Texture> texture: textures){
            std::string texName = "tex" + std::to_string(number); 
            shader->setUniform1i(texName.c_str(), number);
            texture->bind(number);
            number++;
        }
        
        if(textures.size() == 0) shader->setUniform1i("hasTexture", 0);
        else shader->setUniform1i("hasTexture", 1);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    } 

    void draw(Shader *shader, int instanceCount){
        shader->use();
        int number = 0;
        for(std::shared_ptr<Texture> texture: textures){
            std::string texName = "tex" + std::to_string(number); 
            shader->setUniform1i(texName.c_str(), number);
            texture->bind(number);
            number++;
        }
        
        if(textures.size() == 0) shader->setUniform1i("hasTexture", 0);
        else shader->setUniform1i("hasTexture", 1);

        glBindVertexArray(vao);
        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr, instanceCount);
        glBindVertexArray(0);
    }
};
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader{
    unsigned int shaderProgram;

    public:
        Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
        void use();
        ~Shader();
        void setUniformMatrix4fv(const char* uniformName, glm::mat4 value);
        void setUniform1i(const char* uniformName, int value);
        void setUniform1f(const char* uniformName, float value);
        void setUniform3fv(const char* uniformName, const glm::vec3 &vec);
};
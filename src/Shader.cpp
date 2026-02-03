#include <glad/glad.h>
#include <Shader.h>
#include <iostream>
#include <fstream>

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath){
    std::ifstream vsFile(vertexShaderPath), fsFile(fragmentShaderPath);
    std::string vsSrcStr, fsSrcStr;
    std::getline(vsFile, vsSrcStr, '\0');
    std::getline(fsFile, fsSrcStr, '\0');
    const char *vsShaderSrc = vsSrcStr.c_str();
    const char *fsShaderSrc = fsSrcStr.c_str();

    int success;
    char message[1000]; // TODO: Message size Macro?

    unsigned int vsShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsShader, 1, &vsShaderSrc, NULL);
    glCompileShader(vsShader);
    glGetShaderiv(vsShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vsShader, 1000, NULL, message);
        std::cout << "Failed to compile Vertex Shader " << vertexShaderPath << ": " << message << std::endl;
    }

    unsigned int fsShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsShader, 1, &fsShaderSrc, NULL);
    glCompileShader(fsShader);
    glGetShaderiv(fsShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fsShader, 1000, NULL, message);
        std::cout << "Failed to compile Fragment Shader " << fragmentShaderPath << ": " << message << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vsShader);
    glAttachShader(shaderProgram, fsShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shaderProgram, 1000, NULL, message);
        std::cout << "Failed to link shaders " << vertexShaderPath << ", " << fragmentShaderPath << ": " << message << std::endl;
    }

    glDeleteShader(vsShader);
    glDeleteShader(fsShader);
    vsFile.close();
    fsFile.close();
};

void Shader::use(){
    glUseProgram(shaderProgram);
}

Shader::~Shader(){
    glDeleteProgram(shaderProgram);
}

void Shader::setUniformMatrix4fv(const char* uniformName, const glm::mat4 value){
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(value)); 
}

void Shader::setUniform1i(const char* uniformName, const int value){
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform1i(uniformLoc, value);
}

void Shader::setUniform1f(const char* uniformName, const float value){
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform1f(uniformLoc, value);
}

void Shader::setUniform3fv(const char* uniformName, const glm::vec3 &vec){
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform3fv(uniformLoc, 1, &vec[0]);
}

void Shader::setUniform2fv(const char* uniformName, const glm::vec2 &vec){
    int uniformLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform2fv(uniformLoc, 1, &vec[0]);
}
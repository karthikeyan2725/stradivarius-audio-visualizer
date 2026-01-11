#pragma once
#include <Shader.h>
#include <glad/glad.h>

class Waveform{ // TODO: This contains both mesh, and material data. Split mesh to seperate class afterwards
protected:
    unsigned int numVertices;
    unsigned int vbo;
    Shader *shader;
public:
    void draw(float *vertexData); // TODO: Make override of this necessary
};

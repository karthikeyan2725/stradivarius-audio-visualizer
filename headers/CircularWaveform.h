#include <Waveform.h>

class CircularWaveform : public Waveform{ // TODO : move definition and implementation to seperate files, handle override
public:
    CircularWaveform(int numVertices, const char* vsPath, const char* fsPath){
        this->numVertices = numVertices;
        shader = new Shader(vsPath, fsPath);

        shader->use();
        shader->setUniform1f("min_width", 0.5f);
        shader->setUniform1i("total_points", numVertices);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numVertices, NULL, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    ~CircularWaveform(){
        delete shader;
        glDeleteBuffers(1, &vbo);
    }

    void draw(float *vertexData){
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * numVertices, vertexData);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        shader->use();
        glPointSize(3.0f); // TODO: Add to constructor
        glDrawArrays(GL_POINTS, 0, numVertices);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};
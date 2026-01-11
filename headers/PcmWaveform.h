#include <Waveform.h>

class PcmWaveform : public Waveform{ // TODO : move definition and implementation to seperate files, handle override
public:
    PcmWaveform(int numVertices, const char* vsPath, const char* fsPath){
        this->numVertices = numVertices;
        shader = new Shader(vsPath, fsPath);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices, NULL, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    ~PcmWaveform(){
        delete shader;
        glDeleteBuffers(1, &vbo);
    }

    void draw(float *vertexData){
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numVertices, vertexData);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        shader->use();
        glDrawArrays(GL_LINE_STRIP, 0, numVertices);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};
#include <Waveform.h>
#include <glm/gtc/matrix_transform.hpp>

class TerrainWaveform { // TODO : move definition and implementation to seperate files, handle override
unsigned int numCols;
unsigned int numRows;
unsigned int *vbo;
Shader *shader;
public:
    TerrainWaveform(int numCols, int numRows, const char* vsPath, const char* fsPath){
        this->numCols = numCols;
        this->numRows = numRows;
        shader = new Shader(vsPath, fsPath);
        
        vbo = new unsigned int[numRows];
        glGenBuffers(numRows, vbo);
        for(int row = 0; row < numRows; row++){
            glBindBuffer(GL_ARRAY_BUFFER, vbo[row]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numCols, NULL, GL_DYNAMIC_DRAW);
        }

        // TODO : Move to transform code
        float scaleBy = 2.0f;
        glm::mat4 view = glm::mat4(1.0f); 
        view = glm::scale(view, glm::vec3(scaleBy));
        view = glm::rotate(view, 30.0f * (3.14f / 180), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, -140.0f * (3.14f / 180), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(-0.0f, 0.0f, 0.0f));
        shader->use();
        shader->setUniformMatrix4fv("view", view);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    ~TerrainWaveform(){
        glDeleteBuffers(numRows, vbo);
        delete shader;
        delete[] vbo;
    }

    void draw(float **vertexData){
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
        for(int row = 0; row < numRows; row++){
            glBindBuffer(GL_ARRAY_BUFFER, vbo[row]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * numCols, vertexData[row]);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            shader->use();
            shader->setUniform1i("offset", row);
            glPointSize(1.0f);
            glDrawArrays(GL_POINTS, 0, numCols);
        }

        glDisable(GL_BLEND);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};
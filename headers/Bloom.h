#pragma once
#include <glad/glad.h>
#include <Shader.h>
#include <iostream>
#include <FrameBuffer.h>

class Bloom{
    FrameBuffer *inputFrameBuffer, **pingPongFrameBuffers; 
    Shader *brightShader;
    Shader *copyShader;
    Shader *blurShader;
    Shader *blendShader;
    float quadVertex[24] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    unsigned int quadVBO;
public:
    Bloom(int width, int height){

        inputFrameBuffer = new FrameBuffer(width, height, GL_LINEAR, GL_CLAMP_TO_EDGE);
        pingPongFrameBuffers = new FrameBuffer*[2];
        for(int i = 0; i < 2; i++){
            pingPongFrameBuffers[i] = new FrameBuffer(width, height, GL_LINEAR, GL_CLAMP_TO_EDGE);
        }

        glGenBuffers(1, &quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertex), quadVertex, GL_STATIC_DRAW);

        brightShader = new Shader("src/shaders/extract_bright.vs", "src/shaders/extract_bright.fs");
        copyShader = new Shader("src/shaders/copy.vs", "src/shaders/copy.fs");
        blurShader = new Shader("src/shaders/blur.vs", "src/shaders/blur.fs");
        blendShader = new Shader("src/shaders/blend.vs", "src/shaders/blend.fs");
    }

    void start(){
        inputFrameBuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void end(FrameBuffer *targetFrameBuffer){
        pingPongFrameBuffers[0]->bind();
        glClear(GL_COLOR_BUFFER_BIT);
        brightShader->use();
        glBindTexture(GL_TEXTURE_2D, inputFrameBuffer->getColorAttachment());
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        blurShader->use();
        for(int i = 0; i < 2; i++){
            // horizontal blur
            pingPongFrameBuffers[1]->bind();
            glClear(GL_COLOR_BUFFER_BIT);
            blurShader->setUniform1i("horizontal", 1);
            glBindTexture(GL_TEXTURE_2D, pingPongFrameBuffers[0]->getColorAttachment());
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // vertical blur
            pingPongFrameBuffers[0]->bind();
            glClear(GL_COLOR_BUFFER_BIT);
            blurShader->setUniform1i("horizontal", 0);
            glBindTexture(GL_TEXTURE_2D, pingPongFrameBuffers[1]->getColorAttachment());
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        targetFrameBuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT);
        blendShader->use();
        blendShader->setUniform1i("sTexture1", 0);
        blendShader->setUniform1i("sTexture2", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputFrameBuffer->getColorAttachment());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingPongFrameBuffers[0]->getColorAttachment());
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glActiveTexture(GL_TEXTURE0);
    }

    void updateSize(int width, int height){
        inputFrameBuffer->updateSize(width, height);
        for(int i = 0; i < 2; i++) pingPongFrameBuffers[i]->updateSize(width, height);
    }
};
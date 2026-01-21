#pragma once
#include <glad/glad.h>
#include <Shader.h>
#include <iostream>

class Bloom{
    unsigned int *fbo;
    unsigned int *texture;
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

        fbo = new unsigned int[3];
        glGenFramebuffers(3, fbo);
        texture = new unsigned int[3];
        glGenTextures(3, texture);

        for(int i = 0; i < 3; i++){
            glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
            glBindTexture(GL_TEXTURE_2D, texture[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[i], 0);
    
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                std::cout << "Failed to setup framebuffer " << i << " " << std::endl;
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenBuffers(1, &quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertex), quadVertex, GL_STATIC_DRAW);

        brightShader = new Shader("src/shaders/extract_bright.vs", "src/shaders/extract_bright.fs");
        copyShader = new Shader("src/shaders/copy.vs", "src/shaders/copy.fs");
        blurShader = new Shader("src/shaders/blur.vs", "src/shaders/blur.fs");
        blendShader = new Shader("src/shaders/blend.vs", "src/shaders/blend.fs");
    }

    void start(){
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void end(int targetFBO){
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
        glClear(GL_COLOR_BUFFER_BIT);
        brightShader->use();
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        blurShader->use();
        for(int i = 0; i < 2; i++){
            // horizontal blur
            glBindFramebuffer(GL_FRAMEBUFFER, fbo[2]);
            glClear(GL_COLOR_BUFFER_BIT);
            blurShader->setUniform1i("horizontal", 1);
            glBindTexture(GL_TEXTURE_2D, texture[1]);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // vertical blur
            glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
            glClear(GL_COLOR_BUFFER_BIT);
            blurShader->setUniform1i("horizontal", 0);
            glBindTexture(GL_TEXTURE_2D, texture[2]);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // horizontal blur
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[2]);
        glClear(GL_COLOR_BUFFER_BIT);
        blurShader->setUniform1i("horizontal", 1);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
        glClear(GL_COLOR_BUFFER_BIT);
        copyShader->use();
        glBindTexture(GL_TEXTURE_2D, texture[2]);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        blendShader->use();
        blendShader->setUniform1i("sTexture1", 0);
        blendShader->setUniform1i("sTexture2", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
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
        for(int i = 0; i < 3; i++){
            glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
            glBindTexture(GL_TEXTURE_2D, texture[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};
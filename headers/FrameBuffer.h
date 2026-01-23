#pragma once
#include <glad/glad.h>
#include <Texture.h>

class FrameBuffer{
    unsigned int id;
    Texture *colorAttachment;
public:
    FrameBuffer(int width, int height, int filterMode, int wrapMode){
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        colorAttachment = new Texture(width, height, GL_RGB, GL_RGB, filterMode, wrapMode, NULL);
        colorAttachment->bind(0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment->getId(), 0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            std::cout << "Failed to setup A framebuffer" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~FrameBuffer(){
        delete colorAttachment;
        glDeleteFramebuffers(1, &id);
    }

    void bind(){
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    unsigned int getColorAttachment(){
        return colorAttachment->getId();
    }

    unsigned int getId(){
        return id;
    }

    void updateSize(int width, int height){
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        colorAttachment->updateSize(width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};
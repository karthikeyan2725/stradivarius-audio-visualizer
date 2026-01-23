#pragma once
#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>

class Texture{
    unsigned int id;
    GLint internalFormat;
    GLenum format;
    int filterMode;
    int wrapMode;
public:
    Texture(int width, int height, GLint internalFormat, GLenum format, int filterMode, int wrapMode, unsigned char* data){
        this->internalFormat = internalFormat;
        this->format = format;
        this->filterMode = filterMode;
        this->wrapMode = wrapMode;

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    }

    ~Texture(){
        glDeleteTextures(1, &id);
    }

    static Texture load(const char* filePath, int filterMode, int wrapMode){
        stbi_set_flip_vertically_on_load(true);  
        int width, height, nChannels;
        unsigned char* image_data = stbi_load(filePath, &width, &height, &nChannels, 0);
        
        if(!image_data){
            std::cout << "Failed to load texture (" << filePath << ")" << std::endl;
        }

        Texture generatedTexture(width, height, GL_RGB, GL_RGB, filterMode, wrapMode, image_data);
        return generatedTexture;
    }

    void bind(int slot){
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
        glActiveTexture(GL_TEXTURE0);
    }

    unsigned int getId() {
        return id;
    }

    void updateSize(int width, int height){
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <audiofile/AudioFile.h>
#include <miniaudio.h>
#include <stb_image.h>

#include <Scene1.h>
#include <Scene2.h>
#include <Scene3.h>

#define APPNAME "Stradivarius"

Bloom *b = nullptr;
void resize_frame_buffer_on_window(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
    if(b != nullptr) b->updateSize(width, height);
}

int main(int argc, char** argv){
    const char* songPath;
    if(argc < 2) {
        std::cout << "Please specify song path" << std::endl;
        return -1;
    }

    songPath = argv[1];
    std::cout << "Playing : " << songPath << std::endl;

    int glfwInitialized = glfwInit();
    if(glfwInitialized == GLFW_FALSE){
        std::cout << "Failed to Initalize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(500, 500, APPNAME, NULL, NULL);
    if(window == NULL){
        std::cout << "Failed to create a GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initalize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    int frameBufferWidth, frameBufferHeight;
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
    glViewport(0, 0, frameBufferWidth, frameBufferHeight);

    glfwSetFramebufferSizeCallback(window, resize_frame_buffer_on_window);

    ma_result result;
    ma_engine engine;
    ma_sound sound;
    result = ma_engine_init(NULL, &engine);
    if(result != MA_SUCCESS){
        std::cout << "Failed to initialize Miniaudio Engine" << std::endl;
        glfwTerminate();
        return -1;
    } 
    result = ma_sound_init_from_file(&engine, songPath, MA_SOUND_FLAG_ASYNC, NULL, NULL, &sound);
    if(result != MA_SUCCESS){
        std::cout << "Failed to Load Audio" << std::endl;
        glfwTerminate();
        return -1;
    } 
    ma_sound_start(&sound);
    
    AudioFile<double> audioFile;
    audioFile.load(songPath);
    audioFile.printSummary();
    int sampleRate = audioFile.getSampleRate();

    Scene1 scene1(audioFile.samples, sampleRate);
    Scene2 scene2(audioFile.samples, sampleRate);
    Scene3 scene3(audioFile.samples, sampleRate);
    
    int choice = 1;
    bool isBloom = false;
    unsigned int vao;
    glGenVertexArrays(1, &vao); // TODO: Move to each waveforms
    glBindVertexArray(vao);
    
    stbi_set_flip_vertically_on_load(true);  
    int w, h, nC;
    unsigned char *bg_image_data = stbi_load("images/night.jpg", &w, &h, &nC, 0);
    unsigned int bgTexture;
    if(bg_image_data){
        glGenTextures(1, &bgTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, bgTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, bg_image_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to Create Background texture" << std::endl;
    }
    glActiveTexture(GL_TEXTURE0);
    
    stbi_set_flip_vertically_on_load(true);  
    unsigned char *du_dv_image = stbi_load("images/dudv_map.png", &w, &h, &nC, 0);
    unsigned int du_dv_texture;
    if(du_dv_image){
        glGenTextures(1, &du_dv_texture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, du_dv_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, du_dv_image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to Create DU DV texture" << std::endl;
    }
    glActiveTexture(GL_TEXTURE0);

    float quadVertex[24] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertex), quadVertex, GL_STATIC_DRAW);

    Shader quadShader("src/shaders/bg.vs", "src/shaders/bg.fs");

    // Water FBO
    unsigned int waterFBO;
    glGenFramebuffers(1, &waterFBO);
    unsigned int waterTextureAttachment;
    glGenTextures(1, &waterTextureAttachment);
    glBindFramebuffer(GL_FRAMEBUFFER, waterFBO);
    glBindTexture(GL_TEXTURE_2D, waterTextureAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameBufferWidth, frameBufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, waterTextureAttachment, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Failed to setup framebuffer (Water) " << std::endl;
    }

    Shader waterShader("src/shaders/water.vs", "src/shaders/water.fs");
    glm::mat4 view = glm::mat4(1.0f); 
    view = glm::scale(view, glm::vec3(1.0f));
    view = glm::translate(view, glm::vec3(0.0f, -1.0f, 0.0f));
    view = glm::rotate(view, 180.0f * (3.14f / 180), glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::rotate(view, 180.0f * (3.14f / 180), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, 50.0f * (3.14f / 180), glm::vec3(1.0f, 0.0f, 0.0f));
    waterShader.use();
    waterShader.setUniformMatrix4fv("view", view);
    
    Bloom bloom(frameBufferWidth, frameBufferHeight);
    b = &bloom;
    float rotateX = 20.0f;
    float rotateY = 0;
    float lastSecond = 0;
    float waterOffset = 0.0f;
    glEnable( GL_DEPTH_CLAMP ) ; // No clipping of objects
    glEnable(GL_DEPTH_BUFFER_BIT);
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        if(isBloom) bloom.start();
        else glBindFramebuffer(GL_FRAMEBUFFER, waterFBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        quadShader.use();
        quadShader.setUniform1i("sTexture", 3);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ma_int64 currentMillisecond = ma_engine_get_time_in_milliseconds(&engine);
        float currentSecond = currentMillisecond / 1000.0f;
        switch(choice){
            case 1: scene1.draw(currentMillisecond); break;
            case 2: scene2.draw(currentMillisecond); break;
            case 3: 
                if(currentSecond >= lastSecond + 0.05f) {
                    scene3.draw(currentMillisecond, true);
                    lastSecond = currentSecond;
                } else {
                    scene3.draw(currentMillisecond, false);
                }
                break;
            default: break;
        }
        if(isBloom) bloom.end(waterFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, waterTextureAttachment);
        quadShader.use();
        quadShader.setUniform1i("sTexture", 5);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, waterTextureAttachment);
        waterShader.use();
        waterShader.setUniform1i("dudvTexture", 4);
        waterShader.setUniform1i("reflection", 5);
        waterShader.setUniform1f("offset", waterOffset);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        waterOffset += 0.001f;
        waterOffset = fmodf(waterOffset, 1);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
         switch(choice){
            case 1: scene1.draw(currentMillisecond); break;
            case 2: scene2.draw(currentMillisecond); break;
            case 3: 
                if(currentSecond >= lastSecond + 0.05f) {
                    scene3.draw(currentMillisecond, true);
                    lastSecond = currentSecond;
                } else {
                    scene3.draw(currentMillisecond, false);
                }
                break;
            default: break;
        }

        if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) choice = 1;
        if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) choice = 2;
        if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) choice = 3;
        if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) isBloom = true;
        if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) isBloom = false;
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) scene3.rotateBy(0.1f, 0.0f);
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) scene3.rotateBy(-0.1f, 0.0f);
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) scene3.rotateBy(0.0f, -0.05f);
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) scene3.rotateBy(0.0f, 0.05f);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    ma_engine_uninit(&engine);
    ma_sound_uninit(&sound);
    glfwTerminate();

    return 0;
}
 
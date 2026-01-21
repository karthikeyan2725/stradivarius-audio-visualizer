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

int main(){
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

    const char* songPath = "audio/escape.wav";
    
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
    Scene2 scene2(audioFile.samples, sampleRate, frameBufferWidth, frameBufferHeight);
    Scene3 scene3(audioFile.samples, sampleRate, frameBufferWidth, frameBufferHeight);
    
    int choice = 1;
    bool isBloom = false;
    unsigned int vao;
    glGenVertexArrays(1, &vao); // TODO: Move to each waveforms
    glBindVertexArray(vao);
    
    stbi_set_flip_vertically_on_load(true);  
    int w, h, nC;
    unsigned char *bg_image_data = stbi_load("images/red_cloud.jpg", &w, &h, &nC, 0);
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
    
    float quadVertex[24] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int bgVBO;
    glGenBuffers(1, &bgVBO);
    glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertex), quadVertex, GL_STATIC_DRAW);

    Shader bgShader("src/shaders/bg.vs", "src/shaders/bg.fs");

    Bloom bloom(frameBufferWidth, frameBufferHeight);
    b = &bloom;
    float rotateX = 20.0f;
    float rotateY = 0;
    float lastSecond = 0;
    glEnable( GL_DEPTH_CLAMP ) ;
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   
        
        if(isBloom) bloom.start();
        glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        bgShader.use();
        bgShader.setUniform1i("sTexture", 3);
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
                    scene3.draw(currentMillisecond, true);
                }
                break;
            default: break;
        }
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if(isBloom) bloom.end(0);
        
        if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) choice = 1;
        if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) choice = 2;
        if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) choice = 3;
        if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) isBloom = true;
        if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) isBloom = false;
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) rotateX+=0.1;
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) rotateX-=0.1;
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rotateY-=0.05;
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) rotateY+=0.05;

        if(choice == 3){
            // rotateY-=0.05f;
            scene3.setRotation(rotateX, rotateY);
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    ma_engine_uninit(&engine);
    ma_sound_uninit(&sound);
    glfwTerminate();

    return 0;
}
 
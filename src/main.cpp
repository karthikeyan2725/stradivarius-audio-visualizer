#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <audiofile/AudioFile.h>
#include <miniaudio.h>

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
    
    Bloom bloom(frameBufferWidth, frameBufferHeight);
    b = &bloom;
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
        
        ma_int64 currentMillisecond = ma_engine_get_time_in_milliseconds(&engine);
        if(isBloom) bloom.start();
        switch(choice){
            case 1: scene1.draw(currentMillisecond); break;
            case 2: scene2.draw(currentMillisecond); break;
            case 3: scene3.draw(currentMillisecond); break;
            default: break;
        }
        if(isBloom) bloom.end();
        
        if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) choice = 1;
        if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) choice = 2;
        if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) choice = 3;
        if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) isBloom = true;
        if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) isBloom = false;
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    ma_engine_uninit(&engine);
    ma_sound_uninit(&sound);
    glfwTerminate();

    return 0;
}
 
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <audiofile/AudioFile.h>
#include <miniaudio.h>
#include <chrono>

using namespace std::chrono;

void resize_frame_buffer_on_window(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

float generateRandomFloat(int min, int max){
    int decimal = min + (rand() % (int)(max - min));
    float fractional = ((float)rand() / (float) RAND_MAX);
    return (float)(decimal + fractional);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "Stradivarius", NULL, NULL);
    if(window == NULL){
        std::cout << "Failed to create a GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initalize GLAD" << std::endl;
        return -1;
    }

    int frameBufferWidth, frameBufferHeight;
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
    glViewport(0, 0, frameBufferWidth, frameBufferHeight);

    glfwSetFramebufferSizeCallback(window, resize_frame_buffer_on_window);

    std::cout << "Strad has started..." << std::endl;

    // AudioFile test
    AudioFile<double> audioFile;
    audioFile.load("audio/memphis-trap.wav");
    audioFile.printSummary();
    int numSamples = audioFile.getNumSamplesPerChannel();
    for(int i = 0; i < numSamples; i++){
        double sample = audioFile.samples[0][i];
    }

    float signal[16000 * 3] = {0};
    for(int i = 0; i < 16000 * 3; i+=3){
        signal[i] = ((float)i/3)/16000 - 0.5f;
        // std::cout << signal[i] << std::endl;
        signal[i+1] = audioFile.samples[0][i];
        signal[i+2] = 0.0f;
    }

    unsigned int signalVao;
    glGenVertexArrays(1, &signalVao);
    glBindVertexArray(signalVao);

    unsigned int signalVbo;
    glGenBuffers(1, &signalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, signalVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(signal), signal, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    const char* vertexShaderSrc = 
        "#version 330 core\n" 
        "layout (location = 0) in vec3 point;\n"
        "void main(){\n"
           "gl_Position = vec4(point.x, point.y, point.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 color;"
        "void main(){\n"
            "color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\0";

    int success;
    char message[1000];
  
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 1000, NULL, message);
        std::cout << "Failed to compile Vertex Shader: " << message << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 1000, NULL, message);
        std::cout << "Failed to compile Fragment Shader: " << message << std::endl;
    }
    
    unsigned int shader = glCreateProgram();
    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shader, 1000, NULL, message);
        std::cout << "Failed to link shaders: " << message << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    fflush(0);
    
    ma_result result;
    ma_engine engine;

    result = ma_engine_init(NULL, &engine);
    if(result != MA_SUCCESS){
        std::cout << "Failed to initialize Miniaudio Engine" << std::endl;
        return -1;
    } 

    
    glClearColor(1.0f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    int offset = 0;
    float prevTime = glfwGetTime(), currTime;
    int frameCount = 0;
    auto cPrev = system_clock::now();
    auto cNow = system_clock::now();
    ma_engine_play_sound(&engine, "/run/media/karthik/Shared Volume/1_projects/1_dev/strad_audio_visualizer/audio/memphis-trap.wav", NULL);
    while(!glfwWindowShouldClose(window)){
        // for(int i = 0; i < 300; i+=3){
        //     signal[i+1] = generateRandomFloat(0, 1);
        //     std::cout << signal[i+1] << std::endl;
        // }
        for(int i = 0; i < 16000 * 3; i+=3){
            signal[i+1] = audioFile.samples[0][i + (int)(16000/50.0f) * offset];
        }
        offset+=1;
        glBindBuffer(GL_ARRAY_BUFFER, signalVao);
        glBufferData(GL_ARRAY_BUFFER, sizeof(signal), signal, GL_STATIC_DRAW);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);
        glBindVertexArray(signalVao);
        glDrawArrays(GL_LINE_STRIP, 0,16000);
        
        currTime = glfwGetTime();
        float change = currTime - prevTime;
        if(change >= 1){
            float fps = frameCount / change;
            std::cout << "FPS: " << fps << std::endl;
            frameCount = 0;
            prevTime = currTime;
        }

        // cNow = system_clock::now();
        // std::cout << duration_cast<milliseconds>((cNow - cPrev)).count() << std::endl;
        while(duration_cast<milliseconds>((system_clock::now() - cPrev)).count() <= (1000/50.0f)){
            
        }
        cPrev = system_clock::now();
        glfwSwapBuffers(window);
        frameCount++;
        glfwPollEvents();
    }

    ma_engine_uninit(&engine);

    glfwTerminate();
    return 0;
}
 
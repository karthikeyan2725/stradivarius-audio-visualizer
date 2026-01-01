#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <audiofile/AudioFile.h>
#include <miniaudio.h>

#include <iostream>
#include <chrono>
using namespace std::chrono;

#define APPNAME "Stradivarius "
#define ONE_SECOND 1000
#define FPS 60

void resize_frame_buffer_on_window(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, APPNAME, NULL, NULL);
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

    AudioFile<double> audioFile;
    audioFile.load("audio/memphis-trap.wav");
    audioFile.printSummary();
    int sampleRate = audioFile.getSampleRate();
    int numVertices = 16000 * 3;
    int signalVertexDataSize = numVertices * sizeof(float);
    float *signalVertexData = new float[numVertices];
    for(int i = 0; i < numVertices; i+=3){
        signalVertexData[i] = ((float)i/3)/16000 - 0.5f;
        signalVertexData[i+1] = 0.0f;
        signalVertexData[i+2] = 0.0f;
    }

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, signalVertexDataSize, signalVertexData, GL_STATIC_DRAW);
    
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
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shaderProgram, 1000, NULL, message);
        std::cout << "Failed to link shaders: " << message << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    ma_result result;
    ma_engine engine;
    result = ma_engine_init(NULL, &engine);
    if(result != MA_SUCCESS){
        std::cout << "Failed to initialize Miniaudio Engine" << std::endl;
        glfwTerminate();
        return -1;
    } 
    ma_engine_play_sound(&engine, "./audio/memphis-trap.wav", NULL);
    
    float prevTime = glfwGetTime(), currTime, fps, deltaTime;
    int frameCount = 0;
    auto cPrev = system_clock::now();
    
    int offset = 0;

    glClearColor(1.0f, 0.0f, 0.5f, 1.0f);
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);

        for(int i = 0; i < numVertices; i+=3){
            signalVertexData[i+1] = audioFile.samples[0][i + (int)((float)16000 / FPS) * offset];
        }
        offset++;
        glBindBuffer(GL_ARRAY_BUFFER, vao);
        glBufferData(GL_ARRAY_BUFFER, signalVertexDataSize, signalVertexData, GL_STATIC_DRAW);
        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINE_STRIP, 0, 16000);
        
        currTime = glfwGetTime();
        deltaTime = currTime - prevTime;
        if(currTime - prevTime > 1){
            fps = frameCount / deltaTime;
            frameCount = 0;
            prevTime = currTime;

            glfwSetWindowTitle(window, (APPNAME + std::to_string(fps)).c_str());
        }

        while(duration_cast<milliseconds>((system_clock::now() - cPrev)).count() <= ((float)ONE_SECOND / FPS)){
            
        }
        glfwSwapBuffers(window);
        cPrev = system_clock::now();
        frameCount++;
        glfwPollEvents();
    }

    delete signalVertexData;
    ma_engine_uninit(&engine);
    glfwTerminate();

    return 0;
}
 